#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ====================== 显示屏 (OLED) 配置 ======================
#define SCREEN_WIDTH  96
#define SCREEN_HEIGHT 64
#define OLED_CLK      7
#define OLED_MOSI     8
#define OLED_CS       20
#define OLED_DC       10
#define OLED_RST      -1 // -1 if not used

// ====================== 输入设备配置 ======================
#define ENCODER_PIN_A 2
#define ENCODER_PIN_B 1
#define TRIGGER_PIN 9 

// ====================== LED 灯带配置 ======================
#define LED_DATA_PIN    6
#define NUM_LEDS        85
#define LED_BRIGHTNESS  255

// ====================== 其他硬件======================
 #define VOICE_SERIAL_PIN    4
 #define INDICATOR_LED_PIN   3
 #define WATER_PUMP_PIN      5
 #define HUMIDIFIER_PIN      0
 #define LASER_PIN           21

#endif // CONFIG_H