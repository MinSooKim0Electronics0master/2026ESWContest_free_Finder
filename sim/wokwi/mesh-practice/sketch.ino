// mesh-practice — 김민수 구현 영역입니다.
// 목표: PR4의 릴레이 3규칙을 보드 도착 전에 Wokwi에서 먼저 완성합니다.
//
// fake_radio.h 는 전파를 흉내 내는 연습용 도구(멘토 제공)이고,
// 아래 TODO(메시 규칙)가 실제 과제입니다. 보드가 오면 fakeRadio* 호출부만
// RadioLib으로 바꾸면 여기서 만든 로직은 그대로 씁니다.
//
// 준비: 같은 프로젝트에 packet.h(저장소 firmware/common/packet.h 복사)와
//       fake_radio.h 파일을 만들어 두어야 합니다. 라이브러리 PubSubClient 추가.

#include "packet.h"
#include "fake_radio.h"

// ── 노드 설정: 탭(노드)마다 이 블록만 바꿉니다 ─────────────────────
// 노드   MY_ID   HEAR(들리는 이웃)   역할            (다이아몬드 구성)
//  A      1       {2, 3}             발신
//  B      2       {1, 4}             중계
//  C      3       {1, 4}             중계
//  D      4       {2, 3}             수신 확인 대상
const uint8_t MY_ID  = 1;
const uint8_t HEAR[] = {2, 3};
// ───────────────────────────────────────────────────────────────────

// TODO 1: msgId 캐시를 만듭니다 (규칙 1의 재료)
//   - uint32_t 배열, 크기는 FINDER_MSG_CACHE_SIZE (packet.h에 있음)
//   - "이 msgId를 본 적 있는가" 함수와 "기록" 함수를 만듭니다
//   - 꽉 차면 가장 오래된 것부터 덮어씁니다
//   - 같은 동작이 sim/mesh_sim.py 의 caches 에 있으니 로그로 비교해 보세요

// TODO 2: 수신 처리 함수를 만듭니다
//   void onPacket(const FinderPacket &pkt) {
//     - 캐시에 있으면: "중복 폐기" 출력 후 끝            (규칙 1)
//     - 없으면: 캐시에 기록하고 수신 내용 출력(srcId, ttl)
//     - pkt.ttl 이 1보다 크면: ttl을 1 줄인 사본을 만들어  (규칙 2)
//       50~300ms 뒤에 보내도록 "예약"합니다               (규칙 3)
//       힌트 1: 지연 시간은 random(FINDER_RELAY_DELAY_MIN_MS,
//               FINDER_RELAY_DELAY_MAX_MS + 1)
//       힌트 2: delay()로 멈추면 그동안 수신을 못 합니다.
//               "보낼 패킷"과 "보낼 시각(millis() + 지연)"을 변수에
//               저장해 두고, loop에서 시각이 되면 송신하세요.
//   }

// TODO 3: setup()
//   - Serial.begin(115200);
//   - fakeRadioBegin(MY_ID, HEAR, sizeof(HEAR));
//   - fakeRadioOnReceive(onPacket);

// TODO 4: loop()
//   - fakeRadioLoop(); 를 항상 호출합니다 (수신이 여기서 일어남)
//   - A 노드(MY_ID == 1)만: 10초마다 새 패킷을 만들어 fakeRadioSend
//     - msgId = ((uint32_t)MY_ID << 24) | 시퀀스   (packet.h 규격)
//     - srcId = MY_ID, facilityType = FINDER_FACILITY_AED,
//       status = FINDER_STATUS_OK, ttl = FINDER_TTL_INITIAL, reserved = 0
//     - 보낸 msgId도 자기 캐시에 기록합니다 (되돌아온 메아리 폐기용)
//   - TODO 2에서 예약해 둔 재송신 시각이 지났으면 송신합니다

void setup() {
  // TODO 3
}

void loop() {
  // TODO 4
}
