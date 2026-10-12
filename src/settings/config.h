// The live settings: the one copy the UI edits, the engine follows and flash
// remembers. Core 0 only.
#pragma once

#include "settings.h"

namespace config {

// Loads the saved settings (or defaults) and applies them.
void begin();

const Settings &get();

// Sanitizes s, makes it current and applies it. Does not write to flash.
void set(Settings s);

// Writes the current settings to flash if they differ from what is stored.
// Returns false only if the write failed.
bool save();

}  // namespace config
