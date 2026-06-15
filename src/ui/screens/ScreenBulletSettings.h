#ifndef SCREENBULLETSETTINGS_H
#define SCREENBULLETSETTINGS_H

#include <lvgl.h>
#include "../../storage/Settings.h"
#include "../../config.h"

/**
 * @brief 子弹设置屏幕
 *
 * 提供对子弹类型、容量、射速、换弹速度与延迟的查看与编辑。
 */
class ScreenBulletSettings {
public:
    ScreenBulletSettings();

    /**
     * @brief 创建屏幕 UI 对象
     */
    void create(lv_obj_t* parent);

    /**
     * @brief 绑定设置数据指针，用于读写
     */
    void bindData(AppSettings* settings);
    
    void scroll(int8_t direction);
    void action();

    lv_obj_t* getScreenObj();

private:
    lv_obj_t* screen;
    lv_obj_t* title_label;
    lv_obj_t* list_viewport;
    lv_obj_t* items_container;
    lv_obj_t* item_labels[5];
    lv_obj_t* value_labels[5];
    lv_obj_t* focus_bar;
    lv_color_t _primary_color;   /**< 主题主色 */
    lv_color_t _secondary_color; /**< 主题强调色 */

    int selected_index;
    bool is_editing;
    AppSettings* current_settings;

    void updateValues();
    void moveFocusBar(bool animate);
    void applyTheme();
    static void screen_loaded_cb(lv_event_t* e);
    static void start_entry_anim_cb(lv_timer_t* timer);
};

#endif // SCREENBULLETSETTINGS_H