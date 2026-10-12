#include "controls.h"

#include <stdio.h>
#include <stdlib.h>

#include "settings/config.h"

namespace controls {

namespace {

enum class Kind : uint8_t { Slider, Choice, Toggle };

struct Control {
  Kind kind;
  uint16_t Settings::*field;
  Visible visible;
  lv_obj_t *row;     // the object hidden when the control does not apply
  lv_obj_t *widget;  // slider, button matrix or switch
  // Slider only:
  lv_obj_t *valueLabel;
  const char *label;
  const uint16_t *steps;
  uint8_t stepCount;
  Format format;
};

constexpr size_t MAX_CONTROLS = 24;
Control all[MAX_CONTROLS];
size_t count = 0;

Control *add(Kind kind, uint16_t Settings::*field, Visible visible) {
  LV_ASSERT(count < MAX_CONTROLS);
  Control *c = &all[count++];
  *c = {};
  c->kind = kind;
  c->field = field;
  c->visible = visible;
  return c;
}

void store(const Control *c, uint16_t value) {
  Settings s = config::get();
  s.*c->field = value;
  config::set(s);
  refreshAll();  // other controls may depend on this one (visibility, min <= max)
}

uint8_t nearestStep(const Control *c, uint16_t value) {
  uint8_t best = 0;
  for (uint8_t i = 1; i < c->stepCount; i++) {
    if (abs(static_cast<int>(c->steps[i]) - value) < abs(static_cast<int>(c->steps[best]) - value)) {
      best = i;
    }
  }
  return best;
}

void showSliderValue(const Control *c, uint16_t value) {
  char text[24];
  c->format(text, sizeof(text), value);
  lv_label_set_text_fmt(c->valueLabel, "%s  %s", c->label, text);
}

void onSlider(lv_event_t *e) {
  auto *c = static_cast<Control *>(lv_event_get_user_data(e));
  const uint16_t value = c->steps[lv_slider_get_value(c->widget)];
  if (lv_event_get_code(e) == LV_EVENT_RELEASED) {
    store(c, value);
  } else {
    showSliderValue(c, value);  // live feedback while dragging
  }
}

void onChoice(lv_event_t *e) {
  auto *c = static_cast<Control *>(lv_event_get_user_data(e));
  const uint32_t selected = lv_buttonmatrix_get_selected_button(c->widget);
  if (selected != LV_BUTTONMATRIX_BUTTON_NONE) {
    store(c, static_cast<uint16_t>(selected));
  }
}

void onToggle(lv_event_t *e) {
  auto *c = static_cast<Control *>(lv_event_get_user_data(e));
  store(c, lv_obj_has_state(c->widget, LV_STATE_CHECKED) ? 1 : 0);
}

lv_obj_t *plainRow(lv_obj_t *parent) {
  lv_obj_t *row = lv_obj_create(parent);
  lv_obj_remove_style_all(row);
  lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
  lv_obj_remove_flag(row, LV_OBJ_FLAG_SCROLLABLE);
  return row;
}

}  // namespace

lv_obj_t *page(lv_obj_t *tile, const char *title) {
  lv_obj_set_flex_flow(tile, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(tile, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_hor(tile, 16, 0);
  lv_obj_set_style_pad_top(tile, 12, 0);
  lv_obj_set_style_pad_row(tile, 14, 0);
  lv_obj_remove_flag(tile, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *label = lv_label_create(tile);
  lv_label_set_text(label, title);
  lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
  return tile;
}

void slider(lv_obj_t *parent, const char *label, uint16_t Settings::*field, const uint16_t *steps,
            uint8_t stepCount, Format format, Visible visible) {
  Control *c = add(Kind::Slider, field, visible);
  c->label = label;
  c->steps = steps;
  c->stepCount = stepCount;
  c->format = format;

  c->row = plainRow(parent);
  lv_obj_set_flex_flow(c->row, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_row(c->row, 12, 0);
  lv_obj_set_style_pad_bottom(c->row, 8, 0);

  c->valueLabel = lv_label_create(c->row);
  c->widget = lv_slider_create(c->row);
  lv_obj_set_width(c->widget, LV_PCT(88));
  lv_obj_set_style_margin_left(c->widget, 12, 0);
  lv_slider_set_range(c->widget, 0, stepCount - 1);
  lv_obj_set_ext_click_area(c->widget, 14);
  lv_obj_add_event_cb(c->widget, onSlider, LV_EVENT_VALUE_CHANGED, c);
  lv_obj_add_event_cb(c->widget, onSlider, LV_EVENT_RELEASED, c);
}

void choice(lv_obj_t *parent, uint16_t Settings::*field, const char *const *map, Visible visible) {
  Control *c = add(Kind::Choice, field, visible);
  c->row = c->widget = lv_buttonmatrix_create(parent);
  lv_buttonmatrix_set_map(c->widget, map);
  lv_buttonmatrix_set_button_ctrl_all(c->widget, LV_BUTTONMATRIX_CTRL_CHECKABLE);
  lv_buttonmatrix_set_one_checked(c->widget, true);
  lv_obj_set_size(c->widget, LV_PCT(100), 44);
  lv_obj_set_style_pad_all(c->widget, 0, 0);
  lv_obj_set_style_border_width(c->widget, 0, 0);
  lv_obj_set_style_bg_opa(c->widget, LV_OPA_TRANSP, 0);
  lv_obj_add_event_cb(c->widget, onChoice, LV_EVENT_VALUE_CHANGED, c);
}

void toggle(lv_obj_t *parent, const char *label, uint16_t Settings::*field, Visible visible) {
  Control *c = add(Kind::Toggle, field, visible);
  c->row = plainRow(parent);
  lv_obj_set_flex_flow(c->row, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(c->row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  lv_obj_t *text = lv_label_create(c->row);
  lv_label_set_text(text, label);
  c->widget = lv_switch_create(c->row);
  lv_obj_set_ext_click_area(c->widget, 10);
  lv_obj_add_event_cb(c->widget, onToggle, LV_EVENT_VALUE_CHANGED, c);
}

void refreshAll() {
  const Settings &s = config::get();
  for (size_t i = 0; i < count; i++) {
    const Control *c = &all[i];
    const uint16_t value = s.*c->field;
    lv_obj_set_flag(c->row, LV_OBJ_FLAG_HIDDEN, c->visible && !c->visible(s));
    switch (c->kind) {
      case Kind::Slider:
        lv_slider_set_value(c->widget, nearestStep(c, value), LV_ANIM_OFF);
        showSliderValue(c, value);
        break;
      case Kind::Choice:
        lv_buttonmatrix_set_button_ctrl(c->widget, value, LV_BUTTONMATRIX_CTRL_CHECKED);
        break;
      case Kind::Toggle:
        lv_obj_set_state(c->widget, LV_STATE_CHECKED, value != 0);
        break;
    }
  }
}

void formatSeconds(char *buf, size_t size, uint16_t seconds) {
  if (seconds < 60) {
    snprintf(buf, size, "%u s", seconds);
  } else if (seconds < 3600) {
    if (seconds % 60) {
      snprintf(buf, size, "%u min %u s", seconds / 60, seconds % 60);
    } else {
      snprintf(buf, size, "%u min", seconds / 60);
    }
  } else {
    snprintf(buf, size, "%u h", seconds / 3600);
  }
}

void formatTenths(char *buf, size_t size, uint16_t tenths) {
  if (tenths % 10) {
    snprintf(buf, size, "%u.%u s", tenths / 10, tenths % 10);
  } else {
    snprintf(buf, size, "%u s", tenths / 10);
  }
}

void formatCounts(char *buf, size_t size, uint16_t counts) { snprintf(buf, size, "%u", counts); }

void formatPercent(char *buf, size_t size, uint16_t percent) { snprintf(buf, size, "%u%%", percent); }

void formatDimDelay(char *buf, size_t size, uint16_t seconds) {
  if (seconds == 0) {
    snprintf(buf, size, "never");
  } else {
    formatSeconds(buf, size, seconds);
  }
}

}  // namespace controls
