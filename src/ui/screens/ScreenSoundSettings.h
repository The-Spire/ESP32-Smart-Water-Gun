#ifndef SCREENSOUNDSETTINGS_H
#define SCREENSOUNDSETTINGS_H

#include <lvgl.h>
#include "../../storage/Settings.h"
#include "../../config.h"

/**
 * @brief 声音设置屏幕
 *
 * 包含音效开关与音量调节。
 */
class ScreenSoundSettings {
public:
    ScreenSoundSettings();
    void create(lv_obj_t* parent);
    void bindData(AppSettings* settings);
    
    /* 输入处理 */
    void scroll(int8_t direction);
    void action();

    lv_obj_t* getScreenObj();

private:
    /* UI 元素 */
    lv_obj_t* screen;
    lv_obj_t* title_label;
    lv_obj_t* list_viewport;
    lv_obj_t* items_container;
    lv_obj_t* item_labels[2];
    lv_obj_t* value_labels[2];
    lv_obj_t* focus_bar;
    lv_color_t _primary_color;
    lv_color_t _secondary_color;

    /* 状态 */
    int selected_index;
    bool is_editing;
    AppSettings* current_settings;

    /* 私有方法 */
    void updateValues();
    void moveFocusBar(bool animate);
    void applyTheme();

    /* 静态回调 */
    static void screen_loaded_cb(lv_event_t* e);
    static void start_entry_anim_cb(lv_timer_t* timer);
};

#endif // SCREENSOUNDSETTINGS_H