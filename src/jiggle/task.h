// Runs the jiggle engine on core 1 and sends its movement to the host, so that
// nothing the UI does on core 0 can make the cursor stutter. The functions
// other than loop() are safe to call from core 0.
#pragma once

#include "jiggle/engine.h"
#include "settings/settings.h"

namespace jiggle_task {

// Call continuously from loop1().
void loop();

void applySettings(const Settings &s);
void triggerNow();
jiggle::Status status();

}  // namespace jiggle_task
