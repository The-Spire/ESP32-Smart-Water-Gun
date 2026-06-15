#ifndef SCREENMAIN_H
#define SCREENMAIN_H

#include <lvgl.h>
#include "../../config.h"

/**
 * @brief 主界面屏幕对象
 *
 * 显示弹药进度条及数值，并根据剩余弹药改变背景颜色。
 */
class ScreenMain {
public:
    ScreenMain();

    /**
     * @brief 创建主界面 LVGL 对象
     * @param parent 父对象（通常为 NULL 或根对象）
     */
    void create(lv_obj_t* parent);
    
    /**
     * @brief 更新弹药显示
     * @param current_ammo 当前弹药数
     * @param max_ammo 最大弹药数
     */
    void updateAmmo(int current_ammo, int max_ammo);

    lv_obj_t* getScreenObj();

private:
    /* UI 元素 */
    lv_obj_t* screen;
    lv_obj_t* ammo_bg_image;       /**< 背景图片 */
    lv_obj_t* ammo_progress_bar;   /**< 进度条（通过 Y 位移展示） */
    lv_obj_t* ammo_value_label;    /**< 数值标签 */
    
    /* 预定义颜色 */
    static const lv_color_t COLOR_BLUE;
    static const lv_color_t COLOR_YELLOW;
    static const lv_color_t COLOR_RED;
};

#endif // SCREENMAIN_H