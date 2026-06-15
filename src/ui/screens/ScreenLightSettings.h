#ifndef SCREENLIGHTSETTINGS_H
#define SCREENLIGHTSETTINGS_H

#include <lvgl.h>
#include "../../storage/Settings.h"
#include "../../config.h"

class WaterGunLogic;
extern WaterGunLogic* g_logicManager;

/**
 * @brief 灯光设置屏幕
 *
 * 支持设置呼吸灯速度与颜色，并提供颜色拾取滑块。
 */
class ScreenLightSettings {
public:
    ScreenLightSettings();
    void create(lv_obj_t* parent);
    void bindData(AppSettings* settings);
    
    void scroll(int8_t direction);
    void action();

    lv_obj_t* getScreenObj();

private:
    lv_obj_t* screen;
    lv_obj_t* title_label;
    lv_obj_t* item_labels[2];
    lv_obj_t* value_labels[1];

    lv_obj_t* color_preview_box;
    lv_obj_t* color_picker_bar;
    lv_obj_t* focus_bar;
    lv_color_t _primary_color;
    lv_color_t _secondary_color;
    int selected_index;
    bool is_editing;
    AppSettings* current_settings;
    
    static const int COLOR_COUNT = 8;
    static const lv_color_t PRESET_COLORS[COLOR_COUNT];

    /* 私有方法 */
    void updateValues();
    void moveFocusBar(bool animate);
    void applyTheme();
    void _updateColorFromSliderPos();
    lv_coord_t _updateSliderPosFromColor();

    /* 静态回调 */
    static void screen_loaded_cb(lv_event_t* e);
    static void start_entry_anim_cb(lv_timer_t* timer);
};

#endif // SCREENLIGHTSETTINGS_H