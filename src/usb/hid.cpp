#include "hid.h"

#include <Arduino.h>
#include <Mouse.h>
#include <USB.h>
#include <tusb.h>

namespace hid {

void begin() { Mouse.begin(); }

bool hostActive() {
  CoreMutex m(&USB.mutex);
  return tud_mounted() && !tud_suspended();
}

void move(int8_t dx, int8_t dy) { Mouse.move(dx, dy); }

}  // namespace hid
