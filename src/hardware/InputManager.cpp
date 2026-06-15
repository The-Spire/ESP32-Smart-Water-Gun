/**
 * @file InputManager.cpp
 * @brief 输入管理：编码器与扳机处理（防抖、长按检测、编码器方向）
 */

#include "InputManager.h"

/**
 * @brief 构造函数
 *
 * 构造时仅初始化编码器对象，具体引脚与状态在 `init()` 中设置。
 */
InputManager::InputManager() : _encoder(ENCODER_PIN_A, ENCODER_PIN_B) {}

/**
 * @brief 初始化输入子系统
 *
 * - 初始化编码器位置与检查时间
 * - 配置扳机引脚为 `INPUT_PULLUP`
 * - 初始化扳机防抖、长按检测相关状态变量
 */
void InputManager::init() {
    // 编码器初始化
    _lastEncoderPos = -999;
    _encoder.write(0);
    _lastEncoderPos = _encoder.read();
    _lastEncoderCheckTime = 0;

    // 扳机初始化（使用内部上拉）
    pinMode(TRIGGER_PIN, INPUT_PULLUP);
    _triggerState = HIGH;
    _lastTriggerRawState = HIGH;
    _pressStartTime = 0;
    _debounceTime = 0;
    _longPressHandled = false;
}

/**
 * @brief 读取当前输入并返回事件
 *
 * 事件优先级：
 * 1. 扳机事件（去抖 -> 短按/释放 -> 长按）
 * 2. 编码器旋转事件（顺时针/逆时针）
 *
 * @return InputEvent 当前检测到的事件；如无事件返回 `InputEvent::NONE`
 */
InputEvent InputManager::getEvent() {
    // 1) 处理扳机（优先）
    int rawState = digitalRead(TRIGGER_PIN);

    // 去抖：如果读取值变化，重置去抖计时器
    if (rawState != _lastTriggerRawState) {
        _debounceTime = millis();
        _lastTriggerRawState = rawState;
    }

    // 当去抖时间窗口过去且稳定状态发生变化时，报告按下或释放
    if ((millis() - _debounceTime) > DEBOUNCE_DELAY_MS) {
        if (rawState != _triggerState) {
            _triggerState = rawState;

            if (_triggerState == LOW) {
                // 按下：记录按下时间，允许长按检测
                _pressStartTime = millis();
                _longPressHandled = false;
                return InputEvent::TRIGGER_PRESS;
            } else {
                // 松开
                return InputEvent::TRIGGER_RELEASE;
            }
        }
    }

    // 长按检测：当按钮持续按下且尚未处理长按时，判断是否达到长按阈值
    if (_triggerState == LOW && !_longPressHandled) {
        if ((millis() - _pressStartTime) > LONG_PRESS_DELAY_MS) {
            _longPressHandled = true; // 防止重复触发长按事件
            return InputEvent::TRIGGER_LONG_PRESS;
        }
    }

    // 2) 编码器事件（只有在没有扳机事件时才处理）
    if (millis() - _lastEncoderCheckTime > 2) { // 控制读取频率
        long newPos = _encoder.read();
        long diff = newPos - _lastEncoderPos;

        // 超过阈值才视为有效滚动
        if (abs(diff) >= ENCODER_SENSITIVITY) {
            _lastEncoderPos = newPos; // 更新基准位置
            if (diff > 0) {
                return InputEvent::ENCODER_ROTATED_CW;
            } else {
                return InputEvent::ENCODER_ROTATED_CCW;
            }
        }
    }

    // 无事件
    return InputEvent::NONE;
}

