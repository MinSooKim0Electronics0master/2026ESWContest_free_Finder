/*
 * Wokwi A/B/C와 실물 수신 단말 D의 하이브리드 시연 펌웨어입니다.
 *
 * Wokwi의 MQTT 패킷을 실물 D가 받아 실제 OLED와 BLE 광고에 반영합니다.
 * MQTT는 LoRa 전달을 대체할 뿐이며, 이 코드의 화면과 로그를 실제 LoRa
 * 거리·RSSI 검증으로 해석하면 안 됩니다.
 */

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEAdvertising.h>

#include "../../common/packet.h"
#include "../../common/lilygo_t3_v161.h"

#if __has_include("hybrid_secrets.h")
#include "hybrid_secrets.h"
#else
// 실제 업로드 전 hybrid_secrets.example.h를 hybrid_secrets.h로 복사합니다.
#define FINDER_WIFI_SSID     "TODO_WIFI_SSID"
#define FINDER_WIFI_PASSWORD "TODO_WIFI_PASSWORD"
#endif

static const char* MQTT_BROKER = "broker.hivemq.com";
static const uint16_t MQTT_PORT = 1883;
static const char* MQTT_TOPIC =
    "eswcontest/finder-2026-hybrid-8f4a2c/lora";

// 첫 패킷 한 번만 Wokwi A에서 직접 받아 연결을 확인합니다. 그다음부터는
// B/C가 중계한 패킷만 받아 정상 경로와 우회 경로를 구분합니다.
static bool acceptDirectAOnce = true;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
Adafruit_SSD1306 display(
    FINDER_OLED_WIDTH,
    FINDER_OLED_HEIGHT,
    &Wire,
    -1);

static BLEServer* bleServer = nullptr;
static BLEAdvertising* bleAdvertising = nullptr;
static bool displayReady = false;
static uint32_t seenMessages[FINDER_MSG_CACHE_SIZE] = {};
static uint8_t seenCount = 0;
static uint8_t seenWriteIndex = 0;
static uint32_t lastMqttAttemptMs = 0;

// 0xFFFF는 시험용으로 둔 식별값입니다. 등록된 Bluetooth Company ID라고
// 주장하지 않으며, nRF Connect에서 데이터 위치를 쉽게 찾는 용도입니다.
typedef struct __attribute__((packed)) {
  uint16_t testCompanyId;
  uint8_t version;
  uint8_t facilityType;
  uint8_t status;
  uint8_t srcId;
  uint8_t lastHopId;
  uint32_t msgId;
} FinderBlePayload;

static_assert(
    sizeof(FinderBlePayload) == 11,
    "FinderBlePayload는 11바이트여야 한다");

static char nodeLetter(uint8_t nodeId) {
  if (nodeId < FINDER_NODE_A || nodeId > FINDER_NODE_D) {
    return '?';
  }
  return static_cast<char>('A' + nodeId - 1);
}

static const char* facilityText(uint8_t facilityType) {
  switch (facilityType) {
    case FINDER_FACILITY_AED:
      return "AED";
    case FINDER_FACILITY_HYDRANT:
      return "HYDRANT";
    case FINDER_FACILITY_TOILET:
      return "TOILET";
    default:
      return "UNKNOWN";
  }
}

static bool packetIsValid(const FinderPacket& packet) {
  const uint8_t encodedSource = static_cast<uint8_t>(packet.msgId >> 24);
  return packet.msgId != 0 &&
         encodedSource == packet.srcId &&
         packet.srcId >= FINDER_NODE_A &&
         packet.srcId <= FINDER_NODE_D &&
         packet.facilityType >= FINDER_FACILITY_AED &&
         packet.facilityType <= FINDER_FACILITY_TOILET &&
         packet.status <= FINDER_STATUS_MAINT &&
         packet.ttl <= FINDER_TTL_INITIAL &&
         packet.lastHopId >= FINDER_NODE_A &&
         packet.lastHopId <= FINDER_NODE_D;
}

static bool hasSeen(uint32_t msgId) {
  for (uint8_t i = 0; i < seenCount; ++i) {
    if (seenMessages[i] == msgId) {
      return true;
    }
  }
  return false;
}

static void remember(uint32_t msgId) {
  if (seenCount < FINDER_MSG_CACHE_SIZE) {
    seenMessages[seenCount++] = msgId;
    return;
  }
  seenMessages[seenWriteIndex] = msgId;
  seenWriteIndex =
      static_cast<uint8_t>((seenWriteIndex + 1) % FINDER_MSG_CACHE_SIZE);
}

static void drawStatus(const char* line1, const char* line2) {
  if (!displayReady) {
    return;
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("FINDER D / HYBRID");
  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);
  display.setCursor(0, 20);
  display.println(line1);
  display.setCursor(0, 34);
  display.println(line2);
  display.setCursor(0, 52);
  display.println("LORA RF: NOT TESTED");
  display.display();
}

static void drawPacket(const FinderPacket& packet, uint8_t transmitterId) {
  if (!displayReady) {
    return;
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("FACILITY: ");
  display.println(facilityText(packet.facilityType));

  display.setCursor(0, 13);
  display.print("PATH: A -> ");
  display.print(nodeLetter(transmitterId));
  display.println(" -> D");

  display.setCursor(0, 26);
  display.print("MSG #");
  display.print(packet.msgId & 0x00FFFFFFUL);
  display.print(" TTL ");
  display.println(packet.ttl);

  display.setCursor(0, 39);
  display.println("LINK: MQTT SIM");
  display.setCursor(0, 52);
  display.println("BLE: REAL / RF: TODO");
  display.display();
}

static void updateBleAdvertisement(const FinderPacket& packet) {
  if (bleAdvertising == nullptr) {
    return;
  }

  FinderBlePayload payload = {};
  payload.testCompanyId = 0xFFFF;
  payload.version = 1;
  payload.facilityType = packet.facilityType;
  payload.status = packet.status;
  payload.srcId = packet.srcId;
  payload.lastHopId = packet.lastHopId;
  payload.msgId = packet.msgId;

  BLEAdvertisementData advertisementData;
  advertisementData.setFlags(0x06);
  advertisementData.setName("Finder-D");
  advertisementData.setManufacturerData(
      String(reinterpret_cast<const char*>(&payload), sizeof(payload)));

  bleAdvertising->stop();
  bleAdvertising->setAdvertisementData(advertisementData);
  bleAdvertising->start();

  Serial.print("[D/BLE] 광고 갱신 msgId=0x");
  Serial.println(packet.msgId, HEX);
}

static void initBle() {
  BLEDevice::init("Finder-D");
  bleServer = BLEDevice::createServer();
  bleAdvertising = bleServer->getAdvertising();
  bleAdvertising->setScanResponse(false);

  BLEAdvertisementData initialData;
  initialData.setFlags(0x06);
  initialData.setName("Finder-D");
  bleAdvertising->setAdvertisementData(initialData);
  bleAdvertising->start();
  Serial.println("[D/BLE] 초기 광고 시작");
}

static void onMqttMessage(char* topic, byte* payload, unsigned int length) {
  if (strcmp(topic, MQTT_TOPIC) != 0 ||
      length != 1 + sizeof(FinderPacket)) {
    return;
  }

  const uint8_t transmitterId = payload[0];
  FinderPacket packet = {};
  memcpy(&packet, payload + 1, sizeof(packet));

  const bool expectedNeighbor =
      transmitterId == FINDER_NODE_B ||
      transmitterId == FINDER_NODE_C ||
      (acceptDirectAOnce && transmitterId == FINDER_NODE_A);
  if (!expectedNeighbor || packet.lastHopId != transmitterId) {
    return;
  }
  if (!packetIsValid(packet)) {
    Serial.println("[D/MQTT] 잘못된 패킷 폐기");
    return;
  }
  if (hasSeen(packet.msgId)) {
    Serial.print("[D/MQTT] 중복 폐기 msgId=0x");
    Serial.println(packet.msgId, HEX);
    return;
  }

  remember(packet.msgId);
  if (transmitterId == FINDER_NODE_A) {
    acceptDirectAOnce = false;
    Serial.println("[D/MQTT] 직접 연결 확인 완료, 이후 B/C 중계만 수신");
  }

  Serial.print("[D/MQTT] 수신 성공 msgId=0x");
  Serial.print(packet.msgId, HEX);
  Serial.print(" src=");
  Serial.print(nodeLetter(packet.srcId));
  Serial.print(" via=");
  Serial.print(nodeLetter(transmitterId));
  Serial.print(" ttl=");
  Serial.println(packet.ttl);

  drawPacket(packet, transmitterId);
  updateBleAdvertisement(packet);

  digitalWrite(FINDER_BOARD_LED, HIGH);
  delay(80);
  digitalWrite(FINDER_BOARD_LED, LOW);
}

static void connectWifi() {
  drawStatus("WIFI CONNECTING", FINDER_WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(FINDER_WIFI_SSID, FINDER_WIFI_PASSWORD);
  Serial.print("[D/MQTT] Wi-Fi 연결 중");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print('.');
  }
  Serial.print(" 완료, IP=");
  Serial.println(WiFi.localIP());
  drawStatus("WIFI CONNECTED", "MQTT CONNECTING");
}

static void connectMqttIfNeeded() {
  if (mqttClient.connected() || WiFi.status() != WL_CONNECTED) {
    return;
  }
  const uint32_t now = millis();
  if (now - lastMqttAttemptMs < 2000) {
    return;
  }
  lastMqttAttemptMs = now;

  const uint64_t chipId = ESP.getEfuseMac();
  String clientId = "finder-real-D-";
  clientId += String(static_cast<uint32_t>(chipId), HEX);
  if (!mqttClient.connect(clientId.c_str())) {
    Serial.print("[D/MQTT] 브로커 연결 실패, state=");
    Serial.println(mqttClient.state());
    return;
  }
  mqttClient.subscribe(MQTT_TOPIC);
  Serial.print("[D/MQTT] 브로커 연결 및 구독 완료: ");
  Serial.println(MQTT_TOPIC);
  drawStatus("MQTT CONNECTED", "WAITING WOKWI A");
}

void setup() {
  pinMode(FINDER_BOARD_LED, OUTPUT);
  digitalWrite(FINDER_BOARD_LED, LOW);
  Serial.begin(115200);
  delay(300);
  Serial.println();
  Serial.println("=== finder hybrid demo / real D ===");
  Serial.println("MQTT 시뮬레이션 + 실제 OLED/BLE 단계입니다.");

  Wire.begin(FINDER_OLED_SDA, FINDER_OLED_SCL);
  displayReady = display.begin(
      SSD1306_SWITCHCAPVCC,
      FINDER_OLED_ADDRESS);
  drawStatus("BOOTING", "OLED + BLE");

  initBle();
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(onMqttMessage);
  connectWifi();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWifi();
  }
  connectMqttIfNeeded();
  mqttClient.loop();
}
