// Wokwi A/B/C 메시 노드입니다.
// LoRa 대신 공개 MQTT를 패킷 전달 수단으로 사용합니다.

#include <Arduino.h>
#include "packet.h"
#include "fake_radio.h"

// Wokwi 프로젝트를 A/B/C로 복제한 뒤 이 값만 1/2/3으로 바꿉니다.
#ifndef FINDER_WOKWI_NODE_ID
#define FINDER_WOKWI_NODE_ID FINDER_NODE_A
#endif

#if FINDER_WOKWI_NODE_ID == FINDER_NODE_A
static const uint8_t HEAR[] = {FINDER_NODE_B, FINDER_NODE_C};
#elif FINDER_WOKWI_NODE_ID == FINDER_NODE_B
static const uint8_t HEAR[] = {FINDER_NODE_A, FINDER_NODE_D};
#elif FINDER_WOKWI_NODE_ID == FINDER_NODE_C
static const uint8_t HEAR[] = {FINDER_NODE_A, FINDER_NODE_D};
#elif FINDER_WOKWI_NODE_ID == FINDER_NODE_D
static const uint8_t HEAR[] = {FINDER_NODE_B, FINDER_NODE_C};
#else
#error "FINDER_WOKWI_NODE_ID는 1~4여야 합니다"
#endif

static const uint8_t MY_ID = FINDER_WOKWI_NODE_ID;
static const uint32_t SOURCE_INTERVAL_MS = 10000;

static uint32_t seenMessages[FINDER_MSG_CACHE_SIZE] = {};
static uint8_t seenCount = 0;
static uint8_t seenWriteIndex = 0;

static bool relayPending = false;
static FinderPacket pendingPacket = {};
static uint32_t pendingSendAtMs = 0;

static uint32_t sourceSequence = 1;
static uint32_t nextSourceAtMs = 3000;

static char nodeLetter(uint8_t nodeId) {
  if (nodeId < FINDER_NODE_A || nodeId > FINDER_NODE_D) {
    return '?';
  }
  return static_cast<char>('A' + nodeId - 1);
}

static bool timeReached(uint32_t now, uint32_t deadline) {
  return static_cast<int32_t>(now - deadline) >= 0;
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

static uint32_t relayDelayMs() {
  // 정상 상태에서는 B가 먼저 도착하고, B 정지 후에는 C 경로가 보이도록
  // 시연용 지연 범위를 분리합니다. 실제 메시 규칙의 전체 범위 안입니다.
  if (MY_ID == FINDER_NODE_B) {
    return random(50, 121);
  }
  if (MY_ID == FINDER_NODE_C) {
    return random(180, 301);
  }
  return random(
      FINDER_RELAY_DELAY_MIN_MS,
      FINDER_RELAY_DELAY_MAX_MS + 1);
}

static void logPacket(const char* event, const FinderPacket& packet) {
  Serial.print('[');
  Serial.print(nodeLetter(MY_ID));
  Serial.print("] ");
  Serial.print(event);
  Serial.print(" msgId=0x");
  Serial.print(packet.msgId, HEX);
  Serial.print(" src=");
  Serial.print(nodeLetter(packet.srcId));
  Serial.print(" via=");
  Serial.print(nodeLetter(packet.lastHopId));
  Serial.print(" ttl=");
  Serial.println(packet.ttl);
}

static void onPacket(const FinderPacket& received) {
  if (!packetIsValid(received)) {
    Serial.println("[mesh] 잘못된 패킷 폐기");
    return;
  }
  if (hasSeen(received.msgId)) {
    logPacket("중복 폐기", received);
    return;
  }

  remember(received.msgId);
  logPacket("수신", received);

  const bool isRelay =
      MY_ID == FINDER_NODE_B || MY_ID == FINDER_NODE_C;
  if (!isRelay || received.ttl <= 1) {
    return;
  }
  if (relayPending) {
    Serial.println("[mesh] 이전 재송신 대기 중이므로 새 예약 생략");
    return;
  }

  pendingPacket = received;
  --pendingPacket.ttl;
  pendingPacket.lastHopId = MY_ID;
  const uint32_t waitMs = relayDelayMs();
  pendingSendAtMs = millis() + waitMs;
  relayPending = true;

  Serial.print('[');
  Serial.print(nodeLetter(MY_ID));
  Serial.print("] 재송신 예약 ");
  Serial.print(waitMs);
  Serial.println(" ms");
}

static void sendSourcePacket(uint32_t now) {
  FinderPacket packet = {};
  packet.msgId =
      (static_cast<uint32_t>(FINDER_NODE_A) << 24) |
      (sourceSequence & 0x00FFFFFFUL);
  packet.srcId = FINDER_NODE_A;
  packet.facilityType = FINDER_FACILITY_AED;
  packet.status = FINDER_STATUS_OK;
  packet.ttl = FINDER_TTL_INITIAL;
  packet.lastHopId = FINDER_NODE_A;

  if (!fakeRadioSend(packet)) {
    Serial.println("[A] MQTT 미연결: 1초 뒤 발신 재시도");
    nextSourceAtMs = now + 1000;
    return;
  }

  remember(packet.msgId);
  logPacket("발신", packet);
  ++sourceSequence;
  nextSourceAtMs = now + SOURCE_INTERVAL_MS;
}

static void sendPendingRelay(uint32_t now) {
  if (!relayPending || !timeReached(now, pendingSendAtMs)) {
    return;
  }
  if (!fakeRadioSend(pendingPacket)) {
    Serial.println("[mesh] MQTT 미연결: 재송신 500ms 연기");
    pendingSendAtMs = now + 500;
    return;
  }

  logPacket("재송신", pendingPacket);
  relayPending = false;
}

void setup() {
  Serial.begin(115200);
  delay(200);
  randomSeed(micros() ^ (static_cast<uint32_t>(MY_ID) << 16));

  Serial.println();
  Serial.println("=== finder Wokwi mesh node ===");
  Serial.print("NODE: ");
  Serial.println(nodeLetter(MY_ID));

  fakeRadioBegin(MY_ID, HEAR, sizeof(HEAR));
  fakeRadioOnReceive(onPacket);
}

void loop() {
  fakeRadioLoop();

  const uint32_t now = millis();
  if (MY_ID == FINDER_NODE_A && timeReached(now, nextSourceAtMs)) {
    sendSourcePacket(now);
  }
  sendPendingRelay(now);
}
