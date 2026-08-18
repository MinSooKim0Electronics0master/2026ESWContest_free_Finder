# sim — 메시 릴레이 시뮬레이터

펌웨어를 굽기 전에 릴레이 3규칙(msgId 캐시 중복 폐기 / TTL−1 재송신 /
재송신 전 랜덤 지연 50~300 ms)이 의도대로 동작하는지 책상에서 확인하는
이산 이벤트 시뮬레이터. [`../firmware/common/packet.h`](../firmware/common/packet.h)의
규격·상수를 그대로 모사한다. **파이썬 3 표준 라이브러리만** 쓰므로 설치할
것이 없다.

## 실행법

Windows에서 `python` 명령이 없다고 나오면 `python` 대신 `py`를 쓰면 된다
(예: `py mesh_sim.py --topology diamond --kill B`).

```bash
# 결정적 시연 구성: 이중 경로에서 B 제거 → A→C→D 우회 수신 확인
python mesh_sim.py --topology diamond --kill B

# 대조군: 일렬 구성에서 B 제거 → 우회로가 없어 D 수신 실패
python mesh_sim.py --topology line4 --kill B

# 옵션: --src/--dst 발신·확인 노드, --seed 랜덤 지연 시드, --ttl 초기 TTL
python mesh_sim.py --topology full4 --seed 7
```

## 테스트

```bash
python test_detour.py
```

- `test_diamond_kill_B_detour`: **핵심 검증** — B 제거 후에도 D 수신 유지
  (시드 20종 반복)
- `test_line4_kill_B_no_detour`: 우회로가 없으면 실패함을 확인하는 대조군
- `test_duplicate_suppression`: 캐시 중복 폐기로 무한 릴레이가 없음을 확인

## 토폴로지

| 이름 | 구성 | 용도 |
|---|---|---|
| `diamond` | A–B–D, A–C–D 이중 경로 | 대회 결정적 시연(단일 장애점 제거) |
| `line4` | A–B–C–D 일렬 | 단일 장애점 대조군 |
| `full4` | 4대 완전 연결 | 중복 폐기 동작 관찰 |

## 단순화한 것 (펌웨어에서 확인할 것)

- 전파 충돌(동시 송신)은 모델링하지 않는다 — 랜덤 지연의 충돌 회피 효과는
  실기기 실험(PR 4)에서 확인
- airtime은 고정 40 ms 가정 — `TODO:` 실측 후 교체
- 메시지 1건만 흘린다 — 주기 송신·복수 발신은 필요해지면 확장
