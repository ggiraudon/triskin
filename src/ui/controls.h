// Building blocks for the settings pages. Each control is bound to one field
// of Settings: touching it updates the live settings, and refreshAll() makes
// every control show the current value again.
#pragma once

#include <lvgl.h>

#include "settings/settings.h"

namespace controls {

using Format = void (*)(char *buf, size_t size, uint16_t value);
using Visible = bool (*)(const Settings &);

// A titled page filling `tile`; returns the column to add controls to.
lv_obj_t *page(lv_obj_t *tile, const char *title);

// A slider that snaps to the values in `steps`, shown as "label  value".
// Hidden whenever `visible` returns false.
void slider(lv_obj_t *parent, const char *label, uint16_t Settings::*field, const uint16_t *steps,
            uint8_t stepCount, Format format, Visible visible = nullptr);

// A row of mutually exclusive buttons; the field holds the selected index.
// `map` is an LVGL button-matrix map: labels followed by an empty string.
void choice(lv_obj_t *parent, uint16_t Settings::*field, const char *const *map,
            Visible visible = nullptr);

// An on/off switch with a label.
void toggle(lv_obj_t *parent, const char *label, uint16_t Settings::*field,
            Visible visible = nullptr);

void refreshAll();

// Value formatters.
void formatSeconds(char *buf, size_t size, uint16_t seconds);
void formatTenths(char *buf, size_t size, uint16_t tenthsOfASecond);
void formatCounts(char *buf, size_t size, uint16_t counts);
void formatPercent(char *buf, size_t size, uint16_t percent);
void formatDimDelay(char *buf, size_t size, uint16_t seconds);

}  // namespace controls
