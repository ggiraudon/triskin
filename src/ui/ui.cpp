#include "ui.h"

#include <lvgl.h>

#include "drivers/display.h"
#include "home.h"
#include "settings/config.h"
#include "settings_screen.h"

namespace ui {

namespace {

constexpr uint32_t SETTINGS_TIMEOUT_MS = 5000;
constexpr uint32_t FADE_MS = 150;
constexpr uint8_t DIMMED_PERCENT = 6;

lv_obj_t *homeScreen = nullptr;
lv_obj_t *settingsScreen = nullptr;
bool inSettings = false;
bool dimmed = false;

void housekeeping(lv_timer_t *) {
  const uint32_t idle = lv_display_get_inactive_time(nullptr);
  if (inSettings && idle >= SETTINGS_TIMEOUT_MS) {
    showHome();
  }

  const uint32_t dimAfterMs = config::get().dimAfterS * 1000u;
  const bool shouldDim = dimAfterMs != 0 && idle >= dimAfterMs;
  if (shouldDim != dimmed) {
    dimmed = shouldDim;
    display::setBrightness(dimmed ? DIMMED_PERCENT : static_cast<uint8_t>(config::get().brightness));
  }
}

}  // namespace

void begin() {
  lv_display_t *disp = lv_display_get_default();
  lv_display_set_theme(disp, lv_theme_default_init(disp, lv_palette_main(LV_PALETTE_DEEP_PURPLE),
                                                   lv_palette_main(LV_PALETTE_TEAL), true,
                                                   &lv_font_montserrat_14));
  homeScreen = home::create();
  settingsScreen = settings_screen::create();
  lv_screen_load(homeScreen);
  lv_timer_create(housekeeping, 100, nullptr);
}

void showSettings() {
  if (inSettings) {
    return;
  }
  inSettings = true;
  settings_screen::refresh();
  settings_screen::showFirstPage();
  lv_screen_load_anim(settingsScreen, LV_SCREEN_LOAD_ANIM_FADE_IN, FADE_MS, 0, false);
}

void showHome() {
  if (!inSettings) {
    return;
  }
  inSettings = false;
  config::save();
  lv_screen_load_anim(homeScreen, LV_SCREEN_LOAD_ANIM_FADE_IN, FADE_MS, 0, false);
}

}  // namespace ui
