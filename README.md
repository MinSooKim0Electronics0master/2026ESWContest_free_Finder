# finder-project — (낯선 곳에서) 필요시설 알리미

낯선 곳에서 긴급제세동기(AED)·소화전·화장실 같은 필요시설의 위치를,
평시에는 오프라인 지도로, 실내 마지막 50 m에서는 BLE로, 재난 시에는
LoRa 메시 네트워크로 안내하는 3계층 알리미입니다.
(임베디드SW 경진대회 출품작)

## 3계층 구조

```
① 평시 · 원거리 ──────────────────────────────────────────────
   스마트폰(GPS) + 오프라인 공공데이터(CSV)
        └─▶ webdemo/  지도에서 최근접 시설 안내

② 실내 · 최후 50 m ───────────────────────────────────────────
   시설 노드 ─(BLE 광고: 시설 종류·상태, 1초 주기)─▶ 스마트폰

③ 재난 모드 · 통신망 붕괴 시 ─────────────────────────────────
   발신 A ──▶ 중계 B ──┐
      │                ├──▶ 수신 D ─(BLE 재방송)─▶ 주변 스마트폰
      └──▶ 중계 C ──┘
   LoRa 메시 플러딩 릴레이. B의 전원이 빠져도 A→C→D로 우회 수신
   유지 — 단일 장애점 제거가 핵심 시연이다.
```

## 폴더 안내

| 폴더 | 내용 | 담당 |
|---|---|---|
| `firmware/node/` | 시설 부착 노드 펌웨어(.ino) — BLE 비콘·LoRa 메시·게이트웨이 | 이휘 구현·민수 검증 |
| `firmware/handset/` | 수신 단말 펌웨어(.ino) — LoRa 수신·OLED 표시 | 이휘 구현·민수 검증 |
| `firmware/common/` | 공유 패킷 규격(`packet.h`) — 노드·단말·시뮬레이터 공통 | 이휘 |
| `sim/` | 메시 릴레이 파이썬 시뮬레이터와 우회 테스트 | 이휘 |
| `webdemo/` | 평시 원거리 안내 카카오맵 데모(단일 HTML) | 이휘 |
| `webdemo/data/` | 공공데이터 CSV 놓는 곳(원본 CSV는 커밋 제외) | 이휘 |
| `data/` | 실측 기록(RSSI CSV 등) | 민수 |
| `hardware/` | 보드·배선·케이스 등 하드웨어 자료 | 공동 |
| `docs/` | 이슈 초안·가이드·문서 | 이휘 |
| `docs/meetings/` | 회의록 | 이휘 |

## 역할 분담

- **이휘(개발 책임자)**: 펌웨어, 패킷 규격, 시뮬레이터, 웹 데모, 문서와
  통합을 담당하며 AI 코딩 보조를 사용할 수 있습니다.
- **김민수(팀원)**: 펌웨어 실행·검증, 실측 데이터 수집, 시연과 발표 준비를
  담당하고 구현 원리를 숙지합니다.
- 제출 자료에는 실제 수행자와 AI 보조 범위를 사실대로 구분합니다.

## 하드웨어

- LILYGO T3 LoRa32 **V1.6.1** × 4대 (ESP32-PICO-D4 + SX1278 +
  0.96" SSD1306 OLED, 433/470 MHz 판)
- 구성: 시설 노드 A, 중계 노드 B/C, 수신 단말 D
- 시험 무선 설정: **433.92 MHz**, 125 kHz, SF7, 4/5, 2 dBm부터 시작
  (규격은 [firmware/common/packet.h](firmware/common/packet.h) 참고)
- 안테나 연결 전에는 송신하지 않으며, 실제 송신 전 KC 적합성평가와
  국내 기술기준 충족 여부를 별도로 확인합니다.

## 빌드 환경 설치 (Arduino IDE)

1. **Arduino IDE 2.x** 설치 — https://www.arduino.cc/en/software
2. 파일 → 기본 설정 → 추가 보드 매니저 URL에 다음 주소를 추가합니다.
   ```
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
   ```
3. 도구 → 보드 → 보드 매니저에서 `esp32`를 검색하여
   **esp32 by Espressif Systems**를 설치하고 **ESP32 Dev Module**을
   선택합니다.
4. 라이브러리 매니저에서 다음 세 라이브러리를 설치합니다.
   - **RadioLib**
   - **Adafruit GFX Library**
   - **Adafruit SSD1306**
5. A/B/C는 [firmware/node/node.ino](firmware/node/node.ino)를 엽니다.
   파일 위쪽의 `FINDER_THIS_NODE_ID`를 각 보드에 맞게 바꿔 업로드합니다.
6. D는 [firmware/handset/handset.ino](firmware/handset/handset.ino)를
   그대로 업로드합니다.
7. USB 연결 후 도구 → 포트에서 해당 포트를 선택합니다. 처음 인식되지 않으면
   장치 관리자에서 CH9102 USB-Serial 드라이버 상태를 확인합니다.

첫 빌드는 ESP32 코어 전체를 컴파일하므로 오래 걸릴 수 있습니다. 보드가
없어도 컴파일까지는 가능하지만, LoRa·OLED·RSSI·우회 경로는 실기 검증이
필요합니다.

## 협업 규칙 (요약)

- 저장소는 민수 소유, `main` 브랜치 보호(직접 push 금지, PR 필수)
- 흐름: 브랜치 생성 → 커밋 → PR → 이휘 리뷰 → 병합
- 민수는 GitHub Desktop, 이휘는 웹/CLI 사용
- 자세한 절차: [docs/github-guide.md](docs/github-guide.md)
- 구현 로드맵(PR 1~5): [docs/tasks.md](docs/tasks.md)
- 실제 역할·기여 기록: [docs/contributions.md](docs/contributions.md)
- 2026-08-31 온라인 인계: [docs/meetings/2026-08-31-online-handoff.md](docs/meetings/2026-08-31-online-handoff.md)
