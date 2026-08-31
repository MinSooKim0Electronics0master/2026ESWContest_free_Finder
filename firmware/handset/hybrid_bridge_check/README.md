# Wokwi-실물 D 하이브리드 시연

Wokwi의 A·B·C 노드가 MQTT로 전달한 `FinderPacket`을 실물 LILYGO D가
받아 실제 OLED와 BLE 광고에 반영하는 시연입니다.

이 시연에서 MQTT는 LoRa 전송을 대체합니다. 실제로 검증되는 것은 ESP32,
OLED, BLE, 패킷 파싱과 메시 우회 로직이며, LoRa 거리·RSSI·전파 릴레이는
검증되지 않습니다.

## 필요한 프로그램과 라이브러리

- Arduino IDE
- 보드 패키지: `esp32 by Espressif Systems` 3.3.11
- PubSubClient 2.8.0
- Adafruit GFX Library
- Adafruit SSD1306
- 휴대전화 nRF Connect 앱

## 실물 보드 설정

LILYGO 제조사 안내에 따라 Arduino IDE의 **도구** 메뉴를 다음과 같이
설정합니다.

| 메뉴 | 값 |
|---|---|
| Board | ESP32 Dev Module |
| CPU Frequency | 240MHz (WiFi/BT) |
| Flash Frequency | 80MHz |
| Flash Mode | QIO |
| Flash Size | 4MB (32Mb) |
| Partition Scheme | Huge APP (3MB No OTA/1MB SPIFFS) |
| PSRAM | Disabled |
| Upload Speed | 921600 |
| Arduino Runs On | Core 1 |
| Events Run On | Core 1 |

PSRAM은 제조사 하드웨어 표에 없는 것으로 표시되어 있으므로 Disabled로
둡니다.

## Wi-Fi 정보 입력

1. `hybrid_secrets.example.h`를 같은 폴더에 복사합니다.
2. 복사한 파일명을 `hybrid_secrets.h`로 바꿉니다.
3. 아래 두 값을 실제 공유기 또는 휴대전화 핫스팟 정보로 바꿉니다.

```cpp
#define FINDER_WIFI_SSID     "실제 Wi-Fi 이름"
#define FINDER_WIFI_PASSWORD "실제 Wi-Fi 비밀번호"
```

`hybrid_secrets.h`는 `.gitignore` 대상이므로 GitHub에 올라가지 않습니다.

## 첫 연결 확인 순서

1. Arduino IDE에서 `hybrid_bridge_check.ino`를 엽니다.
2. 위 보드 설정과 실물 보드의 포트를 선택합니다.
3. 업로드한 뒤 시리얼 모니터를 115200 baud로 엽니다.
4. 다음 로그를 확인합니다.

```text
[D/BLE] 초기 광고 시작
[D/MQTT] Wi-Fi 연결 중... 완료
[D/MQTT] 브로커 연결 및 구독 완료
```

5. Wokwi A를 실행합니다.
6. 첫 패킷에서 다음 로그를 확인합니다.

```text
[D/MQTT] 직접 연결 확인 완료, 이후 B/C 중계만 수신
[D/MQTT] 수신 성공 ... via=A
```

첫 패킷은 Wokwi와 실물 보드의 연결만 확인합니다. 두 번째 패킷부터는 A의
직접 패킷을 무시하고 B 또는 C가 중계한 패킷만 받습니다.

## 우회 시연 순서

1. Wokwi A·B·C 세 프로젝트와 실물 D를 모두 실행합니다.
2. 정상 상태에서 실물 OLED의 `PATH: A -> B -> D`를 확인합니다.
3. Wokwi B 프로젝트의 정지 버튼을 누릅니다.
4. 다음 새 메시지에서 OLED의 `PATH: A -> C -> D`를 확인합니다.
5. nRF Connect에서 `Finder-D` 광고 데이터의 `msgId`도 바뀌는지 확인합니다.

## BLE 시험 데이터 형식

`Finder-D`의 Manufacturer Specific Data는 다음 11바이트입니다.

| 위치 | 크기 | 내용 |
|---|---:|---|
| 0 | 2 | 시험용 식별값 `0xFFFF` |
| 2 | 1 | 버전 `1` |
| 3 | 1 | 시설 종류 |
| 4 | 1 | 상태 |
| 5 | 1 | 원 발신 노드 ID |
| 6 | 1 | 마지막 중계 노드 ID |
| 7 | 4 | `msgId`, ESP32 리틀엔디언 |

`0xFFFF`는 등록된 Bluetooth Company ID라고 주장하지 않는 내부 시험용
식별값입니다.

## 영상과 보고서 표기

화면과 영상에는 다음 사실을 명시합니다.

> A·B·C는 Wokwi 가상 노드이며 MQTT가 LoRa 전달을 대체합니다. D는 실물
> ESP32로 실제 OLED와 BLE를 구동합니다. 실제 LoRa 거리와 RSSI는 미검증입니다.
