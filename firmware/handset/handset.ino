/*
 * 파인더 수신 단말 D용 최소 메시 시연 펌웨어
 *
 * 대상 보드: LILYGO T3 LoRa32 V1.6.1, SX1278 433 MHz
 * 라이브러리: RadioLib, Adafruit GFX, Adafruit SSD1306
 *
 * B 또는 C가 중계한 패킷만 시연 수신으로 인정한다. 같은 방에서는 A의
 * 전파가 D에 직접 도달하므로, 이 조건이 있어야 B 전원 제거 전후의
 * A→B/C→D 경로 변화를 화면에서 분명히 확인할 수 있다.
 *
 * 주의: 안테나를 연결하지 않은 상태에서는 전원을 켜고 송신하지 않는다.
 */

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <RadioLib.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "../common/packet.h"
#include "../common/lilygo_t3_v161.h"

static const bool DEMO_REQUIRE_RELAY_PATH = true;
static const uint8_t RSSI_AVERAGE_COUNT = 5;

SX1278 radio = new Module(
    FINDER_LORA_CS,
    FINDER_LORA_DIO0,
    FINDER_LORA_RST,
    FINDER_LORA_DIO1);

Adafruit_SSD1306 display(
    FINDER_OLED_WIDTH,
    FINDER_OLED_HEIGHT,
    &Wire,
    -1);

static bool displayReady = false;
static uint32_t seenMessages[FINDER_MSG_CACHE_SIZE] = {};
static uint8_t seenCount = 0;
static uint8_t seenWriteIndex = 0;
static float rssiSamples[RSSI_AVERAGE_COUNT] = {};
static uint8_t rssiCount = 0;
static uint8_t rssiWriteIndex = 0;

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

static char pathLetter(uint8_t nodeId) {
  if (nodeId < FINDER_NODE_A || nodeId > FINDER_NODE_D) {
    return '?';
  }
  return static_cast<char>('A' + nodeId - 1);
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

static float addRssiSample(float rssi) {
  rssiSamples[rssiWriteIndex] = rssi;
  rssiWriteIndex =
      static_cast<uint8_t>((rssiWriteIndex + 1) % RSSI_AVERAGE_COUNT);
  if (rssiCount < RSSI_AVERAGE_COUNT) {
    ++rssiCount;
  }

  float sum = 0.0f;
  for (uint8_t i = 0; i < rssiCount; ++i) {
    sum += rssiSamples[i];
  }
  return sum / rssiCount;
}

static void drawWaiting(const char* message) {
  if (!displayReady) {
    return;
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("FINDER D / HANDSET");
  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);
  display.setCursor(0, 20);
  display.println(message);
  display.setCursor(0, 36);
  display.println("433.92 MHz / SF7");
  display.setCursor(0, 51);
  display.println("PATH: A-B/C-D");
  display.display();
}

static void drawPacket(
    const FinderPacket& packet,
    float instantRssi,
    float averageRssi) {
  if (!displayReady) {
    return;
  }

  const int averageRounded = static_cast<int>(averageRssi);
  const int constrainedRssi = constrain(averageRounded, -120, -40);
  const int barWidth = map(constrainedRssi, -120, -40, 0, 116);

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("FACILITY: ");
  display.println(facilityText(packet.facilityType));

  display.setCursor(0, 13);
  display.print("PATH: A -> ");
  display.print(pathLetter(packet.lastHopId));
  display.println(" -> D");

  display.setCursor(0, 26);
  display.print("RSSI: ");
  display.print(instantRssi, 0);
  display.print(" AVG:");
  display.println(averageRssi, 0);

  display.setCursor(0, 39);
  display.print("MSG #");
  display.print(packet.msgId & 0x00FFFFFFUL);
  display.print(" TTL ");
  display.println(packet.ttl);

  display.drawRect(5, 53, 118, 9, SSD1306_WHITE);
  display.fillRect(6, 54, barWidth, 7, SSD1306_WHITE);
  display.display();
}

static void stopWithError(const char* stage, int16_t state) {
  Serial.print("[ERROR] ");
  Serial.print(stage);
  Serial.print(" 실패, RadioLib 상태=");
  Serial.println(state);

  if (displayReady) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("RADIO ERROR");
    display.setCursor(0, 18);
    display.println(stage);
    display.setCursor(0, 36);
    display.print("STATE ");
    display.println(state);
    display.display();
  }

  while (true) {
    digitalWrite(FINDER_BOARD_LED, HIGH);
    delay(150);
    digitalWrite(FINDER_BOARD_LED, LOW);
    delay(850);
  }
}

static void logPacket(
    const char* event,
    const FinderPacket& packet,
    float rssi) {
  Serial.print("[D] ");
  Serial.print(event);
  Serial.print(" msgId=0x");
  Serial.print(packet.msgId, HEX);
  Serial.print(" src=");
  Serial.print(packet.srcId);
  Serial.print(" via=");
  Serial.print(pathLetter(packet.lastHopId));
  Serial.print(" ttl=");
  Serial.print(packet.ttl);
  Serial.print(" RSSI=");
  Serial.println(rssi, 1);
}

static void receiveOnePacket() {
  FinderPacket packet = {};
  const int16_t state =
      radio.receive(reinterpret_cast<uint8_t*>(&packet), sizeof(packet));

  if (state == RADIOLIB_ERR_RX_TIMEOUT) {
    return;
  }
  if (state == RADIOLIB_ERR_CRC_MISMATCH) {
    Serial.println("[D] CRC 오류 패킷 폐기");
    return;
  }
  if (state != RADIOLIB_ERR_NONE) {
    Serial.print("[D] 수신 오류, RadioLib 상태=");
    Serial.println(state);
    return;
  }
  if (radio.getPacketLength() != sizeof(FinderPacket) ||
      !packetIsValid(packet)) {
    Serial.println("[D] 길이 또는 필드가 잘못된 패킷 폐기");
    return;
  }

  const float rssi = radio.getRSSI();

  // 같은 방에서 직접 들린 A 패킷은 캐시에 넣지 않는다. 이후 B/C가 같은
  // msgId를 중계했을 때 정상 수신할 수 있어야 하기 때문이다.
  if (DEMO_REQUIRE_RELAY_PATH &&
      packet.lastHopId != FINDER_NODE_B &&
      packet.lastHopId != FINDER_NODE_C) {
    logPacket("직접 경로 제외", packet, rssi);
    return;
  }

  if (hasSeen(packet.msgId)) {
    logPacket("중복 폐기", packet, rssi);
    return;
  }

  remember(packet.msgId);
  const float averageRssi = addRssiSample(rssi);
  logPacket("수신 성공", packet, rssi);
  drawPacket(packet, rssi, averageRssi);

  digitalWrite(FINDER_BOARD_LED, HIGH);
  delay(80);
  digitalWrite(FINDER_BOARD_LED, LOW);
}

void setup() {
  pinMode(FINDER_BOARD_LED, OUTPUT);
  digitalWrite(FINDER_BOARD_LED, LOW);

  Serial.begin(115200);
  delay(300);
  Serial.println();
  Serial.println("=== finder-project LILYGO handset D ===");
  Serial.println("주의: 안테나 연결 후에만 전원을 켜십시오.");

  Wire.begin(FINDER_OLED_SDA, FINDER_OLED_SCL);
  displayReady = display.begin(
      SSD1306_SWITCHCAPVCC,
      FINDER_OLED_ADDRESS);
  drawWaiting("BOOTING...");

  SPI.begin(
      FINDER_LORA_SCK,
      FINDER_LORA_MISO,
      FINDER_LORA_MOSI,
      FINDER_LORA_CS);

  const int16_t state = radio.begin(
      FINDER_FREQ_MHZ,
      FINDER_BANDWIDTH_KHZ,
      FINDER_SPREADING_FACTOR,
      FINDER_CODING_RATE,
      FINDER_SYNC_WORD,
      FINDER_TX_POWER_DBM,
      8,
      0);
  if (state != RADIOLIB_ERR_NONE) {
    stopWithError("radio.begin", state);
  }

  const int16_t crcState = radio.setCRC(true);
  if (crcState != RADIOLIB_ERR_NONE) {
    stopWithError("radio.setCRC", crcState);
  }

  drawWaiting("WAITING PACKET");
}

void loop() {
  receiveOnePacket();
}
