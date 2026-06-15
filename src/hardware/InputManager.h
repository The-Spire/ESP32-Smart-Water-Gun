#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include <Encoder.h>
#include "../config.h"

/**
 * @brief 输入事件枚举
 *
 * 包含编码器方向事件与扳机的短/长按事件。
 */
enum class InputEvent {
    NONE,                  /**< 无事件 */
    ENCODER_ROTATED_CW,    /**< 编码器顺时针 */
    ENCODER_ROTATED_CCW,   /**< 编码器逆时针 */
    TRIGGER_PRESS,         /**< 扳机短按（按下）——事件触发一次 */
    TRIGGER_RELEASE,       /**< 扳机释放（松开）——事件触发一次 */
    TRIGGER_LONG_PRESS     /**< 扳机长按——按住期间触发一次 */
};

/**
 * @brief 输入管理类
 *
 * 负责编码器与扳机的读取、去抖、长按检测，并通过 `getEvent()`
 * 将单一事件返回给上层逻辑。
 */
class InputManager {
public:
    /**
     * @brief 构造函数
     */
    InputManager();

    /**
     * @brief 初始化输入子系统
     *
     * - 初始化编码器位置
     * - 配置扳机引脚为 `INPUT_PULLUP`
     * - 初始化防抖与长按检测状态
     */
    void init();

    /**
     * @brief 获取当前发生的单一输入事件
     *
     * 优先返回扳机相关事件（去抖 -> 短按/释放 -> 长按），
     * 若无扳机事件则返回编码器旋转事件；否则返回 `NONE`。
     *
     * @return InputEvent
     */
    InputEvent getEvent();

private:
    /* ---------- 编码器相关 ---------- */
    Encoder _encoder;                 /**< 编码器对象 */
    long _lastEncoderPos;             /**< 上一次编码器位置 */
    unsigned long _lastEncoderCheckTime; /**< 上一次编码器读取时间（ms） */

    /* ---------- 扳机按钮相关 ---------- */
    int _triggerState;                /**< 当前稳定状态（HIGH/LOW） */
    int _lastTriggerRawState;         /**< 上一次原始读取（用于去抖） */
    unsigned long _pressStartTime;    /**< 按下时间（ms） */
    unsigned long _debounceTime;      /**< 上次状态跳变时间（ms） */
    bool _longPressHandled;           /**< 长按事件是否已被处理 */

    /* ---------- 常量（类内定义，便于集中管理） ---------- */
    static const unsigned long DEBOUNCE_DELAY_MS = 50;    /**< 去抖延迟（ms） */
    static const unsigned long LONG_PRESS_DELAY_MS = 800; /**< 长按判定阈值（ms） */
    static const int ENCODER_SENSITIVITY = 2;             /**< 编码器灵敏度阈值 */
};

#endif // INPUTMANAGER_H