# 이슈 등록 초안 (PR 1~5)

아래 5건을 GitHub → Issues → New issue 에 그대로 복사해 등록한다.
제목은 "제목" 줄을, 본문은 그 아래 블록 전체를 붙여 넣으면 된다.
등록 후 각 이슈를 해당 PR에서 `Closes #번호`로 연결하면 병합 시 자동으로
닫힌다.

---

## 이슈 1

**제목**: `[PR1] BLE 비콘: 시설 종류·상태 1초 주기 광고`

```markdown
## 내용
노드가 BLE 광고 패킷에 시설 종류(1=AED, 2=소화전, 3=화장실)와
상태(0=정상, 1=점검)를 실어 1초 주기로 방송한다.

- 작업 폴더: `firmware/node/`
- 브랜치: `feature/ble-beacon`
- 값 정의는 `firmware/common/packet.h`의 `FINDER_FACILITY_*`,
  `FINDER_STATUS_*`를 그대로 쓴다

## 완료 조건
- [ ] nRF Connect 앱에서 광고가 보인다
- [ ] 코드에서 시설 종류/상태를 바꾸면 폰 표시가 바뀐다

담당: 이휘(구현) / 민수(검증·실측·시연)
```

---

## 이슈 2

**제목**: `[PR2] LoRa 점대점: 카운터 송신 + RSSI 시리얼 출력`

```markdown
## 내용
보드 2대로 점대점 통신을 확인한다. 한 대는 카운터를 올려 가며 송신,
다른 한 대는 수신할 때마다 카운터 값과 RSSI를 시리얼로 출력한다.

- 작업 폴더: `firmware/node/` (송신), `firmware/handset/` (수신)
- 브랜치: `feature/lora-p2p`
- 주파수·설정: `firmware/common/packet.h`의 `FINDER_FREQ_MHZ`(433.92 MHz),
  저출력으로 시작
- 참고: RadioLib SX127x Transmit/Receive 예제

## 완료 조건
- [ ] 거리가 다른 3지점에서 RSSI를 기록
- [ ] 기록한 CSV가 `data/`에 커밋됨

담당: 이휘(구현) / 민수(검증·실측·시연)
```

---

## 이슈 3

**제목**: `[PR3] 수신 단말 UI: OLED 시설명 + RSSI 이동평균 세기 바`

```markdown
## 내용
수신 단말이 LoRa 패킷을 받아 OLED에 시설명을 표시하고, RSSI의
이동평균으로 신호 세기 바를 그린다. 바는 노드에 다가가면 길어지고
멀어지면 짧아져야 한다.

- 작업 폴더: `firmware/handset/`
- 브랜치: `feature/handset-ui`
- 패킷 해석: `firmware/common/packet.h`의 `FinderPacket`

## 완료 조건
- [ ] 노드 접근/이탈에 따라 세기 바가 증감하는 영상 촬영·공유

담당: 이휘(구현) / 민수(검증·실측·시연)
```

---

## 이슈 4

**제목**: `[PR4] 메시 v1: 릴레이 3규칙 (중복 폐기·TTL·랜덤 지연)`

```markdown
## 내용
플러딩 릴레이를 구현한다. 규칙 3가지:
1. msgId 캐시(16개 FIFO)에 있으면 중복 — 폐기
2. TTL을 1 줄여 재송신, 0이면 재송신 안 함
3. 재송신 전 50~300 ms 랜덤 지연 (동시 송신 충돌 회피)

- 작업 폴더: `firmware/node/`
- 브랜치: `feature/mesh-v1`
- 상수: `firmware/common/packet.h`의 `FINDER_MSG_CACHE_SIZE`,
  `FINDER_TTL_INITIAL`, `FINDER_RELAY_DELAY_MIN/MAX_MS`
- 로직이 헷갈리면 `sim/mesh_sim.py`를 먼저 돌려 로그와 비교
  (`python mesh_sim.py --topology diamond --kill B`)

## 완료 조건
- [ ] 발신 A → 중계 B/C → 수신 D 구성에서 B 전원을 제거해도
      A→C→D로 우회해 수신 유지 (결정적 시연 리허설)

담당: 이휘(구현) / 민수(검증·실측·시연)
```

---

## 이슈 5

**제목**: `[PR5] BLE 게이트웨이: LoRa 최신 수신 메시지를 BLE 광고로 재방송`

```markdown
## 내용
노드가 LoRa로 마지막에 수신한 메시지 내용을 자기 BLE 광고에 반영한다.
재난 모드에서 게이트웨이 역할 — LoRa 메시로 퍼진 정보를 주변 스마트폰이
BLE로 받게 한다.

- 작업 폴더: `firmware/node/`
- 브랜치: `feature/ble-gateway`
- PR1(BLE 광고)과 PR4(메시 수신)를 잇는 작업

## 완료 조건
- [ ] A가 발신한 메시지가 C의 BLE 광고에 실려 폰(nRF Connect)에서 확인됨

담당: 이휘(구현) / 민수(검증·실측·시연)
```
