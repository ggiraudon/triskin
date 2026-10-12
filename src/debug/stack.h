// High-water mark for core 0's stack, which is where LVGL runs.
#pragma once

namespace stack {

// Fills the unused part of the stack with a known pattern. Call first thing
// in setup().
void paint();

// Bytes of stack that have never been touched since paint(). If this gets
// near zero the UI is about to overflow its stack.
int minFree();

}  // namespace stack
