#include "home.h"

#include <string.h>

#include "character.h"
#include "controls.h"
#include "jiggle/task.h"
#include "settings/config.h"
#include "ui.h"

namespace home {

namespace {

lv_obj_t *statusLabel = nullptr;

void setStatus(const char *text) {
  // Setting the same text again would still redraw it, over the animation.
  if (strcmp(lv_label_get_text(statusLabel), text) != 0) {
    lv_label_set_text(statusLabel, text);
  }
  lv_obj_set_flag(statusLabel, LV_OBJ_FLAG_HIDDEN, text[0] == '\0');
}

void update(lv_timer_t *) {
  const jiggle::Status st = jiggle_task::status();
  switch (st.state) {
    case jiggle::State::NoHost:
      character::setMood(character::Mood::Still);
      setStatus("No computer connected");
      break;
    case jiggle::State::Paused:
      character::setMood(character::Mood::Still);
      setStatus("Paused - hold to resume");
      break;
    case jiggle::State::Waiting: {
      char left[24], text[40];
      controls::formatSeconds(left, sizeof(left), static_cast<uint16_t>((st.msToNext + 999) / 1000));
      lv_snprintf(text, sizeof(text), "next jiggle in %s", left);
      character::setMood(character::Mood::Dancing);
      setStatus(text);
      break;
    }
    case jiggle::State::Moving:
      character::setMood(character::Mood::Shaking);
      setStatus("");
      break;
  }
}

void togglePause(lv_event_t *) {
  Settings s = config::get();
  s.enabled = !s.enabled;
  config::set(s);
  config::save();
}

}  // namespace

lv_obj_t *create() {
  lv_obj_t *scr = lv_obj_create(nullptr);
  lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(scr, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(scr, [](lv_event_t *) { ui::showSettings(); }, LV_EVENT_SHORT_CLICKED, nullptr);
  lv_obj_add_event_cb(scr, togglePause, LV_EVENT_LONG_PRESSED, nullptr);

  character::create(scr);

  statusLabel = lv_label_create(scr);
  lv_label_set_text(statusLabel, "");
  // Above the horns: the bottom of the screen is the lit patch of floor.
  lv_obj_align(statusLabel, LV_ALIGN_TOP_MID, 0, 2);
  lv_obj_set_style_text_color(statusLabel, lv_color_white(), 0);
  lv_obj_set_style_text_opa(statusLabel, LV_OPA_70, 0);

  update(nullptr);
  lv_timer_create(update, 100, nullptr);
  return scr;
}

}  // namespace home
