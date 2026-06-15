#ifndef SCREENSYSINFO_H
#define SCREENSYSINFO_H

#include <lvgl.h>
#include "../../storage/Settings.h"
#include "../../config.h"

class WaterGunLogic;
extern WaterGunLogic* g_logicManager;

class ScreenSysInfo {
public:
    ScreenSysInfo();
    void create(lv_obj_t* parent);
    void bindData(AppSettings* settings);
    
    void scroll(int8_t direction);
    void action();
    void updateData(); // <<< 新增：用于实时更新FPS、RAM等数据

    lv_obj_t* getScreenObj();

private:
    // UI元素
    lv_obj_t* screen;
    lv_obj_t* title_label;
    lv_obj_t* list_viewport;
    lv_obj_t* items_container;
    lv_obj_t* item_labels[4];
    lv_obj_t* value_labels[3]; // 只有前3项有value
    lv_obj_t* dev_logo_img;
    lv_obj_t* dev_name_label;
    lv_obj_t* focus_bar;
    
    // 状态
    int selected_index;
    AppSettings* current_settings;
    
    // 主题颜色
    lv_color_t _primary_color;
    lv_color_t _secondary_color;

    // 私有方法
    void applyTheme();
    void updateValues();
    void moveFocusBar(bool animate);

    // 静态回调
    static void screen_loaded_cb(lv_event_t* e);
    static void start_entry_anim_cb(lv_timer_t* timer);
};

#endif // SCREENSYSINFO_H