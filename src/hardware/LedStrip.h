#ifndef LEDSTRIP_H
#define LEDSTRIP_H

#include <FastLED.h>
#include "../config.h"

/**
 * @brief 灯带流动方向
 */
enum FlowDirection {
    DIRECTION_FORWARD,  /**< 正向 */
    DIRECTION_BACKWARD  /**< 反向 */
};

/**
 * @brief 单个移动光效定义
 *
 * 包含光效位置、宽度、颜色、方向以及运行时状态。
 */
struct MovingLightEffect {
    /* 配置项 */
    int startIndex;      /**< 区域起始索引 */
    int endIndex;        /**< 区域结束索引 */
    int lightWidth;      /**< 高亮段宽度 */
    CRGB color;          /**< 高亮颜色 */
    FlowDirection direction; /**< 流动方向 */

    /* 运行时状态 */
    int currentPosition; /**< 当前高亮起始位置（相对于区域） */
    int regionSize;      /**< 区域大小 */
    unsigned long previousMillis; /**< 上次更新时间(ms) */
    unsigned long updateInterval; /**< 更新间隔(ms) */

    /**
     * @brief 构造函数
     * @param start 区域起始索引
     * @param end 区域结束索引
     * @param width 高亮段宽度
     * @param dir 流动方向
     * @param c 颜色
     * @param interval 更新间隔（ms），默认100
     */
    MovingLightEffect(int start, int end, int width, FlowDirection dir, CRGB c, unsigned long interval = 100);

    /**
     * @brief 更新效果到 leds 缓冲区
     * @param leds 指向全局 LED 缓冲区的指针
     */
    void update(CRGB* leds);

    /**
     * @brief 重置效果到初始状态
     */
    void reset();
};

/**
 * @brief LED 灯带管理类
 *
 * 负责管理多个 MovingLightEffect，并将结果写入 FastLED 缓冲区。
 */
class LedStrip {
public:
    LedStrip();
    void init();
    void update();

    /**
     * @brief 设置所有效果的速度
     * @param speed_setting 速度等级（>0 有效）
     */
    void setAllEffectsSpeed(int speed_setting);

    /**
     * @brief 设置所有效果的颜色（RGB565）
     * @param color_setting 16-bit RGB565 颜色值
     */
    void setAllEffectsColor(uint16_t color_setting);

private:
    CRGB _leds[NUM_LEDS];
    MovingLightEffect _effects[7];
    static const int _effectCount = 7;

    unsigned long _lastUpdateTime; /**< 上次整体显示更新时间(ms) */
};

#endif // LEDSTRIP_H