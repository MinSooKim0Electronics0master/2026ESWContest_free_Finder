// firmware/common/packet.h
// finder-project 공유 패킷 규격 v1
//
// 노드(firmware/node) · 수신 단말(firmware/handset) · 시뮬레이터(sim/mesh_sim.py)
// 가 모두 이 규격을 따른다. 이 파일에는 "규격만" 둔다. 규격을 바꾸면
// sim/mesh_sim.py의 패킷 정의도 반드시 함께 맞출 것.

#ifndef FINDER_PACKET_H
#define FINDER_PACKET_H

#include <stdint.h>

// ---------------------------------------------------------------------------
// 무선 설정
// ---------------------------------------------------------------------------

// 현재 확보한 SX1278 433 MHz 보드의 시험 중심주파수.
// 433.795~434.045 MHz 데이터 전송용 대역 안에서 125 kHz 대역폭을 사용한다.
// 실제 송신 전 KC 적합성평가와 국내 기술기준 충족 여부를 별도로 확인할 것.
#define FINDER_FREQ_MHZ            433.92f
#define FINDER_BANDWIDTH_KHZ       125.0f
#define FINDER_SPREADING_FACTOR    7
#define FINDER_CODING_RATE         5  // RadioLib 값 5는 LoRa 코딩률 4/5
#define FINDER_SYNC_WORD           0x12
#define FINDER_TX_POWER_DBM        2  // SX1278 PA_BOOST에서 지원하는 최저값

// 재송신 전 랜덤 지연 범위(밀리초).
// 같은 패킷을 들은 중계 노드 여럿이 "동시에" 재송신하면 전파가 충돌해
// 모두 유실된다. 각자 다른 시각에 쏘도록 무작위로 벌려 놓는 값이다.
#define FINDER_RELAY_DELAY_MIN_MS  50
#define FINDER_RELAY_DELAY_MAX_MS  300

// msgId 중복 판정 캐시 크기(가장 오래된 것부터 밀어내는 FIFO).
// 노드 3~4대 · 초당 1건 수준의 트래픽에서는 16개면 최근 메시지를 전부
// 기억하고도 남고, MCU RAM도 아낀다(16 × 4바이트 = 64바이트).
#define FINDER_MSG_CACHE_SIZE      16

// TTL(남은 홉 수) 초기값.
// 노드 4대 구성의 최장 경로가 3홉이므로 4면 어떤 경로로도 닿으면서,
// 릴레이가 무한히 도는 것을 막는다.
#define FINDER_TTL_INITIAL         4

// 시연 보드 역할 ID.
#define FINDER_NODE_A              1
#define FINDER_NODE_B              2
#define FINDER_NODE_C              3
#define FINDER_NODE_D              4

// ---------------------------------------------------------------------------
// 필드 값 정의
// ---------------------------------------------------------------------------

// 시설 종류 (facilityType). 0은 "미지정"으로 예약.
#define FINDER_FACILITY_AED        1  // 긴급제세동기
#define FINDER_FACILITY_HYDRANT    2  // 소화전
#define FINDER_FACILITY_TOILET     3  // 화장실

// 시설 상태 (status)
#define FINDER_STATUS_OK           0  // 정상
#define FINDER_STATUS_MAINT        1  // 점검

// ---------------------------------------------------------------------------
// 패킷 구조체
// ---------------------------------------------------------------------------
// packed: 컴파일러가 끼워 넣는 정렬 패딩을 없애, 어떤 보드에서 빌드해도
// 전파에 실리는 바이트 배치가 똑같게 만든다. LoRa는 페이로드가 짧을수록
// 전송 시간(airtime)이 줄어 충돌·전력에 유리하므로 총 9바이트로 최소화했다.
//
// 파이썬 시뮬레이터에서 같은 배치를 읽을 때: struct.unpack("<IBBBBB", ...)

typedef struct __attribute__((packed)) {
    // 메시지 고유 번호. 상위 8비트 = 발신 노드 srcId, 하위 24비트 = 그 노드의
    // 송신 시퀀스. 노드마다 독립적으로 시퀀스를 올려도 전체 네트워크에서
    // 유일해진다. 24비트면 1초 1건 기준 약 194일 치 — 시연·운용에 충분하고,
    // 32비트 한 덩어리라 중복 캐시 비교가 정수 비교 한 번으로 끝난다.
    uint32_t msgId;

    // 발신 노드 ID(1~255). msgId 상위 8비트와 같은 값이지만, 비트 연산 없이
    // 바로 읽기 위한 1바이트짜리 중복 필드다. 노드 4대에 1바이트면 충분.
    uint8_t  srcId;

    // 시설 종류(위 FINDER_FACILITY_*). 종류가 세 가지뿐이므로 1바이트로
    // 충분하고, 255종까지 확장 여유가 있다.
    uint8_t  facilityType;

    // 시설 상태(위 FINDER_STATUS_*). 지금은 0/1 두 값이지만, 나중에 비트
    // 플래그(예: 배터리 부족)로 확장할 수 있게 1바이트를 배정했다.
    uint8_t  status;

    // 남은 홉 수. 중계할 때 1씩 줄이고, 0이 되면 더 이상 재송신하지 않는다.
    // 최대 255홉 — 1바이트로 충분하다. 초기값은 FINDER_TTL_INITIAL.
    uint8_t  ttl;

    // 이 패킷을 마지막으로 송신한 노드 ID. 원 발신 때는 srcId와 같고,
    // 중계할 때마다 자기 ID로 바꾼다. D가 B/C 중 어느 경로로 받았는지
    // 표시하고, 가까운 거리에서 A의 직접 전파를 시연상 제외하는 데 쓴다.
    uint8_t  lastHopId;
} FinderPacket;

// 규격 크기 검증: 패딩이 끼어들면 컴파일 단계에서 바로 잡아낸다.
#ifdef __cplusplus
static_assert(sizeof(FinderPacket) == 9, "FinderPacket은 9바이트여야 한다");
#endif

#endif // FINDER_PACKET_H
