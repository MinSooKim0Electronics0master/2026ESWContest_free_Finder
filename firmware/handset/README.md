# firmware/handset — 수신 단말 펌웨어

> 2026-08-28 역할 변경에 따라 이휘가 구현을 책임지고 AI 코딩 보조를
> 사용할 수 있습니다. 민수는 실행·검증·실측·시연과 동작 원리 숙지를
> 담당합니다. 실제 기여 내역은 제출 문서에 구분해 기록합니다.

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
  (SX127x Receive 예제, RSSI는 수신 후 `getRSSI()` 계열 참고)
- OLED: SSD1306 128×64, I2C 주소 0x3C, SDA 21 / SCL 22.
  전체 핀맵은 [`../node/README.md`](../node/README.md)의 표 참고
- `TODO:` RSSI 이동평균 창 크기(몇 개 평균이 자연스러운지)는 실측하며 결정
