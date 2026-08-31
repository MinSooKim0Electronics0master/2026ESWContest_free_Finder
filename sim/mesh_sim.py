#!/usr/bin/env python3
"""finder-project LoRa 메시 릴레이 이산 이벤트 시뮬레이터.

firmware/common/packet.h 의 규격(FinderPacket 필드, 상수)을 그대로 모사해,
펌웨어를 굽기 전에 릴레이 3규칙(캐시 중복 폐기 / TTL−1 재송신 / 재송신 전
랜덤 지연)이 의도대로 동작하는지 책상에서 확인하는 도구다.
표준 라이브러리만 사용한다.

사용 예:
    python mesh_sim.py --topology diamond --kill B
    python mesh_sim.py --topology line4 --kill B      # 우회로가 없어 실패하는 대조군
    python mesh_sim.py --topology diamond --seed 7

테스트:
    python test_detour.py
"""

import argparse
import heapq
import itertools
import random
import sys
from collections import deque
from dataclasses import dataclass, replace

# Windows 콘솔 기본 인코딩(cp949/cp1252 등)에서 한국어 출력이 깨지거나
# UnicodeEncodeError로 죽지 않도록 UTF-8 출력을 강제한다.
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")

# ---------------------------------------------------------------------------
# firmware/common/packet.h 와 반드시 일치시켜야 하는 상수
# (규격이 바뀌면 여기도 같이 고칠 것)
# ---------------------------------------------------------------------------
MSG_CACHE_SIZE = 16        # FINDER_MSG_CACHE_SIZE
RELAY_DELAY_MIN_MS = 50    # FINDER_RELAY_DELAY_MIN_MS
RELAY_DELAY_MAX_MS = 300   # FINDER_RELAY_DELAY_MAX_MS
TTL_INITIAL = 4            # FINDER_TTL_INITIAL
FREQ_MHZ = 433.92          # FINDER_FREQ_MHZ (동작에는 안 쓰이지만 규격 대조용)

# 패킷 1건이 전파를 타는 시간(전파 지연은 0으로 침).
# TODO: 보드 도착 후 실제 SF/BW 설정의 airtime 실측값으로 교체
#       (RadioLib getTimeOnAir 참고)
AIRTIME_MS = 40

# 단순화: 전파 충돌(같은 시각에 두 노드가 송신)은 모델링하지 않는다.
# 랜덤 지연이 충돌을 줄이는 효과는 실기기 실험(PR 4)에서 확인한다.

TOPOLOGIES = {
    # 일렬 4대. B가 죽으면 우회로가 없어 D 수신 실패 — "단일 장애점" 대조군.
    "line4": {
        "A": ["B"],
        "B": ["A", "C"],
        "C": ["B", "D"],
        "D": ["C"],
    },
    # 이중 경로(A–B–D, A–C–D). B가 죽어도 C로 우회 — 대회 결정적 시연 구성.
    "diamond": {
        "A": ["B", "C"],
        "B": ["A", "D"],
        "C": ["A", "D"],
        "D": ["B", "C"],
    },
    # 완전 그래프 4대. 전부 서로 들리는 최상 조건 — 중복 폐기 동작 관찰용.
    "full4": {
        "A": ["B", "C", "D"],
        "B": ["A", "C", "D"],
        "C": ["A", "B", "D"],
        "D": ["A", "B", "C"],
    },
}


@dataclass(frozen=True)
class Packet:
    """FinderPacket 모사. 바이트 배치는 struct.pack('<IBBBBB', ...) 과 대응."""
    msg_id: int         # 상위 8비트 srcId + 하위 24비트 시퀀스
    src_id: int         # 발신 노드 ID (1~255)
    facility_type: int  # 1=AED, 2=소화전, 3=화장실
    status: int         # 0=정상, 1=점검
    ttl: int            # 남은 홉 수
    last_hop_id: int    # 마지막으로 송신한 노드 ID


def make_msg_id(src_id, seq):
    """packet.h 규격: 상위 8비트 srcId, 하위 24비트 시퀀스."""
    return ((src_id & 0xFF) << 24) | (seq & 0xFFFFFF)


def run_sim(topology="diamond", kill=(), src="A", dst="D",
            seed=42, ttl=TTL_INITIAL, verbose=True):
    """메시지 1건이 src에서 출발해 어디까지 퍼지는지 시뮬레이션한다.

    반환: {"delivered": dst 수신 여부, "first_rx": {노드: 첫 수신 시각 ms},
           "tx_count": 총 송신 횟수, "log": 로그 줄 목록}
    """
    adj = TOPOLOGIES[topology]
    killed = set(kill)
    for name in [src, dst, *killed]:
        if name not in adj:
            raise ValueError(f"토폴로지 {topology}에 없는 노드: {name}")
    if src in killed:
        raise ValueError("발신 노드를 제거하면 시뮬레이션할 것이 없다")

    rng = random.Random(seed)
    node_ids = {name: i + 1 for i, name in enumerate(sorted(adj))}
    id_names = {v: k for k, v in node_ids.items()}

    # 노드별 msgId 캐시: 펌웨어와 같은 "고정 크기 FIFO" (가장 오래된 것부터 밀려남)
    caches = {n: deque(maxlen=MSG_CACHE_SIZE) for n in adj}
    first_rx = {}   # 노드 -> 첫 수신 시각(ms)
    tx_count = 0
    log = []

    def emit(t, msg):
        line = f"[{t:5d} ms] {msg}"
        log.append(line)
        if verbose:
            print(line)

    def label(pkt):
        seq = pkt.msg_id & 0xFFFFFF
        return f"{id_names[pkt.src_id]}#{seq:06d}"

    # 이벤트 큐: (시각, 일련번호, 종류, 노드, 패킷, 직전 송신자)
    # 일련번호는 같은 시각 이벤트의 순서를 고정해 재현성을 보장한다.
    order = itertools.count()
    events = []

    def push(t, kind, node, pkt, frm=None):
        heapq.heappush(events, (t, next(order), kind, node, pkt, frm))

    # 발신: src가 t=0에 시퀀스 1번 메시지를 만들어 즉시 송신.
    pkt0 = Packet(msg_id=make_msg_id(node_ids[src], 1), src_id=node_ids[src],
                  facility_type=1, status=0, ttl=ttl,
                  last_hop_id=node_ids[src])
    caches[src].append(pkt0.msg_id)  # 자기 메시지도 캐시에 — 되돌아온 메아리 폐기용
    emit(0, f"{src}: {label(pkt0)} 발신 시작 (TTL {ttl}, 제거된 노드: "
            f"{', '.join(sorted(killed)) or '없음'})")
    push(0, "tx", src, pkt0)

    while events:
        t, _, kind, node, pkt, frm = heapq.heappop(events)

        if kind == "tx":
            if node in killed:
                continue  # 전원이 빠진 노드는 송신하지 못한다
            tx_count += 1
            emit(t, f"{node}: {label(pkt)} 송신 (TTL {pkt.ttl})")
            for nb in adj[node]:
                if nb in killed:
                    continue  # 전원이 빠진 노드는 수신하지 못한다
                push(t + AIRTIME_MS, "rx", nb, pkt, node)
            continue

        # kind == "rx"
        # 릴레이 규칙 ①: msgId 캐시에 있으면 중복 — 폐기
        if pkt.msg_id in caches[node]:
            emit(t, f"{node}: {label(pkt)} 중복 폐기 ({frm} 발 재수신)")
            continue
        caches[node].append(pkt.msg_id)
        first_rx.setdefault(node, t)
        emit(t, f"{node}: {label(pkt)} 첫 수신 ({frm} 경유, TTL {pkt.ttl})")

        # 릴레이 규칙 ②: TTL−1, 0이 되면 재송신하지 않음
        if pkt.ttl - 1 <= 0:
            emit(t, f"{node}: TTL 소진 — 재송신 안 함")
            continue

        # 릴레이 규칙 ③: 재송신 전 랜덤 지연 (동시 송신 충돌 회피)
        delay = rng.randint(RELAY_DELAY_MIN_MS, RELAY_DELAY_MAX_MS)
        emit(t, f"{node}: {delay} ms 뒤 재송신 예약 (TTL {pkt.ttl}→{pkt.ttl - 1})")
        push(t + delay, "tx", node,
             replace(pkt, ttl=pkt.ttl - 1, last_hop_id=node_ids[node]))

    delivered = dst in first_rx
    emit(max(first_rx.values(), default=0),
         f"결과: 수신 노드 {sorted(first_rx)} / 총 송신 {tx_count}회 / "
         f"{dst} 도달 {'성공' if delivered else '실패'}")
    return {"delivered": delivered, "first_rx": first_rx,
            "tx_count": tx_count, "log": log}


def main():
    parser = argparse.ArgumentParser(
        description="finder-project LoRa 메시 릴레이 시뮬레이터")
    parser.add_argument("--topology", choices=sorted(TOPOLOGIES),
                        default="diamond", help="노드 연결 구성 (기본: diamond)")
    parser.add_argument("--kill", nargs="*", default=[], metavar="NODE",
                        help="전원을 제거할 노드 (예: --kill B)")
    parser.add_argument("--src", default="A", help="발신 노드 (기본: A)")
    parser.add_argument("--dst", default="D", help="수신 확인 대상 노드 (기본: D)")
    parser.add_argument("--seed", type=int, default=42,
                        help="랜덤 지연 시드 — 같은 시드는 같은 결과 (기본: 42)")
    parser.add_argument("--ttl", type=int, default=TTL_INITIAL,
                        help=f"TTL 초기값 (기본: {TTL_INITIAL})")
    args = parser.parse_args()

    run_sim(topology=args.topology, kill=args.kill, src=args.src,
            dst=args.dst, seed=args.seed, ttl=args.ttl, verbose=True)


if __name__ == "__main__":
    main()
