#!/usr/bin/env python3
"""노드 소실 우회 테스트.

이중 경로 토폴로지(diamond: A–B–D, A–C–D)에서 중계 노드 B를 제거해도
D의 수신이 유지되는지(A→C→D 우회) 검증한다 — 대회 결정적 시연과 같은 구성.

실행:
    python test_detour.py
"""

import sys

import mesh_sim

# Windows 콘솔에서 한국어 출력이 깨지지 않도록 (mesh_sim과 동일)
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")


def test_diamond_normal():
    """정상 상태: 모든 노드가 수신한다."""
    r = mesh_sim.run_sim(topology="diamond", kill=[], verbose=False)
    assert r["delivered"], "정상 상태에서 D가 수신하지 못했다"
    assert set(r["first_rx"]) == {"B", "C", "D"}, \
        f"수신 노드가 예상과 다르다: {sorted(r['first_rx'])}"


def test_diamond_kill_B_detour():
    """핵심: B 제거 시에도 A→C→D 우회로 D 수신 유지. 시드 20종으로 반복."""
    for seed in range(20):
        r = mesh_sim.run_sim(topology="diamond", kill=["B"],
                             seed=seed, verbose=False)
        assert r["delivered"], f"seed={seed}: B 제거 시 D가 수신하지 못했다"
        assert "B" not in r["first_rx"], \
            f"seed={seed}: 제거된 B가 수신했다 — 제거 처리 버그"


def test_line4_kill_B_no_detour():
    """대조군: 일렬(line4)에서는 B 제거 시 우회로가 없어 D 수신 실패."""
    r = mesh_sim.run_sim(topology="line4", kill=["B"], verbose=False)
    assert not r["delivered"], \
        "line4에서 B를 제거했는데 D가 수신했다 — 토폴로지 처리 버그"


def test_duplicate_suppression():
    """중복 폐기: 캐시 덕에 노드당 최대 1회만 재송신 → 총 송신 ≤ 노드 수."""
    r = mesh_sim.run_sim(topology="full4", kill=[], verbose=False)
    assert r["tx_count"] <= 4, \
        f"총 송신 {r['tx_count']}회 — 중복 폐기가 안 되면 무한 릴레이가 된다"


if __name__ == "__main__":
    tests = [test_diamond_normal, test_diamond_kill_B_detour,
             test_line4_kill_B_no_detour, test_duplicate_suppression]
    for t in tests:
        t()
        print(f"통과: {t.__name__} — {t.__doc__.strip().splitlines()[0]}")
    print(f"\n모든 테스트 통과 ({len(tests)}건)")
