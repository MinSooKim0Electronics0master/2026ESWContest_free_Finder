# firmware/handset — 수신 단말 펌웨어

> **이 폴더는 김민수 구현 영역입니다.**
> 멘토(이령)와 Claude는 이 폴더의 코드를 작성·수정·생성하지 않습니다.
> 도움은 PR 리뷰 코멘트·질문·개념 힌트로만 합니다.

사용자가 들고 다니는 수신 단말 1대의 펌웨어(.ino)를 여기에 만든다.
LoRa로 노드 메시지를 받아 OLED에 표시한다. 패킷 규격은
[`../common/packet.h`](../common/packet.h)를 따른다.

## 이 폴더에서 진행할 PR과 완료 조건

| PR | 브랜치 | 내용 | 완료 조건 |
|---|---|---|---|
| 2 | feature/lora-p2p | (수신 측) 점대점 수신, RSSI 시리얼 출력 | 3지점 RSSI 기록 CSV가 data/에 커밋 |
| 3 | feature/handset-ui | 수신 단말: LoRa 수신 → OLED 시설명 + RSSI 이동평균 세기 바 | 접근/이탈에 따라 바 증감 영상 |

## 참고 자료

- 패킷 규격: [`../common/packet.h`](../common/packet.h)
- RadioLib 예제: https://github.com/jgromes/RadioLib/tree/master/examples
  (SX126x Receive 예제, RSSI는 수신 후 `getRSSI()` 계열 참고)
- OLED: 라이브러리 "Heltec ESP32 Dev-Boards"의 예제부터 확인.
  핀 상수는 `SDA_OLED`(17) / `SCL_OLED`(18) / `RST_OLED`(21)이고,
  **`Vext`(36)를 LOW로 내려야 OLED에 전원이 들어갑니다** — 전체 핀맵은
  [`../node/README.md`](../node/README.md)의 표 참고
- `TODO:` RSSI 이동평균 창 크기(몇 개 평균이 자연스러운지)는 실측하며 결정
