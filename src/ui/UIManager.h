#ifndef UIMANAGER_H
#define UIMANAGER_H

#include <lvgl.h>
#include "../config.h"
#include "../storage/Settings.h"
#include "screens/ScreenMain.h"

class ScreenFuncSettings;
class ScreenSoundSettings;
class ScreenBulletSettings;
class ScreenLightSettings;
class ScreenThemeSettings;
class ScreenSysInfo;

/**
 * @brief UI 管理器
 *
 * 负责创建、加载与切换 LVGL 屏幕，以及提供通用的子菜单显示接口。
 */
class UIManager {
public:
    /**
     * @brief 构造函数
     * @param sm 主界面实例引用
     * @param sfs 功能设置界面引用
     * @param sss 声音设置界面引用
     * @param sbs 子弹设置界面引用
     * @param sls 灯光设置界面引用
     * @param sts 主题设置界面引用
     * @param ssi 系统信息界面引用
     */
    UIManager(ScreenMain& sm, ScreenFuncSettings& sfs, ScreenSoundSettings& sss, ScreenBulletSettings& sbs, ScreenLightSettings& sls, ScreenThemeSettings& sts, ScreenSysInfo& ssi);

    /**
     * @brief 初始化显示硬件与 LVGL 屏幕
     */
    void init();

    /**
     * @brief 周期性更新（调用 lv_timer_handler）
     */
    void update();

    /* ---------- 主/设置 界面接口 ---------- */
    void showMainScreen();
    void showSettingsScreen();
    int getSelectedSettingIndex();
    void scrollSettings(int8_t direction);
    void enterStandbyMode();

    /* ---------- 子菜单通用接口 ---------- */
    void showFuncSettingsScreen(AppSettings* settings);
    void showSoundSettingsScreen(AppSettings* settings);
    void showBulletSettingsScreen(AppSettings* settings);
    void showLightSettingsScreen(AppSettings* settings);
    void showThemeSettingsScreen(AppSettings* settings);
    void showShowcaseAnimation();
    void closeShowcaseAnimation();
    void showSettingsScreen(AppSettings* settings);
    void showSysInfoScreen(AppSettings* settings);
    void scrollSubmenu(int8_t direction);
    void actionSubmenu();

    /* ---------- 数据更新接口 ---------- */
    void updateFps(int fps);
    void updateSystemInfo(int fps, int ram_percent, int cpu_percent);
    void setSystemInfoVisibility(bool show_fps, bool show_ram, bool show_cpu);
    void updateAmmo(int current_ammo, int max_ammo);

private:
    /**
     * @brief 内部 UI 元素集合（用于组织 LVGL 对象）
     */
    struct {
        lv_obj_t *settings_screen;
        lv_obj_t *icons[7];
        lv_obj_t *icon_label;
        lv_obj_t *indicator_bar;
        lv_obj_t *setting_label;

        /* 系统信息状态栏 */
        lv_obj_t* sys_info_bar;
        lv_obj_t* sys_info_label;
        bool _sys_info_fps_visible;
        bool _sys_info_ram_visible;
        bool _sys_info_cpu_visible;

        int8_t selected_index;
        bool animating;

        /* 动画屏幕相关 */
        lv_obj_t *anim_screen;
        lv_obj_t *gif_obj;
    } _ui;

    /* 子界面模块实例引用 */
    ScreenFuncSettings& _screenFuncSettings;
    ScreenSoundSettings& _screenSoundSettings;
    ScreenBulletSettings& _screenBulletSettings;
    ScreenLightSettings& _screenLightSettings;
    ScreenThemeSettings& _screenThemeSettings;
    ScreenSysInfo& _screenSysInfo;
    ScreenMain& _screenMain;

    /* 私有方法 */
    void _create_main_screen();
    void _create_settings_screen();
    void _animate_selection();
    void _shift_icons(int8_t direction);
    void _play_settings_entry_animation();
    void _create_sys_info_bar();

    /* 静态回调函数 */
    static void _disp_flush_cb(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
    static void _anim_ready_cb(lv_anim_t *anim);
    static void _entry_anim_ready_cb(lv_anim_t *anim);
    static void _settings_screen_loaded_cb(lv_event_t * e);
};

#endif // UIMANAGER_H