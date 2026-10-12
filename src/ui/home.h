// The home screen: what the device shows when nobody is touching it.
// Tap opens the settings; a long press pauses or resumes jiggling.
#pragma once

#include <lvgl.h>

namespace home {

lv_obj_t *create();

}  // namespace home
