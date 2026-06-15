#ifndef ASSETS_H
#define ASSETS_H

#include <lvgl.h>

// --- 字体声明 ---

LV_FONT_DECLARE(MyFont1);
LV_FONT_DECLARE(MyFont2);
LV_FONT_DECLARE(MyIcon);
// (LVGL 内置字体不需要在这里声明)

// --- 图片声明 ---

LV_IMG_DECLARE(logo20x20);
LV_IMG_DECLARE(main_blue);
LV_IMG_DECLARE(main_yellow);
LV_IMG_DECLARE(main_red);
LV_IMG_DECLARE(like); 
#endif // ASSETS_H