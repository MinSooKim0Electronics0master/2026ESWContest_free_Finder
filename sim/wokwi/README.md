# Wokwi 하이브리드 메시 시연

실물 LoRa 보드 4대가 준비되기 전까지 Wokwi A·B·C 세 노드와 실물 수신
단말 D 한 대를 연결해 메시 우회 로직, 실제 OLED와 실제 BLE를 검증합니다.

Wokwi는 LoRa와 BLE를 직접 시뮬레이션하지 않습니다. Wokwi 노드 사이와
실물 D까지의 패킷 전달은 Wi-Fi/MQTT가 대신합니다. 따라서 이 결과는 메시
프로토콜과 실물 출력의 하이브리드 검증이며 실제 LoRa 거리·RSSI 검증이
아닙니다.

| 폴더 | 내용 | 목표 |
|---|---|---|
| `warmup/` | ESP32-S3 + OLED + 버튼 | Arduino·OLED 기본 동작 연습 |
| `mesh-practice/` | Wokwi A·B·C + MQTT | 캐시·TTL·랜덤 지연·B 제거 우회 |

실물 D 준비 방법은
[`../../firmware/handset/hybrid_bridge_check/README.md`](../../firmware/handset/hybrid_bridge_check/README.md)를
따릅니다.

## Wokwi 프로젝트 만들기

1. [Wokwi](https://wokwi.com)에 로그인합니다.
2. **+ New Project → ESP32**를 선택합니다.
3. `mesh-practice/`의 파일을 프로젝트에 만듭니다.
   - `sketch.ino`
   - `packet.h`
   - `fake_radio.h`
   - `diagram.json`
   - `libraries.txt`
4. `diagram.json`을 저장소 내용으로 교체하면 ESP32 DevKit V4와 시리얼
   모니터 연결이 자동으로 설정됩니다.
5. `libraries.txt`를 넣으면 `PubSubClient`가 설치됩니다. 설치되지 않으면
   Library Manager에서 직접 추가합니다.
6. 프로젝트를 `Finder-A`라는 이름으로 저장합니다.
7. 프로젝트 메뉴의 **Save a Copy**로 `Finder-B`, `Finder-C`를 만듭니다.

## 노드 ID 설정

각 프로젝트의 `sketch.ino` 위쪽 값을 다음처럼 설정합니다.

```cpp
// Finder-A
#define FINDER_WOKWI_NODE_ID FINDER_NODE_A

// Finder-B
#define FINDER_WOKWI_NODE_ID FINDER_NODE_B

// Finder-C
#define FINDER_WOKWI_NODE_ID FINDER_NODE_C
```

한 프로젝트에는 한 줄만 사용합니다. 기본값은 A입니다.

## 첫 연결 확인

1. 실물 D에 `hybrid_bridge_check.ino`를 업로드합니다.
2. 실물 D 시리얼 모니터에서 MQTT 구독 완료를 확인합니다.
3. Wokwi Finder-A를 실행합니다.
4. 첫 패킷이 실물 D에 `via=A`로 나타나는지 확인합니다.

첫 패킷은 인터넷 MQTT 연결 확인용입니다. 이후 실물 D는 A의 직접 패킷을
무시하고 B/C가 중계한 패킷만 수신합니다.

## 정상 경로와 우회 경로

1. Finder-A, Finder-B, Finder-C를 모두 실행합니다.
2. 다음 새 메시지에서 실물 D OLED의 `A -> B -> D`를 확인합니다.
3. Finder-B의 빨간 정지 버튼을 누릅니다.
4. 다음 새 메시지에서 실물 D OLED의 `A -> C -> D`를 확인합니다.
5. Finder-C도 정지하면 새 메시지가 D에 도착하지 않는지 확인합니다.
6. Finder-C를 다시 실행하면 수신이 재개되는지 확인합니다.

정상 상태에서는 시연이 분명하게 보이도록 B가 50~120ms, C가 180~300ms
뒤에 재송신합니다. 두 값 모두 공통 규격의 50~300ms 범위 안입니다.

## Wokwi 한 개로 MQTT 연속 패킷 통합 시험

무료 빌드 대기나 브라우저 제한 때문에 여러 Wokwi 프로젝트를 동시에
실행하기 어려울 때 사용합니다. PC 시험 프로그램이 A 발신과 D 수신을
대신하므로 Wokwi에서는 Finder-C 한 개만 실행합니다.

1. Finder-C의 `sketch.ino`와 `fake_radio.h`를 저장소 최신본으로
   교체합니다.
2. 노드 설정이 `FINDER_NODE_C`인지 확인하고 Finder-C만 실행합니다.
3. 시리얼 모니터에서 `[fake_radio] 브로커 연결 완료`를 확인합니다.
4. 저장소 루트의 PowerShell에서 다음 명령을 실행합니다.

```powershell
py sim\mqtt_integration_test.py
```

5. 안내가 나오면 Enter를 누릅니다.

시험 프로그램은 A로 패킷 10건을 100ms 간격으로 보내고, C가 재송신한
패킷을 D처럼 구독합니다. 다음 세 줄이 모두 나오면 통과입니다.

```text
통과: 연속 패킷 전부 수신
통과: TTL=3, lastHopId=C 및 패킷 필드 일치
통과: 동일 msgId 중복 재송신 없음
```

공개 MQTT 브로커를 사용하므로 개인정보나 비밀번호를 패킷에 넣지
않습니다. 시험 중에는 같은 토픽을 사용하는 다른 Finder 프로젝트를 모두
정지합니다.

`브로커 연결 완료`가 계속 반복되면 MQTT 연결이 끊기는 상태입니다.
최신 `fake_radio.h`는 고유한 clientId를 사용하고, 연결이 다시 끊기면
`state=` 값을 출력합니다. 동일한 Finder-C 프로젝트를 실행한 다른 탭도
모두 정지합니다.

## 완료 기준

- [x] 실물 D가 첫 A 직접 패킷을 받아 MQTT 연결을 확인합니다.
- [x] 이후 메시지는 B 또는 C 경유만 실물 D가 받습니다.
- [x] 정상 상태에서 OLED에 `A -> B -> D`가 표시됩니다.
- [x] B 정지 후 OLED에 `A -> C -> D`가 표시됩니다.
- [x] nRF Connect에서 `Finder-D` BLE 광고의 `msgId` 변경을 확인합니다.
- [x] OLED에 `LINK: MQTT SIM`, `BLE: REAL / RF: TODO`가 표시됩니다.

## 확인 기록

2026-08-29에 다음을 실물 D와 Wokwi A·B·C로 확인했습니다.

- Wokwi A 프로젝트: https://wokwi.com/projects/473666618607702017
- B 정상 동작 시 `A -> B -> D`, B 정지 시 `A -> C -> D`로 우회했습니다.
- B를 다시 실행했을 때 정상 경로가 복구되었습니다.
- nRF Connect에서 `Finder-D`의 Manufacturer data와 마지막 중계 노드
  값이 확인되었습니다.
- B·C 프로젝트 주소는 온라인 인계 회의에서 민수 계정에도 저장합니다.

## 주의

- 공개 MQTT 브로커를 사용하므로 개인정보와 비밀번호를 패킷에 넣지 않습니다.
- `hybrid_secrets.h`는 실물 보드에만 있으며 GitHub에 올리지 않습니다.
- 공개 브로커 또는 인터넷이 멈추면 하이브리드 시연도 멈춥니다.
- Wokwi 값을 실제 RSSI·거리·전파 시험 결과로 기록하지 않습니다.
