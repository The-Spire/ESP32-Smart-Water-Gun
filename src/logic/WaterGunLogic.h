#ifndef WATERGUNLOGIC_H
#define WATERGUNLOGIC_H

#include "../hardware/InputManager.h"
#include "../hardware/OutputManager.h"
#include "../hardware/VoiceModule.h"
#include "../storage/SettingsManager.h" // <<< 新增

class UIManager; // <<< 修正：使用前向声明
class LedStrip; // <<< 新增：前向声明
// <<< 修改：添加 STATE_MAIN_EMPTY 状态 >>>
enum class AppState {
    STATE_MAIN_SCREEN,
    STATE_MAIN_FIRING,
    STATE_MAIN_EMPTY,       // 新增：弹药为空的待机状态
    STATE_MAIN_RELOADING,
    STATE_SETTINGS_SCREEN,
    STATE_SETTINGS_SUBMENU_FUNC, // 新增：功能设定子菜单状态
    STATE_SETTINGS_SUBMENU_SOUND,
    STATE_SETTINGS_SUBMENU_BULLET, // <<< 新增
    STATE_SETTINGS_SUBMENU_LIGHT, // <<< 新增
    STATE_SETTINGS_SUBMENU_THEME,
    STATE_SETTINGS_SUBMENU_SYSINFO,
    STATE_SHOWCASE_ANIM // <<< 新增：展示动画状态
};

enum class ReloadPhase {
    IDLE,
    EMPTY_DELAY,
    REFILLING
};
void enterStandbyMode();
class WaterGunLogic {
public:
    WaterGunLogic(InputManager* input, UIManager* ui, OutputManager* output, VoiceModule* voice, SettingsManager* settings, LedStrip* led);
    void init();
    void update();
    void updateLedStripFromSettings();
    void applyAndSaveAllSettings(); // <<< 新增：一个统一的应用和保存函数
    void updateLiveSettings(AppSettings* live_settings);
    bool shouldCalculatePerformanceData() const;
    void resetStandbyTimer(); // <<< 新增：公共接口，用于重置计时器

private:

    InputManager* _inputManager;
    UIManager* _uiManager;
    OutputManager* _outputManager;
    VoiceModule* _voiceModule;
    SettingsManager* _settingsManager; // <<< 新增
    LedStrip* _ledStrip; // <<< 修改类型
    AppState _currentState;
    bool _isWaitingForTriggerReleaseInMenu;
    bool _performance_data_enabled;
    bool _laser_is_on;
    // 游戏逻辑变量
    int _currentAmmo;
    unsigned long _lastShotTime;
    
    ReloadPhase _reloadPhase;
    unsigned long _reloadPhaseStartTime;

    // <<< 新增：用于自动换弹的计时器 >>>
    unsigned long _emptyStateEnterTime;
    unsigned long _last_activity_time;
    unsigned long _anim_start_time; // <<< 新增：记录动画开始的时间
};

#endif // WATERGUNLOGIC_H