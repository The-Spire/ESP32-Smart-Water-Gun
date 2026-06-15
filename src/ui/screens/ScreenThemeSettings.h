#ifndef SCREENTHEMESETTINGS_H
#define SCREENTHEMESETTINGS_H

#include <lvgl.h>
#include "../../storage/Settings.h"
#include "../../config.h"

class WaterGunLogic;
extern WaterGunLogic* g_logicManager;

class ScreenThemeSettings {
public:
    ScreenThemeSettings();
    void create(lv_obj_t* parent);
    void bindData(AppSettings* settings);
    
    void scroll(int8_t direction);
    void action();

    lv_obj_t* getScreenObj();

private:
    // UI元素
    lv_obj_t* screen;
    lv_obj_t* title_label;
    lv_obj_t* item_labels[2];
    lv_obj_t* color_preview_boxes[2]; // 两个颜色预览框
    lv_obj_t* color_picker_bar;
    lv_obj_t* focus_bar;
    lv_color_t _primary_color;   // <<< 新增：存储主题色
    lv_color_t _secondary_color;
    // 状态
    int selected_index;
    bool is_editing;
    AppSettings* current_settings;
    
    static const int COLOR_COUNT = 8;
    static const lv_color_t PRESET_COLORS[COLOR_COUNT];

    // 私有方法
    void updateValues();
    void moveFocusBar(bool animate);
    void _updateColorFromSliderPos();
    void applyTheme(); // <<< 新增：应用主题色的方法
    lv_coord_t  _updateSliderPosFromColor();

    // 静态回调
    static void screen_loaded_cb(lv_event_t* e);
    static void start_entry_anim_cb(lv_timer_t* timer);
};

#endif // SCREENTHEMESETTINGS_H