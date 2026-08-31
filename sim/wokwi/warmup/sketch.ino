// warmup — 역할 변경 전 만든 기초 학습용 스케치입니다.
// 목표: 보드 도착 전에 Arduino 기본기(시리얼, OLED, 버튼)를 익힙니다.
// 배선: OLED는 I2C(SDA=GPIO8, SCL=GPIO9, 주소 0x3C), 버튼은 GPIO4-GND.
//
// TODO 1: setup()에서 Serial.begin(115200) 하고 "warmup 시작"을 출력해
//         시리얼 모니터에서 확인합니다.
// TODO 2: OLED를 초기화합니다.
//         힌트: Adafruit SSD1306 라이브러리의 ssd1306_128x64_i2c 예제에서
//         초기화 부분만 참고 (주소는 0x3C, 크기 128x64).
// TODO 3: loop()에서 1초마다 카운터를 1씩 올려 시리얼과 OLED에 표시합니다.
//         힌트: delay(1000) 으로 시작해도 되고, 익숙해지면 millis() 방식으로.
// TODO 4: 버튼(GPIO4, INPUT_PULLUP — 누르면 LOW)을 누르면 카운터를 0으로.
// 도전:   카운터 값에 비례하는 가로 막대를 OLED에 그립니다(fillRect).
//         → 수신 단말(PR3)의 "신호 세기 바"와 같은 원리입니다.

#include <Wire.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>

void setup() {
  // TODO 1, TODO 2
}

void loop() {
  // TODO 3, TODO 4
}
