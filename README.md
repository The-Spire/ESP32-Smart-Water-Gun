# ESP32-C3 智能水枪 (Intelligent Electronic Water Gun)

## 1. 项目概述
本项目是一款基于 **ESP32-C3** 的DIY智能水枪。外形为APEX中的R99刀刃。

---

## 2. 硬件


### 输入
*   **扳机 (Trigger)**: 连接至 **IO9** (低电平触发，支持外部唤醒)。
*   **旋转编码器 (Encoder)**:
    *   A相: **IO2**
    *   B相: **IO1**
    *   功能: 用于菜单导航和数值调节。

### 输出
*   **显示屏**: 0.96寸 **SSD1331** 彩色 OLED (SPI接口)。
    *   CLK: IO7, MOSI: IO8, CS: IO20, DC: IO10
*   **RGB 灯带**: **WS2812B**，连接至 **IO6**。
*   **语音模块**: **JQ8900-16P** (单线串口通信)，连接至 **IO4**。
*   **水泵 (Pump)**: MOSFET驱动，连接至 **IO5**。
*   **激光器 (Laser)**: 连接至 **IO21**。
*   **加湿器 (Humidifier)**: 连接至 **IO0** (用于模拟枪口烟雾/散热效果)。
*   **LED 指示灯**: 连接至 **IO3** (射击同步闪烁)。

---

## 3. 软件

### 目录结构
```text
src/
├── assets/          # 静态资源 (字体 .c, 图片 .c)
├── hardware/        # 硬件驱动层 (Input, Output, LedStrip, Voice)
├── logic/           # 业务逻辑层 (核心状态机 WaterGunLogic)
├── storage/         # 数据持久化层 (SettingsManager)
├── ui/              # UI 表现层
│   ├── screens/     # 各个独立子屏幕的实现 (ScreenMain, ScreenFunc...)
│   └── UIManager    # UI 总调度器
├── config.h         # 全局引脚和参数配置
└── main.cpp         # 程序入口，主循环
```