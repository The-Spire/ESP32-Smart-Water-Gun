/**
 * @file ScreenBulletSettings.cpp
 * @brief 子弹设置界面实现
 */

#include "ScreenBulletSettings.h"
#include "../../assets/assets.h"

ScreenBulletSettings::ScreenBulletSettings() {
    screen = nullptr;
    current_settings = nullptr;
    selected_index = -1;
    is_editing = false;
    _primary_color.full = 0xFFFF; // 默认白色
    _secondary_color.full = 0xFD00; // 默认橙色
}

lv_obj_t* ScreenBulletSettings::getScreenObj() {
    return screen;
}

/**
 * @brief 应用主题色到界面元素
 */
void ScreenBulletSettings::applyTheme() {
    lv_obj_set_style_text_color(title_label, _primary_color, 0);
    for (int i=0; i<4; ++i) {
        lv_obj_set_style_text_color(item_labels[i], _primary_color, 0);
        lv_obj_set_style_text_color(value_labels[i], _primary_color, 0);
    }

    lv_obj_set_style_bg_color(focus_bar, _secondary_color, 0);
}

/**
 * @brief 创建屏幕并初始化所有控件
 */
void ScreenBulletSettings::create(lv_obj_t* parent) {
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
    lv_label_set_text(title_label, "『子弹设定』");
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

    const char* item_names[] = {"-子弹类型", "-最大载弹量", "-射击速度", "-换弹速度", "-换弹延迟"};
    for(int i = 0; i < 5; i++) {
        item_labels[i] = lv_label_create(items_container);
        lv_label_set_text(item_labels[i], item_names[i]);
        lv_obj_set_style_text_font(item_labels[i], &MyFont2, 0);
        lv_obj_set_style_text_color(item_labels[i], lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_pos(item_labels[i], 5, i * 16);

        value_labels[i] = lv_label_create(items_container);
        lv_label_set_text(value_labels[i], "");
        lv_obj_set_style_text_font(value_labels[i], &MyFont2, 0);
        lv_obj_set_style_text_color(value_labels[i], lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_width(value_labels[i], 30);
        lv_obj_align(value_labels[i], LV_ALIGN_TOP_RIGHT, 10, i * 16);
    }
    applyTheme();
}

void ScreenBulletSettings::bindData(AppSettings* settings) {
    current_settings = settings;
    _primary_color.full = current_settings->themeColorPrimary;
    _secondary_color.full = current_settings->themeColorSecondary;
    applyTheme();
    updateValues();
    selected_index = -1;
    is_editing = false;
}

void ScreenBulletSettings::updateValues() {
    if (!current_settings) return;
    lv_label_set_text(value_labels[0], (current_settings->bulletType == BulletType::WATER) ? "水" : "激光");
    lv_label_set_text_fmt(value_labels[1], "%d", current_settings->maxAmmo);
    lv_label_set_text_fmt(value_labels[2], "%d", current_settings->fireRate);
    lv_label_set_text_fmt(value_labels[3], "%d", current_settings->reloadRate);
    lv_label_set_text_fmt(value_labels[4], "%d S", current_settings->autoReloadDelay);
}

void ScreenBulletSettings::action() {
    if (selected_index < 0) return;

    if (selected_index == 0) { // 子弹类型是切换项
        current_settings->bulletType = (current_settings->bulletType == BulletType::WATER) ? 
                                       BulletType::LASER : BulletType::WATER;
        updateValues();
    } else { // 其他是数值项
        is_editing = !is_editing;
        moveFocusBar(true);
    }
}

void ScreenBulletSettings::scroll(int8_t direction) {
    if (is_editing) {
        int* value_ptr = nullptr;
        int min_val = 1, max_val = 99;
        switch(selected_index) {
            case 1: value_ptr = &current_settings->maxAmmo; min_val = 15; break;
            case 2: value_ptr = &current_settings->fireRate; break;
            case 3: value_ptr = &current_settings->reloadRate; break;
            case 4: value_ptr = &current_settings->autoReloadDelay; break;
        }

        if (value_ptr) {
            int new_value = *value_ptr + direction;
            if (new_value < min_val) new_value = min_val;
            if (new_value > max_val) new_value = max_val;
            *value_ptr = new_value;
            updateValues();
        }
    } 
    else {
        int new_index = selected_index + direction;
        if (new_index < -1) new_index = 4;
        if (new_index > 4) new_index = -1;
        selected_index = new_index;
        moveFocusBar(true);
    }
}

void ScreenBulletSettings::moveFocusBar(bool animate) {
       // 这部分代码现在与 ScreenFuncSettings 中的逻辑完全相同
    lv_obj_t* target_obj;
    
    // <<< 新增：计算列表容器的目标Y坐标 >>>
    lv_coord_t container_target_y = 0; // 默认位置，不滚动
    if (selected_index > 2) { // 如果选中第4项 (index=3)
       container_target_y = -(selected_index - 2) * 16;
    }

    if (is_editing) {
        target_obj = value_labels[selected_index];
    } else if (selected_index == -1) {
        target_obj = title_label;
    } else {
        target_obj = item_labels[selected_index];
    }
    
    lv_coord_t target_y_base;
    if (selected_index == -1) {
        target_y_base = lv_obj_get_y_aligned(target_obj);
    } else {
        // <<< 修改：在计算Y坐标时，使用 container_target_y >>>
        target_y_base = lv_obj_get_y(list_viewport) + container_target_y + lv_obj_get_y_aligned(target_obj);
    }
    
    lv_coord_t target_y = target_y_base - (14 - 12) / 2;
    lv_coord_t target_w = lv_obj_get_width(target_obj) + 4;
    lv_coord_t target_x = lv_obj_get_x(target_obj);

    if (animate) {
         // <<< 新增：启动列表容器的滚动动画 >>>
        lv_anim_t a_scroll;
        lv_anim_init(&a_scroll);
        lv_anim_set_var(&a_scroll, items_container);
        lv_anim_set_values(&a_scroll, lv_obj_get_y(items_container), container_target_y);
        lv_anim_set_time(&a_scroll, 250);
        lv_anim_set_path_cb(&a_scroll, lv_anim_path_ease_out);
        lv_anim_set_exec_cb(&a_scroll, (lv_anim_exec_xcb_t)lv_obj_set_y);
        lv_anim_start(&a_scroll);
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
        lv_obj_set_y(items_container, container_target_y);
        lv_obj_set_pos(focus_bar, target_x, target_y);
        lv_obj_set_width(focus_bar, target_w);
    }
}

// 静态回调函数 (完全复用，无需修改)
void ScreenBulletSettings::screen_loaded_cb(lv_event_t* e) {
    ScreenBulletSettings* self = (ScreenBulletSettings*)lv_obj_get_user_data(lv_event_get_target(e));
    if (self) {
        lv_timer_create(start_entry_anim_cb, 10, self);
    }
}

void ScreenBulletSettings::start_entry_anim_cb(lv_timer_t* timer) {
    ScreenBulletSettings* self = (ScreenBulletSettings*)timer->user_data;
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