/**
 * @file main.cpp
 * @brief 程序入口：初始化子模块并驱动主循环。
 *
 * 目的：保持主循环简洁，初始化顺序清晰。仅整理注释与包含顺序，
 * 不改变程序逻辑或模块间调用关系。
 */

#include <Arduino.h>
#include "config.h"
#include "esp_sleep.h"

#include "hardware/InputManager.h"
#include "hardware/LedStrip.h"
#include "hardware/OutputManager.h"
#include "hardware/VoiceModule.h"

#include "storage/SettingsManager.h"

#include "ui/UIManager.h"
#include "ui/screens/ScreenFuncSettings.h"
#include "ui/screens/ScreenSoundSettings.h"
#include "ui/screens/ScreenBulletSettings.h"
#include "ui/screens/ScreenLightSettings.h"
#include "ui/screens/ScreenThemeSettings.h"
#include "ui/screens/ScreenSysInfo.h"
#include "ui/screens/ScreenMain.h"

#include "logic/WaterGunLogic.h"

// 全局对象（在此文件中定义全局实例，保持原有顺序与依赖）
InputManager    inputManager;
OutputManager   outputManager;
VoiceModule     voiceModule;
LedStrip        ledStrip;

ScreenFuncSettings   screenFuncSettings;
ScreenSoundSettings  screenSoundSettings;
ScreenBulletSettings screenBulletSettings;
ScreenLightSettings  screenLightSettings;
ScreenThemeSettings  screenThemeSettings;
ScreenSysInfo        screenSysInfo;
ScreenMain           screenMain;

UIManager       uiManager(screenMain, screenFuncSettings, screenSoundSettings,
                           screenBulletSettings, screenLightSettings,
                           screenThemeSettings, screenSysInfo);

SettingsManager settingsManager;
WaterGunLogic   logicManager(&inputManager, &uiManager, &outputManager,
                             &voiceModule, &settingsManager, &ledStrip);

// 可选全局指针，某些模块可能引用
WaterGunLogic* g_logicManager = nullptr;

// 性能统计
uint16_t g_frame_count = 0;
unsigned int g_loop_time_us = 0; // 微秒级循环时间
unsigned long lastFpsTime = 0;

/**
 * @brief 进入待机/休眠模式，关闭外设并配置唤醒。
 *
 * 逻辑保持不变：关闭泵、指示灯、加湿器、激光器，清除 LED，设置唤醒并进入轻睡眠。
 */
void enterStandbyMode() {
    outputManager.pumpOff();
    outputManager.indicatorOff();
    outputManager.humidifierOff();
    outputManager.laserOff();
    FastLED.clear(true);
    //uiManager.enterStandbyMode();

    // 禁用并重置唤醒源，使用通用 GPIO 唤醒（适配 ESP32-C3）
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
    esp_sleep_enable_gpio_wakeup();
    // 注意：保持原有引脚与触发逻辑
    gpio_wakeup_enable(GPIO_NUM_9, GPIO_INTR_LOW_LEVEL);

    Serial.println("Entering standby mode...");
    Serial.flush();
    esp_light_sleep_start();
}

void setup() {
    // 保存全局指针（保持与原逻辑一致）
    g_logicManager = &logicManager;

    Serial.begin(115200);

    // 初始化顺序：存储 -> 输入 -> 输出 -> 外设 -> UI -> 逻辑
    settingsManager.init();
    inputManager.init();
    outputManager.init();
    voiceModule.init();
    ledStrip.init();
    delay(500); //显示屏的上电延迟
    uiManager.init();
    logicManager.init();
}

void loop() {
    unsigned long loop_start_time_us = micros();

    // 1) 更新逻辑状态机
    logicManager.update();

    // 2) 更新输出与后台任务
    outputManager.update();
    ledStrip.update();
    uiManager.update(); // 内部会执行 lv_timer_handler()

    // 3) 周期性性能数据更新（保持原有触发条件与计算方法）
    if (logicManager.shouldCalculatePerformanceData()) {
        unsigned long currentTime = millis();
        if (currentTime - lastFpsTime >= 1000) {
            uint16_t fps = g_frame_count;
            g_frame_count = 0;
            lastFpsTime = currentTime;

            multi_heap_info_t heap_info;
            heap_caps_get_info(&heap_info, MALLOC_CAP_DEFAULT);
            size_t total_heap = heap_info.total_free_bytes + heap_info.total_allocated_bytes;

            int ram_percent = 0;
            if (total_heap > 0) {
                ram_percent = (heap_info.total_allocated_bytes * 100) / total_heap;
            }

            uiManager.updateSystemInfo(fps, ram_percent, g_loop_time_us);
        }
    }

    // 记录循环耗时并稍作延时以稳定频率
    g_loop_time_us = micros() - loop_start_time_us;
    delay(1);
}