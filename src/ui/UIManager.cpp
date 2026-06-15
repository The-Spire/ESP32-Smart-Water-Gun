/**
 * @file UIManager.cpp
 * @brief UI 管理器实现：负责 LVGL 界面创建、子菜单切换与显示更新
 */

#include "UIManager.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1331.h>
#include <SPI.h>
#include "../assets/assets.h"
#include "screens/ScreenFuncSettings.h"
#include "screens/ScreenSoundSettings.h"
#include "screens/ScreenBulletSettings.h"
#include "screens/ScreenLightSettings.h"
#include "screens/ScreenThemeSettings.h"
#include "screens/ScreenSysInfo.h"
#include "screens/ScreenMain.h"

extern uint16_t g_frame_count;
// 静态成员和全局变量的定义
static Adafruit_SSD1331 display(OLED_CS, OLED_DC, OLED_RST);
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[SCREEN_WIDTH * SCREEN_HEIGHT];
static lv_disp_drv_t disp_drv;

// 将UIManager实例指针设为静态，以便回调函数可以访问
static UIManager* instance = nullptr;
static const lv_coord_t icon_target_positions_x[7] = {-47, -7, 33, 73, 113, 153, 193};
const char* setting_names[7] = {"子弹设定", "系统信息", "主题设定", "声音设定", "灯光设定", "功能设定", "666"};

/**
 * @brief 构造函数
 */
UIManager::UIManager(ScreenMain& sm,ScreenFuncSettings& sfs, ScreenSoundSettings& sss, ScreenBulletSettings& sbs, ScreenLightSettings& sls,ScreenThemeSettings& sts,ScreenSysInfo& ssi)
    : 
    _screenMain(sm), 
    _screenFuncSettings(sfs), 
    _screenSoundSettings(sss), 
    _screenBulletSettings(sbs), 
    _screenLightSettings(sls), 
    _screenThemeSettings(sts),
    _screenSysInfo(ssi)
        
{
    instance = this;
    _ui._sys_info_fps_visible = false;
    _ui._sys_info_ram_visible = false;
    _ui._sys_info_cpu_visible = false;
}

/**
 * @brief 初始化显示硬件、LVGL 和各子界面
 */
void UIManager::init() {
    // SPI和显示屏初始化
    SPI.begin(OLED_CLK, -1, OLED_MOSI, -1);
    SPI.setFrequency(40000000);
    display.begin(40000000);
    display.fillScreen(0x0000);

    // LVGL初始化
    lv_init();
    lv_disp_draw_buf_init(&draw_buf, buf1, NULL, SCREEN_WIDTH * SCREEN_HEIGHT);
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = SCREEN_WIDTH;
    disp_drv.ver_res = SCREEN_HEIGHT;
    disp_drv.flush_cb = _disp_flush_cb;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    // 创建UI
    
    _screenMain.create(NULL);
    _create_settings_screen();
    _ui.anim_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(_ui.anim_screen, lv_color_hex(0x000000), 0);
    _ui.gif_obj = nullptr; // 初始为空，播放时动态创建以保证每次从头播放
    _screenFuncSettings.create(NULL);
    _screenSoundSettings.create(NULL); // <<< 新增
    _screenBulletSettings.create(NULL); // <<< 新增
    _screenLightSettings.create(NULL); // <<< 新增
    _screenThemeSettings.create(NULL); // <<< 3. 创建UI >>>
    _screenSysInfo.create(NULL);
    _create_sys_info_bar();
    showMainScreen();
    // 默认显示主界面

}

/**
 * @brief 周期性更新：调用 LVGL 的定时处理函数
 */
void UIManager::update() {
    lv_timer_handler();
}

/**
 * @brief 切换到主界面
 */
void UIManager::showMainScreen() {
    lv_scr_load(_screenMain.getScreenObj());
}

/**
 * @brief 切换到设置界面（无参数版本）
 */
void UIManager::showSettingsScreen() {
    lv_scr_load(_ui.settings_screen);
}

/**
 * @brief 在设置界面中滚动选择项
 * @param direction 正数向前，负数向后
 */
void UIManager::scrollSettings(int8_t direction) {
    if (!_ui.animating) {
        _shift_icons(direction);
    }
}

/**
 * @brief 创建并初始化系统信息状态栏的 LVGL 对象
 */
void UIManager::_create_sys_info_bar() {
    // 1. 创建背景容器
    // lv_layer_top() 是一个特殊的父对象，能确保我们的状态栏永远显示在所有界面的最顶层
    _ui.sys_info_bar = lv_obj_create(lv_layer_top());
    lv_obj_remove_style_all(_ui.sys_info_bar);
    // 初始尺寸设为0，它将根据内容自动调整
    lv_obj_set_size(_ui.sys_info_bar, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(_ui.sys_info_bar, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(_ui.sys_info_bar, LV_OPA_90, 0); // 半透明背景
    lv_obj_set_style_radius(_ui.sys_info_bar, 3, 0);
    lv_obj_set_style_pad_all(_ui.sys_info_bar, 2, 0);
    // 初始时完全隐藏
    lv_obj_add_flag(_ui.sys_info_bar, LV_OBJ_FLAG_HIDDEN);

    // 2. 创建文本标签
    _ui.sys_info_label = lv_label_create(_ui.sys_info_bar);
    lv_label_set_text(_ui.sys_info_label, ""); // 初始为空
    // <<< 关键：使用LVGL内置的最小字体，以节省空间 >>>
    // 如果您想用自己的字体，请确保它足够小
    lv_obj_set_style_text_font(_ui.sys_info_label, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_color(_ui.sys_info_label, lv_color_hex(0xFFA500), 0);
}


/**
 * @brief 设置系统信息栏显示项可见性，并立即刷新一次显示
 */
void UIManager::setSystemInfoVisibility(bool show_fps, bool show_ram, bool show_cpu) {
    if (!_ui.sys_info_bar) return;

    // 1. 更新内部状态
    _ui._sys_info_fps_visible = show_fps;
    _ui._sys_info_ram_visible = show_ram;
    _ui._sys_info_cpu_visible = show_cpu;

    // 2. 检查是否需要完全隐藏
    if (!show_fps && !show_ram && !show_cpu) {
        lv_obj_add_flag(_ui.sys_info_bar, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(_ui.sys_info_bar, LV_OBJ_FLAG_HIDDEN);
    }
    
    // 3. 立即用当前（可能过时的）数据刷新一次显示，以调整宽度
    // 注意：我们直接调用 updateSystemInfo，传递-1作为无效数据
    updateSystemInfo(-1, -1, -1);
}
/**
 * @brief 更新系统信息显示（FPS / RAM / CPU）
 *
 * 传入 -1 表示保持上一次有效值不变。
 */
void UIManager::updateSystemInfo(int fps, int ram_percent, int cpu_percent) {
    if (!_ui.sys_info_label || !_ui.sys_info_bar) return;

    // 如果整个条都是隐藏的，就什么都不做
    if (lv_obj_has_flag(_ui.sys_info_bar, LV_OBJ_FLAG_HIDDEN)) {
        return;
    }

    // 静态变量来保存上一次的数据
    static int last_fps = 0, last_ram = 0, last_cpu = 0;
    // 如果传入的是无效数据(-1)，则使用上一次的有效数据
    if (fps != -1) last_fps = fps;
    if (ram_percent != -1) last_ram = ram_percent;
    if (cpu_percent != -1) last_cpu = cpu_percent;

    // 根据内部的可见性状态，动态构建字符串
    char buffer[32] = "";
    bool first_item = true;

    if (_ui._sys_info_fps_visible) {
        sprintf(buffer + strlen(buffer), "F:%d", last_fps);
        first_item = false;
    }
    if (_ui._sys_info_ram_visible) {
        if (!first_item) strcat(buffer, " ");
        sprintf(buffer + strlen(buffer), "R:%d", last_ram);
        first_item = false;
    }
    if (_ui._sys_info_cpu_visible) {
        if (!first_item) strcat(buffer, " ");
        sprintf(buffer + strlen(buffer), "L:%d", last_cpu);
    }
    
    lv_label_set_text(_ui.sys_info_label, buffer);
    // 对齐操作可以在这里进行，确保宽度变化后位置依然正确
    lv_obj_align(_ui.sys_info_bar, LV_ALIGN_BOTTOM_RIGHT, -2, -2);
}
/**
 * @brief 更新主界面弹药计数显示
 */
void UIManager::updateAmmo(int current_ammo, int max_ammo) {
    _screenMain.updateAmmo(current_ammo, max_ammo);
}
/**
 * @brief 返回当前设置界面选中索引
 */
int UIManager::getSelectedSettingIndex() {
    return _ui.selected_index;
}
/**
 * @brief 进入待机模式：清屏并关闭部分外设
 */
void UIManager::enterStandbyMode() {
    // 这里可以访问文件内部的 static display 对象
    display.fillScreen(0x0000); // 清空屏幕
    // Adafruit GFX 库通常没有单独的 display() 函数，fillScreen 之后会自动更新
    // 如果您的屏幕需要，可以加上 display.display()
    pinMode(VOICE_SERIAL_PIN, OUTPUT);
    digitalWrite(VOICE_SERIAL_PIN, LOW);
    delay(20); 
    // 如果库支持关闭背光或进入低功耗模式，可以在这里调用
    //display.enableDisplay(false);
}
/**
 * @brief LVGL 的显示刷新回调，将像素数据写入显示器
 */
void IRAM_ATTR UIManager::_disp_flush_cb(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) {
    uint16_t w = (area->x2 - area->x1 + 1);
    uint16_t h = (area->y2 - area->y1 + 1);
    display.startWrite();
    display.setAddrWindow(area->x1, area->y1, w, h);
    uint16_t *buffer = (uint16_t *)color_p;
    display.writePixels(buffer, w * h, true, false);
    display.endWrite();
    lv_disp_flush_ready(disp_drv);
    g_frame_count++;
}

/**
 * @brief 切换到设置界面并应用主题颜色
 */
void UIManager::showSettingsScreen(AppSettings* settings) {
    // a. 应用颜色
    lv_color_t primary_color, secondary_color;
    primary_color.full = settings->themeColorPrimary;
    secondary_color.full = settings->themeColorSecondary;

    // 应用强调色
    lv_obj_set_style_bg_color(_ui.indicator_bar, secondary_color, 0);
    for (int i = 0; i < 7; ++i) {
        lv_obj_set_style_bg_color(_ui.icons[i], secondary_color, 0);
    }

    // 应用常规色 (这里假设图标字体是白色，如果常规色不是白色会看不见)
    // 为了更好的效果，我们让图标字体颜色保持白色，只改变文本标签颜色
    lv_obj_set_style_text_color(_ui.setting_label, primary_color, 0);
    // b. 加载屏幕
    lv_scr_load(_ui.settings_screen);
}

/**
 * @brief 创建设置界面的 LVGL 对象和图标布局
 */
void UIManager::_create_settings_screen() {
    _ui.settings_screen = lv_obj_create(NULL);
    lv_obj_remove_style_all(_ui.settings_screen);
    lv_obj_set_size(_ui.settings_screen, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(_ui.settings_screen, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(_ui.settings_screen, LV_OPA_COVER, 0); 
    lv_obj_add_event_cb(_ui.settings_screen, _settings_screen_loaded_cb, LV_EVENT_SCREEN_LOADED, NULL);

    const char* icon_symbols[] = {"\uE801", "\uE802", "\uE803", "\uE804", "\uF0EB", "\uF1DE","\uF0EB"};
    for (int i = 0; i < 7; i++) {
        _ui.icons[i] = lv_obj_create(_ui.settings_screen);
        lv_obj_remove_style_all(_ui.icons[i]);
        lv_obj_set_size(_ui.icons[i], 30, 30);
        lv_obj_set_pos(_ui.icons[i], icon_target_positions_x[i], 0);
        lv_obj_set_style_bg_color(_ui.icons[i], lv_color_hex(0xFFA500), 0);
        lv_obj_set_style_bg_opa(_ui.icons[i], LV_OPA_COVER, 0); 
        lv_obj_set_style_radius(_ui.icons[i], 5, 0);
        
        _ui.icon_label = lv_label_create(_ui.icons[i]);
        lv_obj_set_style_text_font( _ui.icon_label, &MyIcon, 0);
        lv_obj_set_style_text_color( _ui.icon_label, lv_color_hex(0xFFFFFF), 0);
        lv_label_set_text( _ui.icon_label, icon_symbols[i]);
        lv_obj_center( _ui.icon_label);
    }

    _ui.indicator_bar = lv_obj_create(_ui.settings_screen);
    lv_obj_remove_style_all(_ui.indicator_bar);
    lv_obj_set_size(_ui.indicator_bar, 5, 21);
    lv_obj_set_style_bg_color(_ui.indicator_bar, lv_color_hex(0xFFA500), 0);
    lv_obj_set_style_bg_opa(_ui.indicator_bar, LV_OPA_COVER, 0);
    lv_obj_set_pos(_ui.indicator_bar, 0, 41);

    _ui.setting_label = lv_label_create(_ui.settings_screen);
    lv_obj_set_style_text_color(_ui.setting_label, lv_color_hex(0xA5A5A5), 0);
    lv_obj_set_style_text_font(_ui.setting_label, &MyFont1, 0);
    lv_obj_set_pos(_ui.setting_label, 15, 41);

    _ui.selected_index = 2; // 默认选中 
    lv_label_set_text(_ui.setting_label, setting_names[_ui.selected_index]);
    _ui.animating = false;
}

/**
 * @brief 设置界面加载完成回调，触发入场动画
 */
void UIManager::_settings_screen_loaded_cb(lv_event_t * e) {
    if (instance) instance->_play_settings_entry_animation();
}

/**
 * @brief 播放设置界面图标入场动画
 */
void UIManager::_play_settings_entry_animation() {
    _ui.animating = true;
    for (int i = 0; i < 7; i++) {
        lv_obj_t * icon = _ui.icons[i];
        lv_coord_t final_x = icon_target_positions_x[i];
        lv_obj_set_x(icon, -30);

        lv_anim_t anim;
        lv_anim_init(&anim);
        lv_anim_set_var(&anim, icon);
        lv_anim_set_values(&anim, -30, final_x);
        lv_anim_set_time(&anim, 200);
        lv_anim_set_delay(&anim, i * 20);
        lv_anim_set_path_cb(&anim, lv_anim_path_ease_out);
        lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_obj_set_x);
        if (i == 5) lv_anim_set_ready_cb(&anim, _entry_anim_ready_cb);
        lv_anim_start(&anim);
    }
}

/**
 * @brief 移动图标以改变选择项
 */
void UIManager::_shift_icons(int8_t direction) {
    if (_ui.animating) return;
    _ui.animating = true;

    _ui.selected_index = (_ui.selected_index + direction + 7) % 7;
    _animate_selection();
    lv_label_set_text(_ui.setting_label, setting_names[_ui.selected_index]);

    for (int i = 0; i < 7; i++) {
        lv_anim_t anim;
        lv_anim_init(&anim);
        lv_anim_set_var(&anim, _ui.icons[i]);
        int target_pos_index = (i - direction + 7) % 7;
        lv_coord_t target_x = icon_target_positions_x[target_pos_index];
        lv_anim_set_values(&anim, lv_obj_get_x(_ui.icons[i]), target_x);
        lv_anim_set_time(&anim, 100);
        lv_anim_set_path_cb(&anim, lv_anim_path_ease_out);
        lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_obj_set_x);
        if (i == 6) {
            lv_anim_set_ready_cb(&anim, _anim_ready_cb);
            lv_anim_set_user_data(&anim, (void*)(intptr_t)direction);
        }
        lv_anim_start(&anim);
    }
}

/**
 * @brief 选择项的视觉反馈动画（指示条 + 文本）
 */
void UIManager::_animate_selection() {
    lv_anim_t bar_anim;
    lv_anim_init(&bar_anim);
    lv_anim_set_var(&bar_anim, _ui.indicator_bar);
    lv_anim_set_values(&bar_anim, 0, -5);
    lv_anim_set_time(&bar_anim, 80);
    lv_anim_set_playback_time(&bar_anim, 80);
    lv_anim_set_path_cb(&bar_anim, lv_anim_path_ease_out);
    lv_anim_set_exec_cb(&bar_anim, (lv_anim_exec_xcb_t)lv_obj_set_x);
    lv_anim_start(&bar_anim);

    lv_anim_del(_ui.setting_label, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_t label_anim;
    lv_anim_init(&label_anim);
    lv_anim_set_var(&label_anim, _ui.setting_label);
    lv_anim_set_values(&label_anim, 41, 61);
    lv_anim_set_time(&label_anim, 80);
    lv_anim_set_playback_time(&label_anim, 80);
    lv_anim_set_path_cb(&label_anim, lv_anim_path_ease_out);
    lv_anim_set_exec_cb(&label_anim, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_start(&label_anim);
}

/**
 * @brief 动画完成回调：调整图标数组顺序并结束动画标记
 */
void UIManager::_anim_ready_cb(lv_anim_t *anim) {
    if (!instance) return;
    int8_t direction = (int8_t)(intptr_t)anim->user_data;
    if (direction > 0) {
        lv_obj_t *first_icon = instance->_ui.icons[0];
        for (int i = 0; i < 6; i++) instance->_ui.icons[i] = instance->_ui.icons[i + 1];
        instance->_ui.icons[6] = first_icon;
    } else {
        lv_obj_t *last_icon = instance->_ui.icons[6];
        for (int i = 6; i > 0; i--) instance->_ui.icons[i] = instance->_ui.icons[i - 1];
        instance->_ui.icons[0] = last_icon;
    }
    for (int i = 0; i < 7; i++) {
        lv_obj_set_x(instance->_ui.icons[i], icon_target_positions_x[i]);
    }
    instance->_ui.animating = false;
}

/**
 * @brief 入场动画完成回调
 */
void UIManager::_entry_anim_ready_cb(lv_anim_t * anim) {
    if (instance) instance->_ui.animating = false;
}
// ====================================================================
// ==================== 功能设定子菜单相关实现 ========================
// ====================================================================

// <<< 新增：创建功能设定界面的函数 >>>
// <<< 新增/修改：子菜单的通用接口实现 >>>
void UIManager::showFuncSettingsScreen(AppSettings* settings) {
    lv_scr_load(_screenFuncSettings.getScreenObj());
    _screenFuncSettings.bindData(settings);
}
void UIManager::showSoundSettingsScreen(AppSettings* settings) {
    lv_scr_load(_screenSoundSettings.getScreenObj());
    _screenSoundSettings.bindData(settings);
}
void UIManager::showBulletSettingsScreen(AppSettings* settings) {
    lv_scr_load(_screenBulletSettings.getScreenObj());
    _screenBulletSettings.bindData(settings);
}
void UIManager::showLightSettingsScreen(AppSettings* settings) {
    lv_scr_load(_screenLightSettings.getScreenObj());
    _screenLightSettings.bindData(settings);
}
void UIManager::showThemeSettingsScreen(AppSettings* settings) {
    lv_scr_load(_screenThemeSettings.getScreenObj());
    _screenThemeSettings.bindData(settings);
}
void UIManager::showSysInfoScreen(AppSettings* settings) {
    lv_scr_load(_screenSysInfo.getScreenObj());
    _screenSysInfo.bindData(settings);
}
void UIManager::scrollSubmenu(int8_t direction) {
    // <<< 修改：根据当前屏幕，转发到不同的模块 >>>
    lv_obj_t* current_screen = lv_scr_act();
    if (current_screen == _screenFuncSettings.getScreenObj()) {
        _screenFuncSettings.scroll(direction);
    } else if (current_screen == _screenSoundSettings.getScreenObj()) {
        _screenSoundSettings.scroll(direction);
    }    // <<< 新增 >>>
    else if (current_screen == _screenBulletSettings.getScreenObj()) {
        _screenBulletSettings.scroll(direction);
    }
    else if (current_screen == _screenLightSettings.getScreenObj()) {
        _screenLightSettings.scroll(direction);
    }
    else if (current_screen == _screenThemeSettings.getScreenObj()) {
        _screenThemeSettings.scroll(direction);
    }
    else if (current_screen == _screenSysInfo.getScreenObj()) {
        _screenSysInfo.scroll(direction);
    }

}

void UIManager::actionSubmenu() {
    lv_obj_t* current_screen = lv_scr_act();
    if (current_screen == _screenFuncSettings.getScreenObj()) {
        _screenFuncSettings.action();
    } else if (current_screen == _screenSoundSettings.getScreenObj()) {
        _screenSoundSettings.action();
    } else if (current_screen == _screenBulletSettings.getScreenObj()) {
        _screenBulletSettings.action();
    } else if (current_screen == _screenLightSettings.getScreenObj()) {
        _screenLightSettings.action();
    } else if (current_screen == _screenThemeSettings.getScreenObj()) {
        _screenThemeSettings.action();
    } else if (current_screen == _screenSysInfo.getScreenObj()) {
        _screenSysInfo.action();
    }
}
void UIManager::showShowcaseAnimation() {
    // 每次播放时重新创建 GIF 对象，确保它从第一帧开始播放
    if (_ui.gif_obj) {
        lv_obj_del(_ui.gif_obj);
    }
    _ui.gif_obj = lv_gif_create(_ui.anim_screen);
    lv_gif_set_src(_ui.gif_obj, &like);
    lv_obj_center(_ui.gif_obj);

    lv_scr_load(_ui.anim_screen);
}
void UIManager::closeShowcaseAnimation() {
    if (_ui.gif_obj) {
        lv_obj_del(_ui.gif_obj); // 彻底删除 GIF 对象，释放那 24KB 的缓存！
        _ui.gif_obj = nullptr;   // 将指针置空，防止悬空指针
    }
}