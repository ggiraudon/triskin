// Screen management: home and settings, the timeout that returns from one to
// the other, and screen dimming.
#pragma once

namespace ui {

// Builds both screens and shows home. Call once the display, touch input and
// settings are up.
void begin();

void showSettings();

// Saves any changed settings and returns to the home screen.
void showHome();

}  // namespace ui
