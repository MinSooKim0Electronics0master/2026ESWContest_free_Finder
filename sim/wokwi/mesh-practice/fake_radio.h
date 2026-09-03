// fake_radio.h — Wokwi 연습용 "가짜 전파" 모듈 (멘토 제공 도구)
//
// LoRa는 Wokwi에서 시뮬레이션되지 않으므로, 공개 MQTT 브로커를 전파처럼
// 사용합니다. 실제 전파의 두 가지 성질을 흉내 냅니다:
//   1) 자기가 송신한 것은 자기에게 들리지 않는다
//   2) "전파가 닿는 이웃"(HEAR 목록)의 송신만 들린다 → 토폴로지 흉내
//
// 이 파일은 연습용입니다. 제품 펌웨어(firmware/)에는 넣지 않습니다.
// 보드가 도착하면 아래 세 함수 호출부를 RadioLib으로 교체합니다:
//   fakeRadioBegin  → 라디오 초기화 (radio.begin, 주파수 설정)
//   fakeRadioSend   → radio.transmit
//   fakeRadioLoop   → 수신 대기/콜백 처리

#ifndef FAKE_RADIO_H
#define FAKE_RADIO_H

#include <WiFi.h>
#include <PubSubClient.h>
#include <esp_system.h>
#include "packet.h"

// ---- 설정 ------------------------------------------------------------
#define FR_WIFI_SSID "Wokwi-GUEST"        // Wokwi 가상 공유기 (비밀번호 없음)
#define FR_BROKER    "broker.hivemq.com"  // 공개 MQTT 브로커
#define FR_PORT      1883
// 공개 브로커라 같은 토픽을 쓰는 남과 섞일 수 있습니다.
// 실물 D의 hybrid_bridge_check.ino와 반드시 같은 토픽이어야 합니다.
// 공개 브로커이므로 개인정보나 비밀번호는 메시지에 넣지 않습니다.
#define FR_TOPIC     "eswcontest/finder-2026-hybrid-8f4a2c/lora"
// ----------------------------------------------------------------------

typedef void (*FakeRadioHandler)(const FinderPacket &pkt);

static WiFiClient _frWifi;
static PubSubClient _frMqtt(_frWifi);
static uint8_t _frMyId = 0;
static const uint8_t *_frHear = nullptr;
static uint8_t _frHearCount = 0;
static FakeRadioHandler _frHandler = nullptr;
static char _frClientId[48] = {};
static uint32_t _frLastConnectAttemptMs = 0;
static bool _frWasConnected = false;

// MQTT 페이로드 구조: [보낸 노드 id 1바이트][FinderPacket 9바이트]
// 앞의 1바이트는 "지금 송신기를 누른 노드"입니다. 중계된 패킷은 원래
// 발신자(srcId)를 유지하므로, 패킷 내용과 별도로 필요합니다.
static void _frOnMessage(char *topic, byte *payload, unsigned int len) {
  if (len != 1 + sizeof(FinderPacket)) return;
  uint8_t txId = payload[0];
  if (txId == _frMyId) return;              // 성질 1: 자기 송신은 안 들림
  bool audible = false;                     // 성질 2: 이웃 송신만 들림
  for (uint8_t i = 0; i < _frHearCount; i++) {
    if (_frHear[i] == txId) { audible = true; break; }
  }
  if (!audible) return;
  FinderPacket pkt;
  memcpy(&pkt, payload + 1, sizeof(pkt));
  if (pkt.lastHopId != txId) return;
  if (_frHandler) _frHandler(pkt);
}

// myId: 내 노드 번호(1~255), hear: 전파가 닿는 이웃 번호 목록
// 주의: Serial.begin 을 먼저 호출해 두면 연결 과정을 볼 수 있습니다.
inline void fakeRadioBegin(uint8_t myId, const uint8_t *hear, uint8_t hearCount) {
  _frMyId = myId;
  _frHear = hear;
  _frHearCount = hearCount;
  Serial.print("[fake_radio] WiFi 연결 중");
  WiFi.begin(FR_WIFI_SSID, "", 6);          // 채널 6 지정 = Wokwi에서 빠른 연결
  while (WiFi.status() != WL_CONNECTED) { delay(200); Serial.print("."); }
  Serial.println(" 완료");

  // 공개 브로커 전체에서 겹치지 않도록 보드 고유값과 난수를 함께 씁니다.
  // 단순한 millis()만 쓰면 별도 Wokwi 실행이 같은 clientId를 사용하여
  // 서로의 MQTT 연결을 반복해서 끊을 수 있습니다.
  const uint64_t chipId = ESP.getEfuseMac();
  snprintf(
      _frClientId,
      sizeof(_frClientId),
      "finder-%u-%08lx-%08lx",
      static_cast<unsigned int>(_frMyId),
      static_cast<unsigned long>(chipId & 0xFFFFFFFFULL),
      static_cast<unsigned long>(esp_random()));

  _frMqtt.setServer(FR_BROKER, FR_PORT);
  _frMqtt.setCallback(_frOnMessage);
  _frMqtt.setKeepAlive(30);
  _frMqtt.setSocketTimeout(10);
}

// 수신 처리 함수 등록 (수신은 fakeRadioLoop 호출 중에 일어납니다)
inline void fakeRadioOnReceive(FakeRadioHandler h) { _frHandler = h; }

// loop()에서 계속 호출해야 수신·재접속이 됩니다
inline void fakeRadioLoop() {
  if (!_frMqtt.connected()) {
    if (_frWasConnected) {
      Serial.print("[fake_radio] 브로커 연결 끊김, state=");
      Serial.println(_frMqtt.state());
      _frWasConnected = false;
    }

    const uint32_t now = millis();
    if (_frLastConnectAttemptMs != 0 &&
        now - _frLastConnectAttemptMs < 2000) {
      return;
    }
    _frLastConnectAttemptMs = now;

    if (!_frMqtt.connect(_frClientId)) {
      Serial.print("[fake_radio] 브로커 연결 실패, state=");
      Serial.println(_frMqtt.state());
      return;
    }

    if (!_frMqtt.subscribe(FR_TOPIC)) {
      Serial.println("[fake_radio] 토픽 구독 실패");
      _frMqtt.disconnect();
      return;
    }

    _frWasConnected = true;
    Serial.print("[fake_radio] 브로커 연결 완료, clientId=");
    Serial.println(_frClientId);
    return;
  }
  _frMqtt.loop();
}

// 송신. 성공하면 true (RadioLib의 radio.transmit 에 해당)
inline bool fakeRadioSend(const FinderPacket &pkt) {
  if (!_frMqtt.connected()) return false;
  uint8_t buf[1 + sizeof(FinderPacket)];
  buf[0] = _frMyId;
  memcpy(buf + 1, &pkt, sizeof(pkt));
  return _frMqtt.publish(FR_TOPIC, buf, sizeof(buf));
}

#endif // FAKE_RADIO_H
