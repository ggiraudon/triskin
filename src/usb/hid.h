// The device's USB mouse.
#pragma once

#include <stdint.h>

namespace hid {

// Adds the mouse to the USB device. The host sees a disconnect and reconnect.
void begin();

// True while a host has configured the device and has not suspended the bus.
// Movement sent at any other time goes nowhere.
bool hostActive();

// Relative movement in mouse counts, positive is right / down.
void move(int8_t dx, int8_t dy);

}  // namespace hid
