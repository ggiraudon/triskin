// CST816D capacitive touch controller on I2C0, registered with LVGL as a pointer.
#pragma once

#include <lvgl.h>

namespace touch {

// Call after display::begin(), which releases the reset line the two share.
// Returns nullptr if the controller does not respond.
lv_indev_t *begin();

// Debug aid: makes LVGL see a finger travel from (x0, y0) to (x1, y1) over
// `ms` milliseconds, in screen coordinates. A tap is the same point twice.
void simulate(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint32_t ms);

}  // namespace touch
