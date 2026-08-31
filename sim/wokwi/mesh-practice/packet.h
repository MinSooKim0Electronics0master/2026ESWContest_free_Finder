// Wokwi 웹 프로젝트는 저장소 상위 폴더를 참조하지 못하므로 둔 복사본입니다.
// 원본: firmware/common/packet.h
#ifndef FINDER_PACKET_H
#define FINDER_PACKET_H

#include <stdint.h>

#define FINDER_RELAY_DELAY_MIN_MS  50
#define FINDER_RELAY_DELAY_MAX_MS  300
#define FINDER_MSG_CACHE_SIZE      16
#define FINDER_TTL_INITIAL         4

#define FINDER_NODE_A              1
#define FINDER_NODE_B              2
#define FINDER_NODE_C              3
#define FINDER_NODE_D              4

#define FINDER_FACILITY_AED        1
#define FINDER_FACILITY_HYDRANT    2
#define FINDER_FACILITY_TOILET     3
#define FINDER_STATUS_OK           0
#define FINDER_STATUS_MAINT        1

typedef struct __attribute__((packed)) {
  uint32_t msgId;
  uint8_t srcId;
  uint8_t facilityType;
  uint8_t status;
  uint8_t ttl;
  uint8_t lastHopId;
} FinderPacket;

static_assert(sizeof(FinderPacket) == 9, "FinderPacket은 9바이트여야 한다");

#endif
