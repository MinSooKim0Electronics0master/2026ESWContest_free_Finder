# firmware/node — 시설 부착 노드 펌웨어

> **이 폴더는 김민수 구현 영역입니다.**
> 멘토(이령)와 Claude는 이 폴더의 코드를 작성·수정·생성하지 않습니다.
> 도움은 PR 리뷰 코멘트·질문·개념 힌트로만 합니다.

시설(AED·소화전·화장실)에 부착되는 노드 3대의 펌웨어(.ino)를 여기에
만든다. 패킷 규격은 [`../common/packet.h`](../common/packet.h)를 따른다.

## 이 폴더에서 진행할 PR과 완료 조건

| PR | 브랜치 | 내용 | 완료 조건 |
|---|---|---|---|
| 1 | feature/ble-beacon | BLE 광고에 시설 종류·상태 실어 1초 주기 방송 | nRF Connect 앱에서 광고 확인, 코드 수정 시 폰 표시 변경 |
| 2 | feature/lora-p2p | 2대 점대점: 카운터 송신, 수신 측 RSSI 시리얼 출력 | 3지점 RSSI 기록 CSV가 data/에 커밋 |
| 4 | feature/mesh-v1 | 릴레이 3규칙: msgId 캐시 중복 폐기, TTL−1 재송신, 재송신 전 랜덤 지연 50~300 ms | B 전원 제거 시 A→C→D 우회 수신 유지 |
| 5 | feature/ble-gateway | LoRa 최신 수신 메시지를 자기 BLE 광고에 반영 | A발 메시지가 C의 BLE 광고로 폰에서 확인 |

## 참고 자료

- 패킷 규격: [`../common/packet.h`](../common/packet.h) — 필드·상수·주파수
- RadioLib 예제: https://github.com/jgromes/RadioLib/tree/master/examples
  (SX126x 폴더의 Transmit/Receive 예제부터)
- 릴레이 동작 참고: [`../../sim/mesh_sim.py`](../../sim/mesh_sim.py) —
  같은 3규칙을 파이썬으로 모사한 시뮬레이터. 로직이 헷갈리면 여기 로그를
  먼저 돌려 볼 것
- BLE 광고 확인 앱: nRF Connect for Mobile (Android/iOS)

## Heltec WiFi LoRa 32 V3 핀맵 (보드 패키지 3.3.8의 variant 정의에서 확인)

보드를 `Heltec WiFi LoRa 32(V3)`로 선택하면 아래 이름이 **상수로 이미
정의**돼 있어 숫자를 직접 쓸 필요가 없습니다.

| 용도 | 상수 이름 | GPIO | 비고 |
|---|---|---|---|
| LoRa SPI 선택(NSS) | `SS` | 8 | RadioLib `Module(SS, DIO0, RST_LoRa, BUSY_LoRa)` |
| LoRa 인터럽트 | `DIO0` | 14 | 이름은 DIO0이지만 SX1262의 **DIO1** 핀입니다 |
| LoRa 리셋 | `RST_LoRa` | 12 | |
| LoRa BUSY | `BUSY_LoRa` | 13 | |
| LoRa SPI | `SCK` / `MISO` / `MOSI` | 9 / 11 / 10 | 기본 `SPI` 객체가 자동 사용 |
| OLED I2C | `SDA_OLED` / `SCL_OLED` | 17 / 18 | 일반 `SDA/SCL`(41/42)과 다름 |
| OLED 리셋 | `RST_OLED` | 21 | |
| 외부 전원 스위치 | `Vext` | 36 | **LOW로 내려야 OLED에 전원**이 들어감 |
| 내장 LED | `LED` | 35 | |
