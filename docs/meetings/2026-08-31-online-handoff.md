# 2026-08-31 온라인 인계 회의

## 회의 목표

김민수가 GitHub Desktop으로 Finder 저장소 전체를 받고, 구현 브랜치와
주요 코드를 직접 열어 본 뒤 다음 작업을 시작할 수 있는 상태로 만듭니다.
파일을 메신저나 USB로 따로 복사하지 않습니다.

## 시작 전 준비

- 민수 PC에 GitHub Desktop을 설치하고 민수 계정으로 로그인합니다.
- 저장소 초대가 수락되어 있는지 GitHub 웹에서 확인합니다.
- 화면 공유를 켜고 GitHub Desktop을 민수가 직접 조작합니다.
- Wi-Fi 비밀번호는 화면 공유나 채팅에 쓰지 않습니다.

## 1. 저장소 받기

처음 받는 경우입니다.

1. **File → Clone repository...**를 누릅니다.
2. **GitHub.com** 탭에서
   `MinSooKim0Electronics0master/2026ESWContest_free_Finder`를 선택합니다.
3. Local path를 정하고 **Clone**을 누릅니다.
4. **Fetch origin**을 누릅니다.
5. **Current Branch → feature/lilygo-mesh-demo**를 선택합니다.
6. **Repository → Show in Explorer**를 눌러 폴더를 확인합니다.

이미 받은 경우입니다.

1. **Current Repository**에서 Finder 저장소를 선택합니다.
2. **Current Branch → feature/lilygo-mesh-demo**를 선택합니다.
3. **Fetch origin**을 누릅니다.
4. 버튼이 **Pull origin**으로 바뀌면 누릅니다.
5. `No local changes`와 최신 커밋이 보이는지 확인합니다.

## 2. 반드시 열어 볼 파일

| 순서 | 파일 | 확인할 내용 |
|---:|---|---|
| 1 | `firmware/common/packet.h` | 9바이트 패킷, TTL, 마지막 중계 노드 |
| 2 | `sim/wokwi/mesh-practice/sketch.ino` | A 발신, B/C 중계, 캐시와 랜덤 지연 |
| 3 | `sim/wokwi/mesh-practice/fake_radio.h` | LoRa 대신 MQTT를 쓰는 하이브리드 구조 |
| 4 | `firmware/handset/hybrid_bridge_check/README.md` | 실물 D 실행·우회·BLE 확인 절차 |
| 5 | `firmware/handset/hybrid_bridge_check/hybrid_bridge_check.ino` | MQTT 수신을 OLED·BLE에 반영하는 부분 |
| 6 | `firmware/node/node.ino`, `firmware/handset/handset.ino` | 보드 4대 도착 후 사용할 실제 LoRa 펌웨어 |
| 7 | `docs/contributions.md` | 사람과 AI의 실제 기여 구분 |

## 3. 45분 회의 진행표

| 시간 | 할 일 | 완료 표시 |
|---:|---|:---:|
| 0~10분 | Clone 또는 Fetch/Pull, 브랜치 전환 | [ ] |
| 10~20분 | 위 7개 파일의 역할 설명 | [ ] |
| 20~25분 | GitHub 웹에서 이번 구현 PR을 확인하고 민수가 Merge | [ ] |
| 25~30분 | Desktop에서 `main`으로 전환하고 Fetch/Pull | [ ] |
| 30~38분 | Wokwi·실물 D 결과와 `sim/test_detour.py` 확인 | [ ] |
| 38~45분 | 민수가 작은 문서 수정 1건을 별도 브랜치에 Commit·Push | [ ] |

## 4. 민수의 첫 연습 커밋

이 연습은 이번 구현 PR을 `main`에 병합한 다음 시작합니다.

1. **Current Branch → main**을 선택합니다.
2. **Fetch origin**을 누르고, **Pull origin**이 나타나면 누릅니다.
3. **Current Branch → New Branch**를 누릅니다.
4. 브랜치 이름을 `docs/minsu-handoff-check`로 만들고, 기준 브랜치는
   최신 `main`을 선택합니다.
5. 이 문서 맨 아래의 확인자 칸에 이름과 확인 시각을 적습니다.
6. GitHub Desktop Changes에서 이 파일 하나만 체크합니다.
7. Summary에 `docs: 온라인 인계 확인 기록`을 입력하고 Commit합니다.
8. **Publish branch**를 누릅니다.
9. **Preview Pull Request → Create Pull Request**를 눌러 웹에서 PR을 만듭니다.
10. 웹에서 base가 `main`인지 확인하고 PR을 생성한 뒤 이휘에게 링크를
    보냅니다. 이 연습 PR은 바로 병합하지 않습니다.

## 5. 비밀번호 파일 확인

`firmware/handset/hybrid_bridge_check/hybrid_secrets.h`에는 실제 Wi-Fi 정보가
들어가므로 GitHub에 올리지 않습니다. 이 파일은 `.gitignore`에 등록되어
있어 GitHub Desktop Changes에 나타나지 않는 것이 정상입니다. 나타나면
Commit하지 말고 회의를 중지한 뒤 이휘에게 알립니다.

## 회의 종료 조건

- [ ] 민수 PC에 저장소 전체가 Clone되어 있습니다.
- [ ] `feature/lilygo-mesh-demo`의 최신 커밋이 보입니다.
- [ ] 민수가 하이브리드와 실제 LoRa 검증의 차이를 설명할 수 있습니다.
- [ ] 민수의 연습 브랜치와 PR이 GitHub에 보입니다.
- [ ] 실제 Wi-Fi 비밀번호가 Changes와 GitHub에 없습니다.

## 확인자

- 확인자: TODO: 김민수
- 확인 시각: TODO: 2026-08-31 __:__
