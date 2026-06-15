/**
 * @file ScreenFuncSettings.cpp
 * @brief 功能设置界面实现
 */

#include "ScreenFuncSettings.h"
#include "../../logic/WaterGunLogic.h"
#include "../../assets/assets.h"

ScreenFuncSettings::ScreenFuncSettings() {
    screen = nullptr;
    current_settings = nullptr;
    selected_index = -1;
    _primary_color.full = 0xFFFF;
    _secondary_color.full = 0xFD00;
}

lv_obj_t* ScreenFuncSettings::getScreenObj() {
    return screen;
}

/**
 * @brief 应用主题色到界面元素
 */
void ScreenFuncSettings::applyTheme() {
    lv_obj_set_style_text_color(title_label, _primary_color, 0);
    for (int i=0; i<4; ++i) {
        lv_obj_set_style_text_color(item_labels[i], _primary_color, 0);
        lv_obj_set_style_text_color(value_labels[i], _primary_color, 0);
    }

    lv_obj_set_style_bg_color(focus_bar, _secondary_color, 0);
}

/**
 * @brief 创建屏幕并初始化控件
 */
void ScreenFuncSettings::create(lv_obj_t* parent) {
    screen = lv_obj_create(parent);
    lv_obj_remove_style_all(screen);
    lv_obj_set_size(screen, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    
    // 将自身实例的指针存入对象，以便静态回调可以找到它
    lv_obj_set_user_data(screen, this); 
    lv_obj_add_event_cb(screen, screen_loaded_cb, LV_EVENT_SCREEN_LOADED, NULL);

    // --- 以下代码大部分是从 UIManager._create_func_settings_screen() 迁移过来的 ---

    focus_bar = lv_obj_create(screen);
    lv_obj_remove_style_all(focus_bar);
    lv_obj_set_height(focus_bar, 14); 
    lv_obj_set_style_bg_color(focus_bar, lv_color_hex(0xFFA500), 0);
    lv_obj_set_style_bg_opa(focus_bar, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(focus_bar, 3, 0);
    
    title_label = lv_label_create(screen);
    lv_label_set_text(title_label, "『功能设定』");
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

    const char* item_names[] = {"-水泵", "-加湿器", "-灯光", "-激光器" ,"-自动待机"};
    for(int i = 0; i < 5; i++) {
        item_labels[i] = lv_label_create(items_container);
        lv_label_set_text(item_labels[i], item_names[i]);
        lv_obj_set_style_text_font(item_labels[i], &MyFont2, 0);
        lv_obj_set_style_text_color(item_labels[i], lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_pos(item_labels[i], 5, i * 16);

        value_labels[i] = lv_label_create(items_container);
        lv_label_set_text(value_labels[i], "启用");
        lv_obj_set_style_text_font(value_labels[i], &MyFont2, 0);
        lv_obj_set_style_text_color(value_labels[i], lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_width(value_labels[i], 30);
        lv_obj_align(value_labels[i], LV_ALIGN_TOP_RIGHT, 10, i * 16);
    }
    applyTheme();
}

void ScreenFuncSettings::bindData(AppSettings* settings) {
    current_settings = settings;
    _primary_color.full = current_settings->themeColorPrimary;
    _secondary_color.full = current_settings->themeColorSecondary;

    applyTheme();

    updateValues();
    selected_index = -1; // 默认选中标题
    is_editing = false;
}

void ScreenFuncSettings::updateValues() {
    if (!current_settings) return;
    lv_label_set_text(value_labels[0], current_settings->pumpEnabled ? "启用" : "禁用");
    lv_label_set_text(value_labels[1], current_settings->humidifierEnabled ? "启用" : "禁用");
    lv_label_set_text(value_labels[2], current_settings->lightsEnabled ? "启用" : "禁用");
    lv_label_set_text(value_labels[3], current_settings->laserEnabled ? "启用" : "禁用");
    if (current_settings->autoStandbyMinutes < 1) {
        lv_label_set_text(value_labels[4], "禁用");
    } else {
        lv_label_set_text_fmt(value_labels[4], "%d M", current_settings->autoStandbyMinutes);
    }
}


void ScreenFuncSettings::action() {
    if (selected_index < 0) return; // 标题不可操作

    // 判断当前选中的是哪种类型的设置项
    if (selected_index >= 0 && selected_index <= 3) { // 前4项是开关
        // 对开关项，短按直接翻转状态，不进入编辑模式
        bool* flag_ptr = nullptr;
        switch(selected_index) {
            case 0: flag_ptr = &current_settings->pumpEnabled; break;
            case 1: flag_ptr = &current_settings->humidifierEnabled; break;
            case 2: flag_ptr = &current_settings->lightsEnabled; break;
            case 3: flag_ptr = &current_settings->laserEnabled; break;
        }
        if (flag_ptr) {
            *flag_ptr = !(*flag_ptr);
            updateValues();
            if (g_logicManager) {
                g_logicManager->updateLiveSettings(current_settings);
            }
        }
    } 
    else if (selected_index == 4) { // 第5项是数值调节
        // 对数值项，短按用于进入/退出编辑模式
        is_editing = !is_editing;
        moveFocusBar(true); // 调用 moveFocusBar 来更新焦点条的形态
    }
}
void ScreenFuncSettings::scroll(int8_t direction) {
    if (is_editing) {
        // <<< 新增：处理自动待机时间的编辑 >>>
        if (selected_index == 4) {
            int new_time = current_settings->autoStandbyMinutes + direction;
            if (new_time < 0) new_time = 0;     // 0 代表禁用
            if (new_time > 99) new_time = 99;
            current_settings->autoStandbyMinutes = new_time;
        }
        updateValues();
        if (g_logicManager) g_logicManager->updateLiveSettings(current_settings);
    } 
    else {
        int new_index = selected_index + direction;
        // <<< 修改：更新索引范围 >>>
        if (new_index < -1) new_index = 4;
        if (new_index > 4) new_index = -1;
        selected_index = new_index;
        moveFocusBar(true);
    }
}

void ScreenFuncSettings::moveFocusBar(bool animate) {
    lv_obj_t* target_obj;
    lv_coord_t target_x, target_y, target_w, target_h = 14, target_radius = 3;

    // --- 1. 列表滚动动画的目标Y值计算 ---
    lv_coord_t container_target_y = 0; // 默认不滚动
    // 视口可以显示3行 (索引 0, 1, 2)
    if (selected_index > 2) {
        // 如果选中的项超出了可视范围的底部
        // 向上滚动 (selected_index - 2) 行的距离
        container_target_y = -(selected_index - 2) * 16;
    }

    // --- 2. 确定焦点条的目标对象和形态 ---
    if (is_editing && selected_index == 4) { // 编辑“自动待机”
        target_obj = value_labels[4];
    } else if (selected_index == -1) { // 标题
        target_obj = title_label;
    } else { // 列表项
        target_obj = item_labels[selected_index];
    }

    // --- 3. 焦点条动画的目标参数计算 ---
    // Y坐标的计算必须使用容器的“目标Y”来确保同步
    lv_coord_t target_y_base;
    if (selected_index == -1) {
        target_y_base = lv_obj_get_y(title_label);
    } else {
        // 全局Y = 视口Y + 容器目标Y + 项目在容器内的Y
        target_y_base = lv_obj_get_y(list_viewport) + container_target_y + lv_obj_get_y(item_labels[selected_index]);
    }
    
    // 如果在编辑模式，焦点应该对齐到 value_label
    if (is_editing && selected_index == 4) {
        target_y_base = lv_obj_get_y(list_viewport) + container_target_y + lv_obj_get_y(value_labels[selected_index]);
    }

    target_y = target_y_base - (14 - 12) / 2;
    
    // X 和 W 坐标计算
    target_w = lv_obj_get_width(target_obj) + 4;
    target_x = lv_obj_get_x(target_obj) - 2;

    // --- 4. 执行动画 ---
    if (animate) {
        // a. 启动列表容器的滚动动画
        lv_anim_t a_scroll;
        lv_anim_init(&a_scroll);
        lv_anim_set_var(&a_scroll, items_container);
        lv_anim_set_values(&a_scroll, lv_obj_get_y(items_container), container_target_y);
        lv_anim_set_time(&a_scroll, 250);
        lv_anim_set_path_cb(&a_scroll, lv_anim_path_ease_out);
        lv_anim_set_exec_cb(&a_scroll, (lv_anim_exec_xcb_t)lv_obj_set_y);
        lv_anim_start(&a_scroll);

        // b. 启动焦点条的移动和变形动画 (代码与其他菜单模块相同)
        lv_anim_t a_h, a_r, a_x, a_y, a_w;
        lv_anim_init(&a_h); lv_anim_init(&a_r); lv_anim_init(&a_x); lv_anim_init(&a_y); lv_anim_init(&a_w);
        
        lv_anim_set_var(&a_h, focus_bar); lv_anim_set_time(&a_h, 250); lv_anim_set_exec_cb(&a_h, (lv_anim_exec_xcb_t)lv_obj_set_height);
        lv_anim_set_var(&a_r, focus_bar); lv_anim_set_time(&a_r, 250); lv_anim_set_exec_cb(&a_r, (lv_anim_exec_xcb_t)lv_obj_set_style_radius);
        lv_anim_set_var(&a_x, focus_bar); lv_anim_set_time(&a_x, 250); lv_anim_set_path_cb(&a_x, lv_anim_path_ease_out); lv_anim_set_exec_cb(&a_x, (lv_anim_exec_xcb_t)lv_obj_set_x);
        lv_anim_set_var(&a_y, focus_bar); lv_anim_set_time(&a_y, 250); lv_anim_set_path_cb(&a_y, lv_anim_path_ease_out); lv_anim_set_exec_cb(&a_y, (lv_anim_exec_xcb_t)lv_obj_set_y);
        lv_anim_set_var(&a_w, focus_bar); lv_anim_set_time(&a_w, 300); lv_anim_set_path_cb(&a_w, lv_anim_path_overshoot); lv_anim_set_exec_cb(&a_w, (lv_anim_exec_xcb_t)lv_obj_set_width);

        lv_anim_set_values(&a_h, lv_obj_get_height(focus_bar), target_h);
        lv_anim_set_values(&a_r, lv_obj_get_style_radius(focus_bar, 0), target_radius);
        lv_anim_set_values(&a_x, lv_obj_get_x(focus_bar), target_x);
        lv_anim_set_values(&a_y, lv_obj_get_y(focus_bar), target_y);
        lv_anim_set_values(&a_w, lv_obj_get_width(focus_bar), target_w);

        lv_anim_start(&a_h); lv_anim_start(&a_r); lv_anim_start(&a_x); lv_anim_start(&a_y); lv_anim_start(&a_w);
    } else { // 非动画模式
        lv_obj_set_y(items_container, container_target_y);
        lv_obj_set_pos(focus_bar, target_x, target_y);
        lv_obj_set_size(focus_bar, target_w, target_h);
        lv_obj_set_style_radius(focus_bar, target_radius, 0);
    }
}

// 静态回调函数
void ScreenFuncSettings::screen_loaded_cb(lv_event_t* e) {
    ScreenFuncSettings* self = (ScreenFuncSettings*)lv_obj_get_user_data(lv_event_get_target(e));
    if (self) {
        lv_timer_create(start_entry_anim_cb, 10, self);
    }
}

void ScreenFuncSettings::start_entry_anim_cb(lv_timer_t* timer) {
    ScreenFuncSettings* self = (ScreenFuncSettings*)timer->user_data;
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