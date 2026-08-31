#ifndef FINDER_LILYGO_T3_V161_H
#define FINDER_LILYGO_T3_V161_H

// LILYGO T3 LoRa32 V1.6.1 (ESP32-PICO-D4 + SX1278) 공통 핀맵.
// 제조사 자료와 보드 실크 인쇄를 기준으로 한다.

#define FINDER_LORA_SCK            5
#define FINDER_LORA_MISO           19
#define FINDER_LORA_MOSI           27
#define FINDER_LORA_CS             18
#define FINDER_LORA_RST            23
#define FINDER_LORA_DIO0           26
#define FINDER_LORA_DIO1           33

#define FINDER_OLED_SDA            21
#define FINDER_OLED_SCL            22
#define FINDER_OLED_ADDRESS        0x3C
#define FINDER_OLED_WIDTH          128
#define FINDER_OLED_HEIGHT         64

#define FINDER_BOARD_LED           25

#endif  // FINDER_LILYGO_T3_V161_H
