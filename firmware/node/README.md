# firmware/node — 시설 부착 노드 펌웨어

> 2026-08-28 역할 변경에 따라 이휘가 구현을 책임지고 AI 코딩 보조를
> 사용할 수 있습니다. 민수는 실행·검증·실측·시연과 동작 원리 숙지를
> 담당합니다. 실제 기여 내역은 제출 문서에 구분해 기록합니다.

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
  (SX127x 폴더의 Transmit/Receive 예제부터)
- 릴레이 동작 참고: [`../../sim/mesh_sim.py`](../../sim/mesh_sim.py) —
  같은 3규칙을 파이썬으로 모사한 시뮬레이터. 로직이 헷갈리면 여기 로그를
  먼저 돌려 볼 것
- BLE 광고 확인 앱: nRF Connect for Mobile (Android/iOS)

## LILYGO T3 LoRa32 V1.6.1 핀맵

| 용도 | 상수 이름 | GPIO | 비고 |
|---|---|---|---|
| LoRa SPI 선택(NSS) | `LORA_CS` | 18 | RadioLib `Module(18, 26, 23, 33)` |
| LoRa 인터럽트 DIO0 | `LORA_DIO0` | 26 | SX1278 수신·송신 완료 인터럽트 |
| LoRa 리셋 | `LORA_RST` | 23 | |
| LoRa DIO1 | `LORA_DIO1` | 33 | RadioLib 보조 인터럽트 |
| LoRa SPI | `SCK` / `MISO` / `MOSI` | 5 / 19 / 27 | `SPI.begin()`에서 명시 |
| OLED I2C | `SDA` / `SCL` | 21 / 22 | SSD1306 주소 0x3C |
| 내장 LED | `LED` | 25 | |

안테나를 먼저 연결한 뒤 전원을 켜고 송신합니다.
