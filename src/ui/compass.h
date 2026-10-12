// The direction picker: a ring of eight dots at 45 degree steps. Each dot
// toggles whether the cursor may move that way; a spoke from the centre shows
// the ones that are on. Bound to Settings::directions.
#pragma once

#include <lvgl.h>

namespace compass {

void create(lv_obj_t *parent);
void refresh();

}  // namespace compass
