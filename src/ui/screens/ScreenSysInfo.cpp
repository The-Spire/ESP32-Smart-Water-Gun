#include "ScreenSysInfo.h"
#include "../../logic/WaterGunLogic.h"
#include "../../assets/assets.h" // 包含所有资源声明


ScreenSysInfo::ScreenSysInfo() {    
    screen = nullptr; // 初始化为空
    current_settings = nullptr;
    selected_index = -1;
    _primary_color.full = 0xFFFF; // 默认白色
    _secondary_color.full = 0xFD00; // 默认橙色
}

lv_obj_t* ScreenSysInfo::getScreenObj() { return screen; }

void ScreenSysInfo::create(lv_obj_t* parent) {
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
    lv_label_set_text(title_label, "『系统信息』");
    lv_obj_set_style_text_font(title_label, &MyFont2, 0);
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(title_label, LV_ALIGN_TOP_LEFT, 5, 2);

    list_viewport = lv_obj_create(screen);
    lv_obj_remove_style_all(list_viewport);
    lv_obj_set_size(list_viewport, SCREEN_WIDTH, 48); 
    lv_obj_set_pos(list_viewport, 0, 21);

    items_container = lv_obj_create(list_viewport);
    lv_obj_remove_style_all(items_container);
    // 高度足够容纳所有项，包括开发者信息的两行
    lv_obj_set_size(items_container, SCREEN_WIDTH, 16 * 6); 
    lv_obj_set_pos(items_container, 0, 0); 

    // --- 创建列表项 ---
    const char* item_names[] = {"-显示帧率", "-显示RAM", "-显示CPU", "-开发者信息 :"};
    
    // 前3项
    for (int i = 0; i < 3; ++i) {
        item_labels[i] = lv_label_create(items_container);
        lv_label_set_text(item_labels[i], item_names[i]);
        lv_obj_set_style_text_font(item_labels[i], &MyFont2, 0);
        lv_obj_set_pos(item_labels[i], 5, i * 16);

        value_labels[i] = lv_label_create(items_container);
        lv_obj_set_width(value_labels[i], 30);
        lv_label_set_text(value_labels[i], "启用");
        lv_obj_set_style_text_font(value_labels[i], &MyFont2, 0);
        lv_obj_align(value_labels[i], LV_ALIGN_TOP_RIGHT, 5, i * 16);
    }
    
    // 第4项: 开发者信息 (这是一个复合项)
    item_labels[3] = lv_label_create(items_container);
    lv_label_set_text(item_labels[3], item_names[3]);
    lv_obj_set_style_text_font(item_labels[3], &MyFont2, 0);
    lv_obj_set_pos(item_labels[3], 5, 3 * 16);

    // logo 和 名字 放在一个容器里，方便对齐
    lv_obj_t* dev_info_container = lv_obj_create(items_container);
    lv_obj_remove_style_all(dev_info_container);
    lv_obj_set_size(dev_info_container, SCREEN_WIDTH, 20);
    lv_obj_set_pos(dev_info_container, 5, 4 * 16+2);

    dev_logo_img = lv_img_create(dev_info_container);
    lv_img_set_src(dev_logo_img, &logo20x20);
    lv_obj_align(dev_logo_img, LV_ALIGN_LEFT_MID, 0, 0);

    dev_name_label = lv_label_create(dev_info_container);
    lv_label_set_text(dev_name_label, "螺旋之域");
    lv_obj_set_style_text_font(dev_name_label, &MyFont2, 0);
    lv_obj_align_to(dev_name_label, dev_logo_img, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

    applyTheme(); // 应用初始主题
}

void ScreenSysInfo::applyTheme() {
    lv_obj_set_style_text_color(title_label, _primary_color, 0);
    for (int i=0; i<4; ++i) {
    lv_obj_set_style_text_color(item_labels[i], _primary_color, 0);
        if(i<3){lv_obj_set_style_text_color(value_labels[i], _primary_color, 0);}
    }
    lv_obj_set_style_text_color(dev_name_label, _primary_color, 0);
    // 应用强调色
    lv_obj_set_style_bg_color(focus_bar, _secondary_color, 0);
}

void ScreenSysInfo::bindData(AppSettings* settings) {
    current_settings = settings;
    _primary_color.full = current_settings->themeColorPrimary;
    _secondary_color.full = current_settings->themeColorSecondary;
    applyTheme();

    updateValues();
    selected_index = -1; // 默认选中标题
}

void ScreenSysInfo::updateValues() {
    if (!current_settings) return;
    lv_label_set_text(value_labels[0], current_settings->showFps ? "启用" : "禁用");
    lv_label_set_text(value_labels[1], current_settings->showRam ? "启用" : "禁用");
    lv_label_set_text(value_labels[2], current_settings->showCpu ? "启用" : "禁用");
    // TODO: 实时更新数据
}

void ScreenSysInfo::action() {
    if (selected_index < 0 || selected_index > 2) return; // 开发者信息项不可操作
    
    bool* flag_ptr = nullptr;
    switch(selected_index) {
        case 0: flag_ptr = &current_settings->showFps; break;
        case 1: flag_ptr = &current_settings->showRam; break;
        case 2: flag_ptr = &current_settings->showCpu; break;
    }
    if (flag_ptr) {
        *flag_ptr = !(*flag_ptr);
        updateValues();
        if (g_logicManager) {
             g_logicManager->updateLiveSettings(current_settings);
        }
    }
}

void ScreenSysInfo::scroll(int8_t direction) {
    int new_index = selected_index + direction;
    if (new_index < -1) new_index = 3;
    if (new_index > 3) new_index = -1;
    selected_index = new_index;
    moveFocusBar(true);
}

// <<< 核心修改：moveFocusBar 实现特殊滚动 >>>
void ScreenSysInfo::moveFocusBar(bool animate) {
    lv_obj_t* target_obj = (selected_index == -1) ? title_label : item_labels[selected_index];
    
    // --- 1. 列表滚动动画的目标Y值计算 ---
    lv_coord_t container_target_y = 0; // 默认位置，不滚动
    if (selected_index == 3) {
        // 当选中第4项(开发者信息)时，列表向上滚动3行的高度
        container_target_y = -16 * 3;
    }
    
    // --- 2. 焦点条动画的目标参数计算 ---
    lv_coord_t target_x, target_y, target_w, target_h = 14, target_radius = 3;

    // Y坐标的计算必须使用容器的“目标Y”来确保同步
    lv_coord_t target_y_base;
    if (selected_index == -1) {
        target_y_base = lv_obj_get_y(title_label);
    } else {
        target_y_base = lv_obj_get_y(list_viewport) + container_target_y + lv_obj_get_y(item_labels[selected_index]);
    }
    
    target_y = target_y_base - (14 - 12) / 2; // (条高 - 字高) / 2
    
    // X 和 W 坐标计算
    target_w = lv_obj_get_width(target_obj) + 4;
    target_x = lv_obj_get_x(target_obj) - 2;

    // --- 3. 执行动画 ---
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

        // b. 启动焦点条的移动和变形动画
        // (这部分代码与其他菜单模块完全相同)
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

void ScreenSysInfo::screen_loaded_cb(lv_event_t* e) {
    ScreenSysInfo* self = (ScreenSysInfo*)lv_obj_get_user_data(lv_event_get_target(e));
    if (self) {
        lv_timer_create(start_entry_anim_cb, 10, self);
    }
}

void ScreenSysInfo::start_entry_anim_cb(lv_timer_t* timer) {
    ScreenSysInfo* self = (ScreenSysInfo*)timer->user_data;
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