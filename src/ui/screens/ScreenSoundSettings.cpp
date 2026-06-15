/**
 * @file ScreenSoundSettings.cpp
 * @brief 声音设置界面实现
 */

#include "ScreenSoundSettings.h"
#include "../../logic/WaterGunLogic.h"
#include "../../assets/assets.h"

ScreenSoundSettings::ScreenSoundSettings() {
    screen = nullptr;
    current_settings = nullptr;
    selected_index = -1;
    is_editing = false;
    _primary_color.full = 0xFFFF;
    _secondary_color.full = 0xFD00;
}

lv_obj_t* ScreenSoundSettings::getScreenObj() {
    return screen;
}

/**
 * @brief 应用主题色
 */
void ScreenSoundSettings::applyTheme() {
    lv_obj_set_style_text_color(title_label, _primary_color, 0);
    for (int i=0; i<2; ++i) {
        lv_obj_set_style_text_color(item_labels[i], _primary_color, 0);
        lv_obj_set_style_text_color(value_labels[i], _primary_color, 0);
    }

    lv_obj_set_style_bg_color(focus_bar, _secondary_color, 0);
}

/**
 * @brief 创建界面并初始化控件
 */
void ScreenSoundSettings::create(lv_obj_t* parent) {
    screen = lv_obj_create(parent);
    lv_obj_remove_style_all(screen);
    lv_obj_set_size(screen, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    
    lv_obj_set_user_data(screen, this); 
    lv_obj_add_event_cb(screen, screen_loaded_cb, LV_EVENT_SCREEN_LOADED, NULL);

    focus_bar = lv_obj_create(screen);
    lv_obj_remove_style_all(focus_bar);
    lv_obj_set_height(focus_bar, 14); 
    lv_obj_set_style_bg_color(focus_bar, lv_color_hex(0xFFA500), 0);
    lv_obj_set_style_bg_opa(focus_bar, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(focus_bar, 3, 0);
    
    title_label = lv_label_create(screen);
    lv_label_set_text(title_label, "『声音设定』");
    lv_obj_set_style_text_font(title_label, &MyFont2, 0);
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(title_label, LV_ALIGN_TOP_LEFT, 5, 2);

    list_viewport = lv_obj_create(screen);
    lv_obj_remove_style_all(list_viewport);
    lv_obj_set_size(list_viewport, SCREEN_WIDTH, 48); 
    lv_obj_set_pos(list_viewport, 0, 22);

    items_container = lv_obj_create(list_viewport);
    lv_obj_remove_style_all(items_container);
    lv_obj_set_size(items_container, SCREEN_WIDTH, 100);
    lv_obj_set_pos(items_container, 0, 0); 

    const char* item_names[] = {"-音效", "-音量"};
    for(int i = 0; i < 2; i++) {
        item_labels[i] = lv_label_create(items_container);
        lv_label_set_text(item_labels[i], item_names[i]);
        lv_obj_set_style_text_font(item_labels[i], &MyFont2, 0);
        lv_obj_set_style_text_color(item_labels[i], lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_pos(item_labels[i], 5, i * 16);

        value_labels[i] = lv_label_create(items_container);
        lv_label_set_text(value_labels[i], ""); // 初始为空
        lv_obj_set_style_text_font(value_labels[i], &MyFont2, 0);
        lv_obj_set_style_text_color(value_labels[i], lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_width(value_labels[i], 30);
        lv_obj_align(value_labels[i], LV_ALIGN_TOP_RIGHT, 10, i * 16);
    }
    applyTheme();
}

void ScreenSoundSettings::bindData(AppSettings* settings) {
    current_settings = settings;
    _primary_color.full = current_settings->themeColorPrimary;
    _secondary_color.full = current_settings->themeColorSecondary;
    applyTheme();

    updateValues();
    selected_index = -1;
    is_editing = false;
}

void ScreenSoundSettings::updateValues() {
    if (!current_settings) return;
    lv_label_set_text(value_labels[0], current_settings->soundEnabled ? "启用" : "禁用");
    lv_label_set_text_fmt(value_labels[1], "%d", current_settings->volume);
}

void ScreenSoundSettings::action() {
    if (selected_index < 0) return; // 标题不可操作

    // 如果正在编辑音量，则退出编辑模式
    if (is_editing) {
        is_editing = false;
        moveFocusBar(false); // 更新焦点条回到"音量"标签
        return;
    }

    // 根据选中的项目执行不同操作
    switch(selected_index) {
        case 0: // 音效 开/关
            current_settings->soundEnabled = !current_settings->soundEnabled;
            updateValues();
            break;
        case 1: // 音量
            is_editing = true;
            moveFocusBar(false); // 更新焦点条到音量数值上
            break;
    }
}

void ScreenSoundSettings::scroll(int8_t direction) {
    // 如果正在编辑音量，则编码器用于修改数值
    if (is_editing && selected_index == 1) {
        int new_volume = current_settings->volume + direction;
        if (new_volume < 0) new_volume = 0;
        if (new_volume > 30) new_volume = 30;
        current_settings->volume = new_volume;
        updateValues();
    } 
    // 否则，编码器用于滚动菜单项
    else {
        int new_index = selected_index + direction;
        if (new_index < -1) new_index = 1; // 只有2项，循环范围是 -1 到 1
        if (new_index > 1) new_index = -1;
        selected_index = new_index;
        moveFocusBar(true);
    }
}

void ScreenSoundSettings::moveFocusBar(bool animate) {
    lv_obj_t* target_obj;
    // <<< 核心修改：判断焦点应该在标签上还是数值上 >>>
    if (is_editing && selected_index == 1) {
        target_obj = value_labels[1]; // 编辑模式下，焦点在数值上
    } else if (selected_index == -1) {
        target_obj = title_label;
    } else {
        target_obj = item_labels[selected_index];
    }
    
    // Y坐标计算不需要滚动，因为只有2项
    lv_coord_t target_y_base;
    if (selected_index == -1) {
        target_y_base = lv_obj_get_y_aligned(target_obj);
    } else {
        target_y_base = lv_obj_get_y(list_viewport) + lv_obj_get_y_aligned(target_obj);
    }
    
    lv_coord_t target_y = target_y_base - (14 - 12) / 2;
    lv_coord_t target_w = lv_obj_get_width(target_obj) + 4;
    // <<< 新增：X坐标也需要根据目标动态计算 >>>
    lv_coord_t target_x = lv_obj_get_x(target_obj);

    if (animate) {
        // ... (动画代码与功能设定菜单类似，但增加了X轴移动) ...
        lv_anim_t a_x;
        lv_anim_init(&a_x);
        lv_anim_set_var(&a_x, focus_bar);
        lv_anim_set_values(&a_x, lv_obj_get_x(focus_bar), target_x);
        lv_anim_set_time(&a_x, 250);
        lv_anim_set_path_cb(&a_x, lv_anim_path_ease_out);
        lv_anim_set_exec_cb(&a_x, (lv_anim_exec_xcb_t)lv_obj_set_x);
        lv_anim_start(&a_x);

        lv_anim_t a_y;
        lv_anim_init(&a_y);
        lv_anim_set_var(&a_y, focus_bar);
        lv_anim_set_values(&a_y, lv_obj_get_y(focus_bar), target_y); 
        lv_anim_set_time(&a_y, 250);
        lv_anim_set_path_cb(&a_y, lv_anim_path_ease_out);
        lv_anim_set_exec_cb(&a_y, (lv_anim_exec_xcb_t)lv_obj_set_y);
        lv_anim_start(&a_y);

        lv_anim_t a_w;
        lv_anim_init(&a_w);
        lv_anim_set_var(&a_w, focus_bar);
        lv_anim_set_values(&a_w, lv_obj_get_width(focus_bar), target_w);
        lv_anim_set_time(&a_w, 300);
        lv_anim_set_path_cb(&a_w, lv_anim_path_overshoot);
        lv_anim_set_exec_cb(&a_w, (lv_anim_exec_xcb_t)lv_obj_set_width);
        lv_anim_start(&a_w);
    } else {
        lv_obj_set_pos(focus_bar, target_x, target_y);
        lv_obj_set_width(focus_bar, target_w);
    }
}

// 静态回调函数 (完全复用，无需修改)
void ScreenSoundSettings::screen_loaded_cb(lv_event_t* e) {
    ScreenSoundSettings* self = (ScreenSoundSettings*)lv_obj_get_user_data(lv_event_get_target(e));
    if (self) {
        lv_timer_create(start_entry_anim_cb, 10, self);
    }
}

void ScreenSoundSettings::start_entry_anim_cb(lv_timer_t* timer) {
    ScreenSoundSettings* self = (ScreenSoundSettings*)timer->user_data;
    if (!self) {
        lv_timer_del(timer);
        return;
    }
    
    lv_obj_set_width(self->focus_bar, lv_obj_get_width(self->title_label) + 4);
    lv_obj_set_x(self->focus_bar, 5);
    lv_coord_t target_y = lv_obj_get_y_aligned(self->title_label) - (14 - 12) / 2;

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, self->focus_bar);
    lv_anim_set_values(&a, SCREEN_HEIGHT, target_y);
    lv_anim_set_time(&a, 400);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_start(&a);

    self->selected_index = -1;
    lv_timer_del(timer);
}