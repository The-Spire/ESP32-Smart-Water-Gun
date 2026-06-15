/**
 * @file ScreenMain.cpp
 * @brief 主界面实现：弹药显示与进度逻辑
 */

#include "ScreenMain.h"
#include "../../assets/assets.h"

// 颜色常量定义
const lv_color_t ScreenMain::COLOR_BLUE = lv_color_hex(0x0000FF);
const lv_color_t ScreenMain::COLOR_YELLOW = lv_color_hex(0xFFFF00);
const lv_color_t ScreenMain::COLOR_RED = lv_color_hex(0xFF0000);

ScreenMain::ScreenMain() { /* 构造函数保持为空 */ }

lv_obj_t* ScreenMain::getScreenObj() { return screen; }

/**
 * @brief 创建主界面对象与子控件
 */
void ScreenMain::create(lv_obj_t* parent) {
    screen = lv_obj_create(parent);
    lv_obj_remove_style_all(screen);
    lv_obj_set_size(screen, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

    // --- 1. 创建视口 ---
    lv_obj_t* viewport = lv_obj_create(screen);
    lv_obj_remove_style_all(viewport);
    lv_obj_set_size(viewport, 96, 54);
    lv_obj_set_pos(viewport, 0, 3);
    lv_obj_clear_flag(viewport, LV_OBJ_FLAG_SCROLLABLE);

    // --- 2. 创建进度条 ---
    ammo_progress_bar = lv_obj_create(viewport);
    lv_obj_remove_style_all(ammo_progress_bar);
    lv_obj_set_size(ammo_progress_bar, 96, 40);
    lv_obj_set_pos(ammo_progress_bar, 0, 5);
    lv_obj_set_style_bg_opa(ammo_progress_bar, LV_OPA_COVER, 0);

    // --- 3. 创建主背景图片 ---
    ammo_bg_image = lv_img_create(screen);
    lv_obj_set_pos(ammo_bg_image, 0, 3);

    // --- 4. 创建弹药数值标签 (核心修改) ---
    ammo_value_label = lv_label_create(screen);
    
    // a. 给标签一个固定的宽度，足够容纳两位数 (例如22像素)
    lv_obj_set_width(ammo_value_label, 30);
    // b. 设置文本在标签的边界框内居中对齐
    lv_obj_set_style_text_align(ammo_value_label, LV_TEXT_ALIGN_CENTER, 0);
    
    // c. 将整个标签对象在屏幕上居中
    lv_obj_align(ammo_value_label, LV_ALIGN_TOP_MID, 0, 22);
    
    // d. 设置字体
    lv_obj_set_style_text_font(ammo_value_label, &MyFont1, 0);
    // 初始颜色将在 updateAmmo 中设置
}

/**
 * @brief 根据当前弹药更新显示：数字、背景与进度条位置
 */
void ScreenMain::updateAmmo(int current_ammo, int max_ammo) {
    if (max_ammo <= 0) return;

    // 1. 更新数值
    lv_label_set_text_fmt(ammo_value_label, "%d", current_ammo);

    // 2. 计算弹药百分比
    int percent = (current_ammo * 100) / max_ammo;

    // 3. 根据百分比选择背景图片和进度条/数字的颜色
    lv_color_t current_color;
    if (percent > 66) {
        lv_img_set_src(ammo_bg_image, &main_blue);
        current_color = COLOR_BLUE;
    } else if (percent > 32) {
        lv_img_set_src(ammo_bg_image, &main_yellow);
        current_color = COLOR_YELLOW;
    } else {
        lv_img_set_src(ammo_bg_image, &main_red);
        current_color = COLOR_RED;
    }
    
    // <<< 关键修正：同时更新进度条和数字的颜色 >>>
    lv_obj_set_style_bg_color(ammo_progress_bar, current_color, 0);
    lv_obj_set_style_text_color(ammo_value_label, current_color, 0);

    // 4. 根据百分比计算并设置进度条的Y坐标
    int y_pos = 43 - (percent * 38) / 100;
    lv_obj_set_y(ammo_progress_bar, y_pos);
}