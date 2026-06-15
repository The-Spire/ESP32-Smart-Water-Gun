#include "ScreenThemeSettings.h"
#include "../../logic/WaterGunLogic.h"
#include "../../assets/assets.h" // 包含所有资源声明

const lv_color_t ScreenThemeSettings::PRESET_COLORS[ScreenThemeSettings::COLOR_COUNT] = {
    lv_color_hex(0xFF0000), lv_color_hex(0xFFA500), lv_color_hex(0xFFFF00),
    lv_color_hex(0x00FF00), lv_color_hex(0x00FFFF), lv_color_hex(0x0000FF),
    lv_color_hex(0xFF00FF),lv_color_hex(0xFFFFFF)  // White (新增)
};

ScreenThemeSettings::ScreenThemeSettings() {     
    screen = nullptr;
    current_settings = nullptr;
    selected_index = -1;
    is_editing = false; 
    _primary_color.full = 0xFFFF; // 默认白色
    _secondary_color.full = 0xFD00; // 默认橙色
}

lv_obj_t* ScreenThemeSettings::getScreenObj() { return screen; }
void ScreenThemeSettings::applyTheme() {
    // 应用常规色
    lv_obj_set_style_text_color(title_label, _primary_color, 0);
    for (int i=0; i<2; ++i) {
        lv_obj_set_style_text_color(item_labels[i], _primary_color, 0);
        lv_obj_set_style_text_color(color_preview_boxes[i], _primary_color, 0); // 预览框的文字（如果有）
    }

    // 应用强调色
    lv_obj_set_style_bg_color(focus_bar, _secondary_color, 0);
}
void ScreenThemeSettings::create(lv_obj_t* parent) {
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
    lv_label_set_text(title_label, "『主题设定』");
    lv_obj_set_style_text_font(title_label, &MyFont2, 0);
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_pos(title_label, 5, 2); // 明确位置

    // --- 创建列表项 ---
    const char* item_names[] = {"-常规色", "-强调色"};
    for (int i = 0; i < 2; ++i) {
        item_labels[i] = lv_label_create(screen);
        lv_label_set_text(item_labels[i], item_names[i]);
        lv_obj_set_style_text_font(item_labels[i], &MyFont2, 0);
        lv_obj_set_style_text_color(item_labels[i], lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_pos(item_labels[i], 5, 22 + i * 16);

        color_preview_boxes[i] = lv_obj_create(screen);
        lv_obj_remove_style_all(color_preview_boxes[i]);
        lv_obj_set_size(color_preview_boxes[i], 20, 10);
        lv_obj_set_style_radius(color_preview_boxes[i], 3, 0);
        lv_obj_set_style_bg_opa(color_preview_boxes[i], LV_OPA_COVER, 0);
        lv_obj_align(color_preview_boxes[i], LV_ALIGN_TOP_RIGHT, -5, 22 + i * 16);
    }
    
    static lv_color_t cbuf[LV_CANVAS_BUF_SIZE_TRUE_COLOR(86, 12)];

    // b. 创建Canvas对象
    color_picker_bar = lv_canvas_create(screen);
    lv_canvas_set_buffer(color_picker_bar, cbuf, 86, 12, LV_IMG_CF_TRUE_COLOR);
    lv_obj_add_flag(color_picker_bar, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(color_picker_bar, 5, 54);

    // c. 绘制彩虹
    // 我们将整个宽度分成 (COLOR_COUNT - 1) 段
    int segments = COLOR_COUNT - 1;
    for (int i = 0; i < segments; ++i) {
        lv_color_t c1 = PRESET_COLORS[i];
        lv_color_t c2 = PRESET_COLORS[i + 1];

        // 计算这个颜色段在Canvas上的起始和结束X坐标
        int start_x = (i * 86) / segments;
        int end_x = ((i + 1) * 86) / segments;

        // 在这个颜色段内，逐列绘制垂直的渐变色
        for (int x = start_x; x < end_x; ++x) {
            // 计算当前x在当前小段内的比例
            float segment_ratio = (float)(x - start_x) / (float)(end_x - start_x);
            lv_color_t col = lv_color_mix(c2, c1, (uint8_t)(segment_ratio * 255));
            
            // 用这个颜色画一条垂直线
            for (int y = 0; y < 12; ++y) {
                lv_canvas_set_px_color(color_picker_bar, x, y, col);
            }
        }
    }
    applyTheme();
}

void ScreenThemeSettings::bindData(AppSettings* settings) {
    current_settings = settings;
    _primary_color.full = current_settings->themeColorPrimary;
    _secondary_color.full = current_settings->themeColorSecondary;
    applyTheme();
    updateValues();
    selected_index = -1;
    is_editing = false;
    _updateSliderPosFromColor();
}

void ScreenThemeSettings::updateValues() {
    if (!current_settings) return;
    lv_color_t primary, secondary;
    primary.full = current_settings->themeColorPrimary;
    secondary.full = current_settings->themeColorSecondary;
    lv_obj_set_style_bg_color(color_preview_boxes[0], primary, 0);
    lv_obj_set_style_bg_color(color_preview_boxes[1], secondary, 0);
}

void ScreenThemeSettings::action() {
    if (selected_index < 0) return;
    is_editing = !is_editing;
    
    if (is_editing) {
        _updateSliderPosFromColor();
    }
    
    moveFocusBar(true);
}

void ScreenThemeSettings::scroll(int8_t direction) {
    if (is_editing) {
        // 移动滑块 (焦点条)
        lv_coord_t current_x = lv_obj_get_x(focus_bar);
        lv_coord_t min_x = 5;
        lv_coord_t max_x = 5 + 86 - 4; // 滑块宽度4
        lv_coord_t new_x = current_x + direction * 2;
        if (new_x < min_x) new_x = min_x;
        if (new_x > max_x) new_x = max_x;
        lv_obj_set_x(focus_bar, new_x);
        
        // 根据滑块位置更新颜色
        _updateColorFromSliderPos();
                // <<< 新增：实时预览主题颜色 >>>
        _primary_color.full = current_settings->themeColorPrimary;
        _secondary_color.full = current_settings->themeColorSecondary;
        applyTheme();
        updateValues();
        if (g_logicManager) {
         g_logicManager->updateLiveSettings(current_settings);
        }
    } 
    else {
        int new_index = selected_index + direction;
        if (new_index < -1) new_index = 1;
        if (new_index > 1) new_index = -1;
        selected_index = new_index;
        moveFocusBar(true);
    }
}

// <<< 核心修改：更新哪个颜色取决于 selected_index >>>
void ScreenThemeSettings::_updateColorFromSliderPos() {
     lv_coord_t current_x = lv_obj_get_x(focus_bar);
    lv_coord_t width = 4; // 滑块宽度
    lv_coord_t min_x = 5;
    lv_coord_t max_x = 5 + 86 - width;
    
    float ratio = (float)(current_x - min_x) / (float)(max_x - min_x);
    if (ratio < 0.0f) ratio = 0.0f;
    if (ratio > 1.0f) ratio = 1.0f;

    int segments = COLOR_COUNT - 1;
    float total_pos = ratio * segments;
    int segment_index = (int)total_pos;
    if (segment_index >= segments) segment_index = segments - 1;
    
    float segment_ratio = total_pos - segment_index;

    lv_color_t c1 = PRESET_COLORS[segment_index];
    lv_color_t c2 = PRESET_COLORS[segment_index + 1];
    
    uint8_t mix_ratio_8bit = (uint8_t)(segment_ratio * 255.0f);
    
    lv_color_t final_color = lv_color_mix(c2, c1, mix_ratio_8bit);
    
    
    // 唯一的区别是最后赋值给哪个变量
    if (selected_index == 0) { // 正在编辑常规色
        current_settings->themeColorPrimary = final_color.full;
    } else if (selected_index == 1) { // 正在编辑强调色
        current_settings->themeColorSecondary = final_color.full;
    }
}

// <<< 核心修改：从哪个颜色初始化滑块，也取决于 selected_index >>>
lv_coord_t ScreenThemeSettings::_updateSliderPosFromColor() {
    uint16_t color_to_find = (selected_index == 0) ? 
        current_settings->themeColorPrimary : 
        current_settings->themeColorSecondary;

    lv_color_t saved_color;
    saved_color.full = color_to_find;
    // 将保存的颜色转换为32位(RGB888)，以便进行精确比较
    lv_color32_t saved_color32;
    saved_color32.full = lv_color_to32(saved_color);

    float best_global_ratio = 0.0f;
    int smallest_diff = 100000;

    for (int i = 0; i < COLOR_COUNT - 1; ++i) {
        lv_color_t c1 = PRESET_COLORS[i];
        lv_color_t c2 = PRESET_COLORS[i + 1];
        
        // 同样，将预设颜色也转换为32位
        lv_color32_t c1_32 = {.full = lv_color_to32(c1)};
        lv_color32_t c2_32 = {.full = lv_color_to32(c2)};

        // 在每个线段上进行10次采样来查找最接近的颜色
        for (int j = 0; j <= 10; ++j) {
            float ratio_in_segment = (float)j / 10.0f;
            
            // 在32位空间进行混合
            lv_color32_t mixed_color32;
            mixed_color32.ch.red = c1_32.ch.red + (c2_32.ch.red - c1_32.ch.red) * ratio_in_segment;
            mixed_color32.ch.green = c1_32.ch.green + (c2_32.ch.green - c1_32.ch.green) * ratio_in_segment;
            mixed_color32.ch.blue = c1_32.ch.blue + (c2_32.ch.blue - c1_32.ch.blue) * ratio_in_segment;

            // 在32位空间计算颜色差异
            int diff = abs(mixed_color32.ch.red - saved_color32.ch.red) * 3 +
                       abs(mixed_color32.ch.green - saved_color32.ch.green) * 6 +
                       abs(mixed_color32.ch.blue - saved_color32.ch.blue) * 1;

            if (diff < smallest_diff) {
                smallest_diff = diff;
                best_global_ratio = ((float)i + ratio_in_segment) / (float)(COLOR_COUNT - 1);
            }
        }
    }

    lv_coord_t width = 4; // 滑块宽度
    lv_coord_t min_x = 5;
    lv_coord_t max_x = 5 + 86 - width;
    
    // <<< 关键：返回计算出的X坐标，而不是设置它 >>>
    return min_x + best_global_ratio * (max_x - min_x);
}

void ScreenThemeSettings::moveFocusBar(bool animate) {
    lv_coord_t target_x, target_y, target_w, target_h = 14, target_radius = 3;
    
    bool is_entering_color_edit = is_editing && (selected_index == 0 || selected_index == 1);
    
    // --- 确定目标形态和位置 ---
    if (is_entering_color_edit) {
        // 变形为滑块
        target_w = 4;
        target_h = 16;
        target_radius = 2;
        target_y = lv_obj_get_y(color_picker_bar) - 2;
        target_x = _updateSliderPosFromColor(); // 调用函数获取正确的X坐标
    } else {
        // 恢复为正常形态
        lv_obj_t* target_obj = (selected_index == -1) ? title_label : item_labels[selected_index];
        target_w = lv_obj_get_width(target_obj) + 4;
        target_x = lv_obj_get_x(target_obj) - 2;
        target_y = lv_obj_get_y(target_obj) - 1;
        target_h = 14;
        target_radius = 3;
    }

    // --- 执行动画 ---
    if (animate) {
        // 拾取条的淡入/淡出控制
        if (is_entering_color_edit) {
            lv_obj_clear_flag(color_picker_bar, LV_OBJ_FLAG_HIDDEN);
        }

        // 焦点条变形与移动动画
        lv_anim_t a_h, a_r, a_x, a_y, a_w;
        lv_anim_init(&a_h); lv_anim_init(&a_r); lv_anim_init(&a_x); lv_anim_init(&a_y); lv_anim_init(&a_w);
        
        // 设置动画公共参数
        lv_anim_set_var(&a_h, focus_bar); lv_anim_set_time(&a_h, 250); lv_anim_set_exec_cb(&a_h, (lv_anim_exec_xcb_t)lv_obj_set_height);
        lv_anim_set_var(&a_r, focus_bar); lv_anim_set_time(&a_r, 250); lv_anim_set_exec_cb(&a_r, (lv_anim_exec_xcb_t)lv_obj_set_style_radius);
        lv_anim_set_var(&a_x, focus_bar); lv_anim_set_time(&a_x, 250); lv_anim_set_path_cb(&a_x, lv_anim_path_ease_out); lv_anim_set_exec_cb(&a_x, (lv_anim_exec_xcb_t)lv_obj_set_x);
        lv_anim_set_var(&a_y, focus_bar); lv_anim_set_time(&a_y, 250); lv_anim_set_path_cb(&a_y, lv_anim_path_ease_out); lv_anim_set_exec_cb(&a_y, (lv_anim_exec_xcb_t)lv_obj_set_y);
        lv_anim_set_var(&a_w, focus_bar); lv_anim_set_time(&a_w, 300); lv_anim_set_path_cb(&a_w, lv_anim_path_overshoot); lv_anim_set_exec_cb(&a_w, (lv_anim_exec_xcb_t)lv_obj_set_width);

        // 设置动画目标值
        lv_anim_set_values(&a_h, lv_obj_get_height(focus_bar), target_h);
        lv_anim_set_values(&a_r, lv_obj_get_style_radius(focus_bar, 0), target_radius);
        lv_anim_set_values(&a_x, lv_obj_get_x(focus_bar), target_x);
        lv_anim_set_values(&a_y, lv_obj_get_y(focus_bar), target_y);
        lv_anim_set_values(&a_w, lv_obj_get_width(focus_bar), target_w);

        // 设置完成回调，用于在退出编辑时隐藏拾取条
        lv_anim_set_ready_cb(&a_y, [](lv_anim_t* a) {
            lv_obj_t* screen_obj = lv_obj_get_screen((lv_obj_t*)a->var);
            ScreenThemeSettings* self = (ScreenThemeSettings*)lv_obj_get_user_data(screen_obj);
            if (!self) return;

            if (!self->is_editing) {
                lv_obj_add_flag(self->color_picker_bar, LV_OBJ_FLAG_HIDDEN);
            }
        });
        
        // 启动所有动画
        lv_anim_start(&a_h); lv_anim_start(&a_r); lv_anim_start(&a_x); lv_anim_start(&a_y); lv_anim_start(&a_w);
    } else { // 非动画模式
        lv_obj_set_pos(focus_bar, target_x, target_y);
        lv_obj_set_size(focus_bar, target_w, target_h);
        lv_obj_set_style_radius(focus_bar, target_radius, 0);

        if (is_entering_color_edit) {
            lv_obj_clear_flag(color_picker_bar, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(color_picker_bar, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

void ScreenThemeSettings::screen_loaded_cb(lv_event_t* e) {
    ScreenThemeSettings* self = (ScreenThemeSettings*)lv_obj_get_user_data(lv_event_get_target(e));
    if (self) {
        lv_timer_create(start_entry_anim_cb, 10, self);
    }
}

void ScreenThemeSettings::start_entry_anim_cb(lv_timer_t* timer) {
    ScreenThemeSettings* self = (ScreenThemeSettings*)timer->user_data;
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