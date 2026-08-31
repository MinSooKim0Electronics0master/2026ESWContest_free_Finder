/*
 * 파인더 시설 노드 A/B/C용 최소 메시 시연 펌웨어
 *
 * 대상 보드: LILYGO T3 LoRa32 V1.6.1, SX1278 433 MHz
 * 라이브러리: RadioLib, Adafruit GFX, Adafruit SSD1306
 *
 * 업로드할 보드에 맞춰 아래 FINDER_THIS_NODE_ID 한 줄만 바꾼다.
 *   A(발신) = FINDER_NODE_A
 *   B(중계) = FINDER_NODE_B
 *   C(중계) = FINDER_NODE_C
 *
 * 주의: 안테나를 연결하지 않은 상태에서는 송신하지 않는다.
 */

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <RadioLib.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "../common/packet.h"
#include "../common/lilygo_t3_v161.h"

#ifndef FINDER_THIS_NODE_ID
#define FINDER_THIS_NODE_ID FINDER_NODE_A
#endif

#if (FINDER_THIS_NODE_ID != FINDER_NODE_A) && (FINDER_THIS_NODE_ID != FINDER_NODE_B) && (FINDER_THIS_NODE_ID != FINDER_NODE_C)
#error "FINDER_THIS_NODE_ID는 A, B, C 중 하나여야 합니다"
#endif

// A는 15초마다 한 번 자동 송신한다. 시리얼 모니터에서 s를 입력해도 즉시
// 송신한다. 반복 송신 간격은 실기 측정 후 전파 점유율 기준에 맞춰 조정한다.
static const bool ENABLE_AUTO_SOURCE = true;
static const uint32_t SOURCE_INTERVAL_MS = 15000UL;
static const uint32_t SOURCE_FIRST_DELAY_MS = 5000UL;

// 시연에서 B가 정상일 때 B 경로가 먼저 보이고, B를 끄면 C 경로가 보이도록
// 전체 50~300 ms 범위 안에서 B와 C의 지연 구간을 나눈다.
static const bool DEMO_PREFER_B_PATH = true;

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
static uint32_t sourceSequence = 0;
static uint32_t lastSourceMs = 0;
static uint32_t seenMessages[FINDER_MSG_CACHE_SIZE] = {};
static uint8_t seenCount = 0;
static uint8_t seenWriteIndex = 0;

static char nodeLetter() {
  return static_cast<char>('A' + FINDER_THIS_NODE_ID - 1);
}

static const char* nodeRoleText() {
  return FINDER_THIS_NODE_ID == FINDER_NODE_A ? "SOURCE" : "RELAY";
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

static void drawScreen(const char* line1, const char* line2, const char* line3) {
  if (!displayReady) {
    return;
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("FINDER ");
  display.print(nodeLetter());
  display.print(" / ");
  display.println(nodeRoleText());
  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);
  display.setCursor(0, 17);
  display.println(line1);
  display.setCursor(0, 32);
  display.println(line2);
  display.setCursor(0, 47);
  display.println(line3);
  display.display();
}

static void stopWithError(const char* stage, int16_t state) {
  Serial.print("[ERROR] ");
  Serial.print(stage);
  Serial.print(" 실패, RadioLib 상태=");
  Serial.println(state);

  char stateText[22];
  snprintf(stateText, sizeof(stateText), "STATE %d", state);
  drawScreen("RADIO ERROR", stage, stateText);

  while (true) {
    digitalWrite(FINDER_BOARD_LED, HIGH);
    delay(150);
    digitalWrite(FINDER_BOARD_LED, LOW);
    delay(850);
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

static uint16_t relayDelayMs() {
  if (!DEMO_PREFER_B_PATH) {
    return static_cast<uint16_t>(
        random(FINDER_RELAY_DELAY_MIN_MS, FINDER_RELAY_DELAY_MAX_MS + 1));
  }

  if (FINDER_THIS_NODE_ID == FINDER_NODE_B) {
    return static_cast<uint16_t>(random(50, 121));
  }
  return static_cast<uint16_t>(random(180, 301));
}

static void logPacket(const char* event, const FinderPacket& packet) {
  Serial.print('[');
  Serial.print(nodeLetter());
  Serial.print("] ");
  Serial.print(event);
  Serial.print(" msgId=0x");
  Serial.print(packet.msgId, HEX);
  Serial.print(" src=");
  Serial.print(packet.srcId);
  Serial.print(" via=");
  Serial.print(packet.lastHopId);
  Serial.print(" ttl=");
  Serial.println(packet.ttl);
}

static void transmitSourcePacket() {
  ++sourceSequence;
  sourceSequence &= 0x00FFFFFFUL;
  if (sourceSequence == 0) {
    sourceSequence = 1;
  }

  FinderPacket packet = {};
  packet.msgId =
      (static_cast<uint32_t>(FINDER_THIS_NODE_ID) << 24) | sourceSequence;
  packet.srcId = FINDER_THIS_NODE_ID;
  packet.facilityType = FINDER_FACILITY_AED;
  packet.status = FINDER_STATUS_OK;
  packet.ttl = FINDER_TTL_INITIAL;
  packet.lastHopId = FINDER_THIS_NODE_ID;

  remember(packet.msgId);
  logPacket("송신", packet);

  digitalWrite(FINDER_BOARD_LED, HIGH);
  const int16_t state =
      radio.transmit(reinterpret_cast<uint8_t*>(&packet), sizeof(packet));
  digitalWrite(FINDER_BOARD_LED, LOW);

  char sequenceText[22];
  snprintf(sequenceText, sizeof(sequenceText), "MSG #%lu",
           static_cast<unsigned long>(sourceSequence));
  drawScreen(
      state == RADIOLIB_ERR_NONE ? "TX SUCCESS" : "TX FAILED",
      facilityText(packet.facilityType),
      sequenceText);

  if (state != RADIOLIB_ERR_NONE) {
    Serial.print("[A] 송신 실패, RadioLib 상태=");
    Serial.println(state);
  }
}

static void receiveAndRelay() {
  FinderPacket packet = {};
  const int16_t state =
      radio.receive(reinterpret_cast<uint8_t*>(&packet), sizeof(packet));

  if (state == RADIOLIB_ERR_RX_TIMEOUT) {
    return;
  }
  if (state == RADIOLIB_ERR_CRC_MISMATCH) {
    Serial.println("[RELAY] CRC 오류 패킷 폐기");
    return;
  }
  if (state != RADIOLIB_ERR_NONE) {
    Serial.print("[RELAY] 수신 오류, RadioLib 상태=");
    Serial.println(state);
    return;
  }
  if (radio.getPacketLength() != sizeof(FinderPacket) ||
      !packetIsValid(packet)) {
    Serial.println("[RELAY] 길이 또는 필드가 잘못된 패킷 폐기");
    return;
  }
  if (hasSeen(packet.msgId)) {
    logPacket("중복 폐기", packet);
    return;
  }

  remember(packet.msgId);
  logPacket("첫 수신", packet);

  if (packet.ttl <= 1) {
    drawScreen("RX / TTL END", facilityText(packet.facilityType), "NO RELAY");
    return;
  }

  const uint16_t waitMs = relayDelayMs();
  char waitText[22];
  snprintf(waitText, sizeof(waitText), "WAIT %u ms", waitMs);
  drawScreen("RX / RELAY READY", facilityText(packet.facilityType), waitText);
  delay(waitMs);

  --packet.ttl;
  packet.lastHopId = FINDER_THIS_NODE_ID;
  digitalWrite(FINDER_BOARD_LED, HIGH);
  const int16_t txState =
      radio.transmit(reinterpret_cast<uint8_t*>(&packet), sizeof(packet));
  digitalWrite(FINDER_BOARD_LED, LOW);

  logPacket(txState == RADIOLIB_ERR_NONE ? "중계 송신" : "중계 실패", packet);
  drawScreen(
      txState == RADIOLIB_ERR_NONE ? "RELAY SUCCESS" : "RELAY FAILED",
      facilityText(packet.facilityType),
      txState == RADIOLIB_ERR_NONE ? "WAITING NEXT" : "CHECK SERIAL");
}

void setup() {
  pinMode(FINDER_BOARD_LED, OUTPUT);
  digitalWrite(FINDER_BOARD_LED, LOW);

  Serial.begin(115200);
  delay(300);
  Serial.println();
  Serial.println("=== finder-project LILYGO node ===");
  Serial.print("노드=");
  Serial.println(nodeLetter());
  Serial.println("주의: 안테나 연결 후에만 송신하십시오.");

  Wire.begin(FINDER_OLED_SDA, FINDER_OLED_SCL);
  displayReady = display.begin(
      SSD1306_SWITCHCAPVCC,
      FINDER_OLED_ADDRESS);
  drawScreen("BOOTING", "433.92 MHz", "LOW POWER 2 dBm");

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

  randomSeed(esp_random());
  lastSourceMs = millis();

  if (FINDER_THIS_NODE_ID == FINDER_NODE_A) {
    drawScreen("READY TO SEND", "AUTO: 15 sec", "SERIAL: s");
  } else {
    drawScreen("RELAY READY", "WAITING PACKET", "B/C MESH PATH");
  }
}

void loop() {
  if (FINDER_THIS_NODE_ID == FINDER_NODE_A) {
    while (Serial.available() > 0) {
      const char command = static_cast<char>(Serial.read());
      if (command == 's' || command == 'S') {
        transmitSourcePacket();
        lastSourceMs = millis();
      }
    }

    const uint32_t now = millis();
    const uint32_t firstOrInterval =
        sourceSequence == 0 ? SOURCE_FIRST_DELAY_MS : SOURCE_INTERVAL_MS;
    if (ENABLE_AUTO_SOURCE && now - lastSourceMs >= firstOrInterval) {
      transmitSourcePacket();
      lastSourceMs = now;
    }
    delay(10);
    return;
  }

  receiveAndRelay();
}
