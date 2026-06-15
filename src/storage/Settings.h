#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>

/**
 * @brief 子弹类型枚举
 */
enum class BulletType {
    WATER, /**< 水弹 */
    LASER  /**< 激光 */
};

/**
 * @brief 应用设置集合
 *
 * 将所有可配置项集中在一个结构体中，便于保存/加载与传递。
 */
struct AppSettings {
    /* ---------- 功能开关 ---------- */
    bool pumpEnabled;        /**< 水泵开关 */
    bool humidifierEnabled;  /**< 加湿器开关 */
    bool lightsEnabled;      /**< 灯光开关 */
    bool laserEnabled;       /**< 激光开关 */
    bool soundEnabled;       /**< 声音开关 */
    int volume;              /**< 音量 (0-30) */

    /* ---------- 子弹设定 ---------- */
    BulletType bulletType;   /**< 子弹类型 */
    int maxAmmo;             /**< 最大弹容量 */
    int fireRate;            /**< 射速 (Rounds Per Second) */
    int reloadRate;          /**< 换弹速度 (Rounds Per Second) */
    int autoReloadDelay;     /**< 自动换弹延迟 (Seconds) */

    /* ---------- 灯光与主题 ---------- */
    int lightSpeed;          /**< 灯光速度 */
    uint16_t lightColor;     /**< 灯光颜色（RGB565） */
    uint16_t themeColorPrimary;   /**< 主题主色（RGB565） */
    uint16_t themeColorSecondary; /**< 主题强调色（RGB565） */

    /* ---------- 系统信息显示 ---------- */
    bool showFps;            /**< 显示 FPS */
    bool showRam;            /**< 显示 RAM */
    bool showCpu;            /**< 显示 CPU */
    int autoStandbyMinutes;  /**< 自动待机时间（分钟），0 为禁用 */
};

#endif // SETTINGS_H