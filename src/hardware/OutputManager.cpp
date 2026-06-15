#include "OutputManager.h"

OutputManager::OutputManager() {
    _isBlinking = false;
    _blinkInterval = 28; // 默认 28 ms
    _lastBlinkTime = 0;

    _isPumpRamping = false;
    _pumpCurrentDuty = 0;
    _pumpTargetDuty = 0;
    _pumpRampStartTime = 0;
    _pumpRampDuration = 300; // 软启动时长（ms）
}

void OutputManager::init() {
    pinMode(WATER_PUMP_PIN, OUTPUT);
    pinMode(INDICATOR_LED_PIN, OUTPUT);
    pinMode(HUMIDIFIER_PIN, OUTPUT);
    pinMode(LASER_PIN, OUTPUT);

    ledcSetup(PUMP_PWM_CHANNEL, PUMP_PWM_FREQUENCY, PUMP_PWM_RESOLUTION);
    ledcAttachPin(WATER_PUMP_PIN, PUMP_PWM_CHANNEL);
    analogWrite(WATER_PUMP_PIN, 0);

    digitalWrite(INDICATOR_LED_PIN, LOW);
    digitalWrite(HUMIDIFIER_PIN, LOW);
    digitalWrite(LASER_PIN, LOW);
}

void OutputManager::update() {
    // LED 闪烁处理
    if (_isBlinking) {
        if (millis() - _lastBlinkTime > _blinkInterval) {
            digitalWrite(INDICATOR_LED_PIN, !digitalRead(INDICATOR_LED_PIN));
            _lastBlinkTime = millis();
        }
    }

    // 泵的软启动处理
    if (_isPumpRamping) {
        unsigned long elapsed = millis() - _pumpRampStartTime;
        if (elapsed >= _pumpRampDuration) {
            _pumpCurrentDuty = _pumpTargetDuty;
            analogWrite(WATER_PUMP_PIN, _pumpCurrentDuty);
            _isPumpRamping = false;
        } else {
            uint8_t duty = map(elapsed, 0, _pumpRampDuration, 0, _pumpTargetDuty);
            if (duty != _pumpCurrentDuty) {
                _pumpCurrentDuty = duty;
                analogWrite(WATER_PUMP_PIN, _pumpCurrentDuty);
            }
        }
    }
}

void OutputManager::pumpOn() {
    _pumpTargetDuty = 255;
    _pumpRampStartTime = millis();
    _isPumpRamping = true;
}

void OutputManager::pumpOff() {
    _isPumpRamping = false;
    _pumpCurrentDuty = 0;
    analogWrite(WATER_PUMP_PIN, 0);
}

void OutputManager::indicatorOn() {
    _isBlinking = false; // 停止闪烁，确保稳定点亮
    digitalWrite(INDICATOR_LED_PIN, HIGH);
}

void OutputManager::indicatorOff() {
    _isBlinking = false;
    digitalWrite(INDICATOR_LED_PIN, LOW);
}

void OutputManager::indicatorStartBlinking(unsigned int interval) {
    _isBlinking = true;
    _blinkInterval = interval;
    _lastBlinkTime = millis();
    digitalWrite(INDICATOR_LED_PIN, HIGH);
}

void OutputManager::indicatorStopBlinking() {
    _isBlinking = false;
    digitalWrite(INDICATOR_LED_PIN, LOW);
}

void OutputManager::humidifierOn() {
    digitalWrite(HUMIDIFIER_PIN, HIGH);
}

void OutputManager::humidifierOff() {
    digitalWrite(HUMIDIFIER_PIN, LOW);
}

void OutputManager::laserOn() {
    digitalWrite(LASER_PIN, HIGH);
}

void OutputManager::laserOff() {
    digitalWrite(LASER_PIN, LOW);
}