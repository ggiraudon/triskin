// ST7789T3 panel on SPI0, registered with LVGL as its display.
#pragma once

#include <lvgl.h>

namespace display {

// Resets the panel and creates the LVGL display. Call after lv_init(). The
// reset line is shared with the touch controller, so call touch::begin() after
// this, never before. The backlight stays off until setBrightness().
lv_display_t *begin();

void setBrightness(uint8_t percent);

// Blocks until no pixel data is on its way to the panel. Call before anything
// that stops the world for a while, such as writing to flash.
void waitIdle();

}  // namespace display
