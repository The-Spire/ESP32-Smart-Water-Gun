#include "LedStrip.h"

// ------------------ MovingLightEffect 实现 ------------------
MovingLightEffect::MovingLightEffect(int start, int end, int width, FlowDirection dir, CRGB c, unsigned long interval)
    : startIndex(start), endIndex(end), lightWidth(width), direction(dir), color(c), updateInterval(interval)
{
    regionSize = end - start + 1;
    previousMillis = 0;
    reset();
}

void MovingLightEffect::reset() {
    currentPosition = (direction == DIRECTION_BACKWARD) ? (regionSize - 1) : 0;
}

void MovingLightEffect::update(CRGB* leds) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= updateInterval) {
        previousMillis = currentMillis;

        // 动态计算并填充背景暗色
        CRGB dynamic_dim_color = color;
        dynamic_dim_color.nscale8(20);

        for (int i = startIndex; i <= endIndex; i++) {
            leds[i] = dynamic_dim_color;
        }

        // 静态模式：高亮覆盖整个区域
        if (lightWidth >= regionSize) {
            for (int i = startIndex; i <= endIndex; i++) {
                leds[i] = color;
            }
        } else {
            // 流动模式：绘制高亮段
            for (int i = 0; i < lightWidth; i++) {
                int pos = (direction == DIRECTION_BACKWARD) ?
                          ((currentPosition - i + regionSize) % regionSize) :
                          ((currentPosition + i) % regionSize);
                leds[startIndex + pos] = color;
            }

            // 更新位置
            currentPosition = (direction == DIRECTION_BACKWARD) ?
                              ((currentPosition + regionSize - 1) % regionSize) :
                              ((currentPosition + 1) % regionSize);
        }
    }
}


// ------------------ LedStrip 实现 ------------------
LedStrip::LedStrip() :
    _effects{
        {0, 1, 2, DIRECTION_FORWARD, CRGB::Blue},
        {2, 23, 3, DIRECTION_FORWARD, CRGB::Blue},
        {24, 34, 3, DIRECTION_FORWARD, CRGB::Blue},
        {35, 39, 5, DIRECTION_FORWARD, CRGB::Blue},
        {40, 50, 3, DIRECTION_BACKWARD, CRGB::Blue},
        {51, 67, 3, DIRECTION_BACKWARD, CRGB::Blue},
        {68, 84, 3, DIRECTION_BACKWARD, CRGB::Blue}
    }
{
    _lastUpdateTime = 0;
}

void LedStrip::init() {
    FastLED.addLeds<WS2812B, LED_DATA_PIN, GRB>(_leds, NUM_LEDS);
    FastLED.setBrightness(LED_BRIGHTNESS);
    FastLED.clear();
    for (int i = 0; i < _effectCount; i++) {
        _effects[i].reset();
    }
    FastLED.show();
}

void LedStrip::update() {
    if (millis() - _lastUpdateTime > 20) {
        for (int i = 0; i < _effectCount; i++) {
            _effects[i].update(_leds);
        }
        FastLED.show();
        _lastUpdateTime = millis();
    }
}

/**
 * @brief 将 RGB565 转换为 FastLED 的 CRGB
 * @param color565 16-bit RGB565
 * @return CRGB 24-bit 颜色
 */
static CRGB convertRGB565toCRGB(uint16_t color565) {
    uint8_t r5 = (color565 >> 11) & 0x1F;
    uint8_t g6 = (color565 >> 5) & 0x3F;
    uint8_t b5 = color565 & 0x1F;
    uint8_t r8 = (r5 * 255 + 15) / 31;
    uint8_t g8 = (g6 * 255 + 31) / 63;
    uint8_t b8 = (b5 * 255 + 15) / 31;
    return CRGB(r8, g8, b8);
}

void LedStrip::setAllEffectsSpeed(int speed_setting) {
    unsigned long interval = (speed_setting > 0) ? (1000 / speed_setting) : 1000;
    if (interval < 10) interval = 10;

    for (int i = 0; i < _effectCount; i++) {
        _effects[i].updateInterval = interval;
    }
}

void LedStrip::setAllEffectsColor(uint16_t color_setting) {
    CRGB new_color = convertRGB565toCRGB(color_setting);
    for (int i = 0; i < _effectCount; i++) {
        _effects[i].color = new_color;
    }
}