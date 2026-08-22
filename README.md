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
| `firmware/node/` | 시설 부착 노드 펌웨어(.ino) — BLE 비콘·LoRa 메시·게이트웨이 | **민수** |
| `firmware/handset/` | 수신 단말 펌웨어(.ino) — LoRa 수신·OLED 표시 | **민수** |
| `firmware/common/` | 공유 패킷 규격(`packet.h`) — 노드·단말·시뮬레이터 공통 | 이령 |
| `sim/` | 메시 릴레이 파이썬 시뮬레이터와 우회 테스트 | 이령 |
| `webdemo/` | 평시 원거리 안내 카카오맵 데모(단일 HTML) | 이령 |
| `webdemo/data/` | 공공데이터 CSV 놓는 곳(원본 CSV는 커밋 제외) | 이령 |
| `data/` | 실측 기록(RSSI CSV 등) | 민수 |
| `hardware/` | 보드·배선·케이스 등 하드웨어 자료 | 공동 |
| `docs/` | 이슈 초안·가이드·문서 | 이령 |
| `docs/meetings/` | 회의록 | 이령 |

## 역할 분담

- **김민수(제작자)**: 제품 펌웨어 전부(`firmware/node/`, `firmware/handset/`),
  실측 데이터 수집. 심사 Q&A에서 답할 사람이 직접 만든다.
- **이령(멘토)**: 패킷 규격, 시뮬레이터, 웹 데모, 문서, PR 리뷰.
  펌웨어 구현 코드는 작성하지 않는다(리뷰 코멘트·질문·개념 힌트만).

## 하드웨어

- Heltec WiFi LoRa 32 **V3** × 4대 (ESP32-S3 + SX1262 + 0.96" OLED,
  863–928 MHz 판)
- 구성: 시설 노드 3대 + 수신 단말 1대
- 무선: KR920 대역 내 **923.0 MHz**, 저출력으로 시작 (규격은
  [firmware/common/packet.h](firmware/common/packet.h) 참고)

## 빌드 환경 설치 (Arduino IDE)

1. **Arduino IDE 2.x** 설치 — https://www.arduino.cc/en/software
2. **Heltec 보드 매니저 등록**: 파일 → 기본 설정 → "추가 보드 매니저 URL"에
   아래 주소 추가
   ```
   https://resource.heltec.cn/download/package_heltec_esp32_index.json
   ```
3. **보드 설치**: 도구 → 보드 → 보드 매니저에서 `heltec esp32` 검색 →
   "Heltec ESP32 Series Arduino Develop Environment" 설치 후, 보드로
   **WiFi LoRa 32(V3)** 선택
4. **라이브러리 설치**: 스케치 → 라이브러리 포함 → 라이브러리 매니저에서
   - **RadioLib** (LoRa 송수신)
   - **Heltec ESP32 Dev-Boards** (보드 내장 OLED 표시용)
   - BLE는 ESP32 기본 스택 사용이라 추가 설치 불필요
5. USB 연결 후 도구 → 포트에서 보드 포트 선택. 업로드가 안 되면 드라이버
   (USB-Serial) 설치 여부 확인
   - `TODO:` 보드 도착 후 실제 포트명·드라이버 종류를 여기에 기록

검증 기록: 2026-08-22, 위 구성(Heltec 코어 3.3.8 + RadioLib 7.7.1)에서
RadioLib `SX1262` + `packet.h`를 포함한 테스트 스케치가 보드
`Heltec WiFi LoRa 32(V3)`용으로 컴파일 통과(프로그램 362 KB, 10%).
첫 빌드는 코어 전체를 컴파일하므로 10분 이상 걸릴 수 있고, 도중에 멈추면
다음 빌드에서 링크 오류(`undefined reference to pinMode` 등)가 날 수
있다 — 그때는 스케치 → "컴파일 출력 지우기" 후 다시 빌드하거나 전체
재빌드(`--clean`)하면 된다.

## 협업 규칙 (요약)

- 저장소는 민수 소유, `main` 브랜치 보호(직접 push 금지, PR 필수)
- 흐름: 브랜치 생성 → 커밋 → PR → 이령 리뷰 → 병합
- 민수는 GitHub Desktop, 이령은 웹/CLI 사용
- 자세한 절차: [docs/github-guide.md](docs/github-guide.md)
- 구현 로드맵(PR 1~5): [docs/tasks.md](docs/tasks.md)
