#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include "Settings.h"
#include <Preferences.h>

/**
 * @brief 设置管理器
 *
 * 负责从持久化存储加载与保存 `AppSettings`。
 */
class SettingsManager {
public:
    /**
     * @brief 构造函数
     */
    SettingsManager();

    /**
     * @brief 初始化并加载设置（从 Preferences）
     */
    void init();

    AppSettings settings; /**< 公开的设置对象，方便直接访问 */

    /**
     * @brief 将当前 `settings` 保存到闪存
     */
    void save();

private:
    Preferences _preferences;
    const char* _pref_namespace = "watergun"; /**< Preferences 命名空间 */
};

#endif // SETTINGSMANAGER_H