// Triskin: a USB mouse jiggler with a touch UI.
// Core 0 runs the UI; core 1 runs the jiggle engine and talks to the host.
#include <Arduino.h>
#include <lvgl.h>

#include "debug/console.h"
#include "debug/stack.h"
#include "drivers/display.h"
#include "drivers/touch.h"
#include "jiggle/task.h"
#include "settings/config.h"
#include "ui/ui.h"
#include "usb/hid.h"

// By default the two cores share one 8 KB stack, 4 KB each. LVGL needs more
// than that on core 0 and overflows into core 1's half, which stops the jiggle
// engine or hangs the device. This gives core 1 a stack of its own.
bool core1_separate_stack = true;

void setup() {
  stack::paint();
  Serial.begin(115200);
  hid::begin();

  lv_init();
  lv_tick_set_cb([]() -> uint32_t { return millis(); });
  display::begin();
  touch::begin();
  ui::begin();

  lv_timer_handler();  // draw the first frame before the backlight comes on
  config::begin();     // loads settings; applying them turns the backlight on
}

void loop() {
  lv_timer_handler();
  console::poll();
  delay(2);
}

void loop1() { jiggle_task::loop(); }
