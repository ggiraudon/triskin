#include "stack.h"

#include <Arduino.h>

// Core 0's stack grows down from the top of SCRATCH_Y through SCRATCH_X (8 KB
// in all, since core 1 has its own stack; see core1_separate_stack in main.cpp).
// Anything the linker placed in SCRATCH_X ends at __scratch_x_end__.
extern "C" uint32_t __scratch_x_end__;

namespace stack {

namespace {

constexpr uint32_t PATTERN = 0xA5C35A3Cu;
constexpr uint32_t MARGIN = 128;  // leave paint()'s own frame and its callers alone

uint32_t *bottom() { return &__scratch_x_end__; }

}  // namespace

void paint() {
  uint32_t *const end = reinterpret_cast<uint32_t *>((rp2040.getStackPointer() - MARGIN) & ~3u);
  for (uint32_t *p = bottom(); p < end; p++) {
    *p = PATTERN;
  }
}

int minFree() {
  const uint32_t *p = bottom();
  const uint32_t *const end = reinterpret_cast<uint32_t *>(rp2040.getStackPointer());
  while (p < end && *p == PATTERN) {
    p++;
  }
  return static_cast<int>(reinterpret_cast<uintptr_t>(p) - reinterpret_cast<uintptr_t>(bottom()));
}

}  // namespace stack
