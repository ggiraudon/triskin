// The mascot: a full-screen animation played from pre-rendered frames.
#pragma once

#include <lvgl.h>

namespace character {

enum class Mood : uint8_t {
  Still,    // frozen and dimmed: nothing is going to happen
  Dancing,  // the idle loop
  Shaking,  // the butt shake, while the cursor is being jiggled
};

// Creates the animation as the background of `parent`. Starts out Still.
void create(lv_obj_t *parent);

// Changes what the mascot is doing; different loops cross-fade.
void setMood(Mood mood);

// How many frames have been put on screen since boot, for measuring the frame
// rate actually achieved.
uint32_t framesShown();

}  // namespace character
