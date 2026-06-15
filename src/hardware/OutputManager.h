#ifndef OUTPUTMANAGER_H
#define OUTPUTMANAGER_H

#include "../config.h"

/**
 * @brief 输出管理器
 *
 * 负责水泵、指示灯、加湿器、激光等外设的控制与周期性更新（如闪烁与泵软启动）。
 */
class OutputManager {
public:
    OutputManager();

    /**
     * @brief 初始化输出引脚与 PWM 通道
     */
    void init();

    /**
     * @brief 周期性更新函数，处理需要定时的任务
     */
    void update();

    void pumpOn();
    void pumpOff();

    void indicatorOn();
    void indicatorOff();

    /**
     * @brief 开始指示灯闪烁
     * @param interval 闪烁间隔（ms），默认 40
     */
    void indicatorStartBlinking(unsigned int interval = 40);

    /**
     * @brief 停止闪烁并关闭指示灯
     */
    void indicatorStopBlinking();

    void humidifierOn();
    void humidifierOff();
    void laserOn();
    void laserOff();

private:
    bool _isBlinking;
    unsigned int _blinkInterval;
    unsigned long _lastBlinkTime;

    bool _isPumpRamping;
    uint8_t _pumpCurrentDuty;
    uint8_t _pumpTargetDuty;
    unsigned long _pumpRampStartTime;
    unsigned int _pumpRampDuration;

    static const uint8_t PUMP_PWM_CHANNEL = 0;
    static const uint32_t PUMP_PWM_FREQUENCY = 2000;
    static const uint8_t PUMP_PWM_RESOLUTION = 8;
};

#endif // OUTPUTMANAGER_H