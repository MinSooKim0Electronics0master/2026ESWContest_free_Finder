#!/usr/bin/env python3
"""연속 패킷 재송신 대기열 회귀 테스트입니다.

Wokwi 서버 없이 단일 대기 슬롯의 누락을 재현하고, 16칸 FIFO 대기열이
같은 입력을 모두 보존하는지 검사합니다. 또한 Wokwi 스케치가 단일 슬롯
구현으로 되돌아가지 않았는지 확인합니다.
"""

from collections import deque
from pathlib import Path
import sys


if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")


MESSAGE_COUNT = 10
ARRIVAL_INTERVAL_MS = 100
RELAY_DELAY_MS = 250
QUEUE_CAPACITY = 16


def simulate_single_slot():
    """수정 전 구조: 대기 중 새 패킷은 즉시 버립니다."""
    pending = None
    sent = []

    for now in range(0, 2001):
        if pending is not None and pending[1] <= now:
            sent.append(pending[0])
            pending = None

        if now % ARRIVAL_INTERVAL_MS != 0:
            continue
        sequence = now // ARRIVAL_INTERVAL_MS + 1
        if sequence > MESSAGE_COUNT:
            continue
        if pending is None:
            pending = (sequence, now + RELAY_DELAY_MS)

    return sent


def simulate_fifo_queue():
    """수정 후 구조: 들어온 패킷을 FIFO 대기열에 보관합니다."""
    pending = deque(maxlen=QUEUE_CAPACITY)
    sent = []

    for now in range(0, 2001):
        while pending and pending[0][1] <= now:
            sent.append(pending.popleft()[0])

        if now % ARRIVAL_INTERVAL_MS != 0:
            continue
        sequence = now // ARRIVAL_INTERVAL_MS + 1
        if sequence > MESSAGE_COUNT:
            continue
        assert len(pending) < QUEUE_CAPACITY, "시험 중 대기열이 가득 찼습니다"
        pending.append((sequence, now + RELAY_DELAY_MS))

    return sent


def test_sketch_uses_queue():
    sketch_path = (
        Path(__file__).parent / "wokwi" / "mesh-practice" / "sketch.ino"
    )
    source = sketch_path.read_text(encoding="utf-8")
    assert "static bool relayPending" not in source
    assert "이전 재송신 대기 중이므로 새 예약 생략" not in source
    assert "enqueueRelay" in source
    assert "sendPendingRelays" in source


def main():
    expected = list(range(1, MESSAGE_COUNT + 1))
    legacy_sent = simulate_single_slot()
    queued_sent = simulate_fifo_queue()

    assert legacy_sent != expected, "단일 슬롯 누락 재현에 실패했습니다"
    assert queued_sent == expected, (
        f"대기열 전달 결과가 다릅니다: {queued_sent}"
    )
    test_sketch_uses_queue()

    legacy_missing = [n for n in expected if n not in legacy_sent]
    print(f"재현: 단일 슬롯 전달={legacy_sent}, 누락={legacy_missing}")
    print(f"통과: 16칸 대기열 전달={queued_sent}")
    print("통과: Wokwi 스케치가 재송신 대기열 구현을 사용합니다")


if __name__ == "__main__":
    main()
