#include "settings_screen.h"

#include "compass.h"
#include "controls.h"
#include "settings/config.h"

namespace settings_screen {

namespace {

constexpr uint8_t PAGES = 6;
constexpr int32_t DOTS_HEIGHT = 22;

// The values each slider snaps to. Wide ranges are stepped roughly
// geometrically so that both ends are usable on a 200 px track.
const uint16_t INTERVAL_STEPS[] = {1,   2,   3,   5,   10,  15,   20,   30,   45,   60,
                                   90,  120, 180, 300, 600, 900, 1200, 1800, 2700, 3600};
const uint16_t DURATION_STEPS[] = {2, 3, 5, 7, 10, 15, 20, 30, 40, 50, 75, 100, 150, 200, 300};
const uint16_t AMPLITUDE_STEPS[] = {1,  2,  3,  5,   8,   10,  15,  20,  30,
                                    40, 60, 80, 100, 150, 200, 300, 400, 500};
const uint16_t BOX_STEPS[] = {20, 40, 60, 100, 150, 200, 300, 400, 600, 800, 1000, 1500, 2000};
const uint16_t BRIGHTNESS_STEPS[] = {5,  10, 15, 20, 25, 30, 35, 40, 45, 50,
                                     55, 60, 65, 70, 75, 80, 85, 90, 95, 100};
const uint16_t DIM_STEPS[] = {15, 30, 60, 120, 300, 600, 0};  // 0 = never

const char *const FIXED_OR_RANDOM[] = {"Fixed", "Random", ""};
const char *const SPEEDS[] = {"Slow", "Medium", "Fast", ""};
const char *const PATH_MODES[] = {"Wander", "Out & back", ""};

template <size_t N>
constexpr uint8_t len(const uint16_t (&)[N]) {
  return static_cast<uint8_t>(N);
}

lv_obj_t *tileview = nullptr;
lv_obj_t *tiles[PAGES];
lv_obj_t *dots[PAGES];

void buildTiming(lv_obj_t *tile) {
  lv_obj_t *p = controls::page(tile, "Interval");
  controls::choice(p, &Settings::intervalRandom, FIXED_OR_RANDOM);
  controls::slider(p, "Every", &Settings::intervalS, INTERVAL_STEPS, len(INTERVAL_STEPS),
                   controls::formatSeconds, [](const Settings &s) { return !s.intervalRandom; });
  controls::slider(p, "At least", &Settings::intervalMinS, INTERVAL_STEPS, len(INTERVAL_STEPS),
                   controls::formatSeconds, [](const Settings &s) { return s.intervalRandom != 0; });
  controls::slider(p, "At most", &Settings::intervalMaxS, INTERVAL_STEPS, len(INTERVAL_STEPS),
                   controls::formatSeconds, [](const Settings &s) { return s.intervalRandom != 0; });
}

void buildDuration(lv_obj_t *tile) {
  lv_obj_t *p = controls::page(tile, "Jiggle length");
  controls::choice(p, &Settings::durationRandom, FIXED_OR_RANDOM);
  controls::slider(p, "For", &Settings::durationDs, DURATION_STEPS, len(DURATION_STEPS),
                   controls::formatTenths, [](const Settings &s) { return !s.durationRandom; });
  controls::slider(p, "At least", &Settings::durationMinDs, DURATION_STEPS, len(DURATION_STEPS),
                   controls::formatTenths, [](const Settings &s) { return s.durationRandom != 0; });
  controls::slider(p, "At most", &Settings::durationMaxDs, DURATION_STEPS, len(DURATION_STEPS),
                   controls::formatTenths, [](const Settings &s) { return s.durationRandom != 0; });
}

void buildDirections(lv_obj_t *tile) {
  lv_obj_t *p = controls::page(tile, "Directions");
  compass::create(p);
}

void buildMovement(lv_obj_t *tile) {
  lv_obj_t *p = controls::page(tile, "Movement");
  controls::slider(p, "Distance", &Settings::amplitude, AMPLITUDE_STEPS, len(AMPLITUDE_STEPS),
                   controls::formatCounts);
  lv_obj_t *label = lv_label_create(p);
  lv_label_set_text(label, "Speed");
  lv_obj_set_width(label, LV_PCT(100));
  controls::choice(p, &Settings::speed, SPEEDS);
}

void buildPath(lv_obj_t *tile) {
  const auto wandering = [](const Settings &s) { return s.pathMode == PATH_WANDER; };
  lv_obj_t *p = controls::page(tile, "Path");
  controls::choice(p, &Settings::pathMode, PATH_MODES);
  controls::slider(p, "Box width", &Settings::boxW, BOX_STEPS, len(BOX_STEPS), controls::formatCounts,
                   wandering);
  controls::slider(p, "Box height", &Settings::boxH, BOX_STEPS, len(BOX_STEPS), controls::formatCounts,
                   wandering);
  controls::toggle(p, "Return to start", &Settings::returnHome, wandering);
}

void buildDevice(lv_obj_t *tile) {
  lv_obj_t *p = controls::page(tile, "Device");
  controls::toggle(p, "Jiggling enabled", &Settings::enabled);
  controls::slider(p, "Brightness", &Settings::brightness, BRIGHTNESS_STEPS, len(BRIGHTNESS_STEPS),
                   controls::formatPercent);
  controls::slider(p, "Dim screen after", &Settings::dimAfterS, DIM_STEPS, len(DIM_STEPS),
                   controls::formatDimDelay);

  lv_obj_t *reset = lv_button_create(p);
  lv_obj_set_style_bg_color(reset, lv_palette_darken(LV_PALETTE_GREY, 2), 0);
  lv_obj_t *label = lv_label_create(reset);
  lv_label_set_text(label, "Hold to reset all");
  lv_obj_add_event_cb(reset, [](lv_event_t *) {
    config::set(defaultSettings());
    refresh();
  }, LV_EVENT_LONG_PRESSED, nullptr);
}

void onPageChanged(lv_event_t *) {
  lv_obj_t *active = lv_tileview_get_tile_active(tileview);
  for (uint8_t i = 0; i < PAGES; i++) {
    lv_obj_set_style_bg_opa(dots[i], tiles[i] == active ? LV_OPA_COVER : LV_OPA_30, 0);
  }
}

}  // namespace

lv_obj_t *create() {
  lv_obj_t *scr = lv_obj_create(nullptr);
  lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

  tileview = lv_tileview_create(scr);
  lv_obj_set_size(tileview, LV_PCT(100), lv_display_get_vertical_resolution(nullptr) - DOTS_HEIGHT);
  lv_obj_set_style_bg_opa(tileview, LV_OPA_TRANSP, 0);
  lv_obj_set_scrollbar_mode(tileview, LV_SCROLLBAR_MODE_OFF);
  for (uint8_t i = 0; i < PAGES; i++) {
    tiles[i] = lv_tileview_add_tile(tileview, i, 0, LV_DIR_HOR);
  }
  buildTiming(tiles[0]);
  buildDuration(tiles[1]);
  buildDirections(tiles[2]);
  buildMovement(tiles[3]);
  buildPath(tiles[4]);
  buildDevice(tiles[5]);
  lv_obj_add_event_cb(tileview, onPageChanged, LV_EVENT_VALUE_CHANGED, nullptr);

  lv_obj_t *row = lv_obj_create(scr);
  lv_obj_remove_style_all(row);
  lv_obj_set_size(row, LV_PCT(100), DOTS_HEIGHT);
  lv_obj_align(row, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_column(row, 8, 0);
  for (uint8_t i = 0; i < PAGES; i++) {
    dots[i] = lv_obj_create(row);
    lv_obj_remove_style_all(dots[i]);
    lv_obj_set_size(dots[i], 8, 8);
    lv_obj_set_style_radius(dots[i], LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(dots[i], lv_color_white(), 0);
  }
  onPageChanged(nullptr);
  return scr;
}

void refresh() {
  controls::refreshAll();
  compass::refresh();
}

void showFirstPage() {
  lv_tileview_set_tile_by_index(tileview, 0, 0, LV_ANIM_OFF);
  onPageChanged(nullptr);
}

}  // namespace settings_screen
