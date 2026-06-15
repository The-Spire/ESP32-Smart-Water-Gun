#include "SettingsManager.h"

/**
 * @file SettingsManager.cpp
 * @brief 实现 SettingsManager 的加载与保存逻辑
 */

SettingsManager::SettingsManager() {
    // 构造函数：无需额外初始化
}

/**
 * @brief 从 Preferences 加载设置到 `settings`
 *
 * 若偏好项不存在则使用内置默认值。
 */
void SettingsManager::init() {
    _preferences.begin(_pref_namespace, false);

    // 功能开关
    settings.pumpEnabled = _preferences.getBool("pumpEn", true);
    settings.humidifierEnabled = _preferences.getBool("humidEn", true);
    settings.lightsEnabled = _preferences.getBool("lightsEn", true);
    settings.laserEnabled = _preferences.getBool("laserEn", false);

    // 声音设置
    settings.soundEnabled = _preferences.getBool("soundEn", true);
    settings.volume = _preferences.getInt("volume", 20);

    // 子弹设置
    settings.bulletType = (BulletType)_preferences.getInt("bulletType", (int)BulletType::WATER);
    settings.maxAmmo = _preferences.getInt("maxAmmo", 30);
    settings.fireRate = _preferences.getInt("fireRate", 17);
    settings.reloadRate = _preferences.getInt("reloadRate", 17);
    settings.autoReloadDelay = _preferences.getInt("autoReload", 2);

    // 灯光设置
    settings.lightSpeed = _preferences.getInt("lightSpeed", 50);
    settings.lightColor = _preferences.getUShort("lightColor", 0xFD00); // 默认橙色 (RGB565)
    settings.themeColorPrimary = _preferences.getUShort("themePrimary", 0xFFFF);
    settings.themeColorSecondary = _preferences.getUShort("themeSecondary", 0xFD00);

    // 系统信息显示
    settings.showFps = _preferences.getBool("showFps", true);
    settings.showRam = _preferences.getBool("showRam", false);
    settings.showCpu = _preferences.getBool("showCpu", false);

    settings.autoStandbyMinutes = _preferences.getInt("standbyTime", 5);
    _preferences.end();
}

/**
 * @brief 将当前 `settings` 写回 Preferences
 */
void SettingsManager::save() {
    _preferences.begin(_pref_namespace, false);

    _preferences.putBool("pumpEn", settings.pumpEnabled);
    _preferences.putBool("humidEn", settings.humidifierEnabled);
    _preferences.putBool("lightsEn", settings.lightsEnabled);
    _preferences.putBool("laserEn", settings.laserEnabled);

    _preferences.putBool("soundEn", settings.soundEnabled);
    _preferences.putInt("volume", settings.volume);

    _preferences.putInt("bulletType", (int)settings.bulletType);
    _preferences.putInt("maxAmmo", settings.maxAmmo);
    _preferences.putInt("fireRate", settings.fireRate);
    _preferences.putInt("reloadRate", settings.reloadRate);
    _preferences.putInt("autoReload", settings.autoReloadDelay);

    _preferences.putInt("lightSpeed", settings.lightSpeed);
    _preferences.putUShort("lightColor", settings.lightColor);
    _preferences.putUShort("themePrimary", settings.themeColorPrimary);
    _preferences.putUShort("themeSecondary", settings.themeColorSecondary);

    _preferences.putBool("showFps", settings.showFps);
    _preferences.putBool("showRam", settings.showRam);
    _preferences.putBool("showCpu", settings.showCpu);

    _preferences.putInt("standbyTime", settings.autoStandbyMinutes);
    _preferences.end();

    // 调试输出可选：Serial.println("Settings saved!");
}