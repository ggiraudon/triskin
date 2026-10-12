// Line-based debug commands over USB serial.
//   shot              dump the current screen as raw RGB565 (see tools/screenshot.py)
//   get               print every setting
//   set <name> <n>    change a setting (applies immediately, not saved)
//   save              write the settings to flash
//   defaults          restore default settings (not saved)
//   status            print what the jiggle engine is doing
//   go                start a jiggle session now
//   reboot            restart the device
//   tap <x> <y>       simulate a touch at a screen position
//   hold <x> <y> <ms> simulate a long press
//   swipe <x0> <y0> <x1> <y1> <ms>   simulate a drag
#pragma once

namespace console {

void poll();

}  // namespace console
