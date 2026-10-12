// The settings: six pages side by side, swiped left and right, with a row of
// page dots underneath.
#pragma once

#include <lvgl.h>

namespace settings_screen {

lv_obj_t *create();

// Makes every control show the current settings.
void refresh();

void showFirstPage();

}  // namespace settings_screen
