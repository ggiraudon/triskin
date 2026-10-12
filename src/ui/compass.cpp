#include "compass.h"

#include <math.h>

#include "settings/config.h"

namespace compass {

namespace {

constexpr int32_t SIZE = 204;
constexpr int32_t RADIUS = 80;
constexpr int32_t DOT = 34;
constexpr int32_t CENTRE = SIZE / 2;

lv_obj_t *dots[8];
lv_obj_t *spokes[8];
lv_point_precise_t spokePoints[8][2];

void onDot(lv_event_t *e) {
  const auto bit = static_cast<uint16_t>(1u << reinterpret_cast<uintptr_t>(lv_event_get_user_data(e)));
  Settings s = config::get();
  // The last remaining direction cannot be switched off.
  if ((s.directions ^ bit) != 0) {
    s.directions ^= bit;
    config::set(s);
  }
  refresh();
}

}  // namespace

void create(lv_obj_t *parent) {
  lv_obj_t *box = lv_obj_create(parent);
  lv_obj_remove_style_all(box);
  lv_obj_set_size(box, SIZE, SIZE);
  lv_obj_remove_flag(box, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *ring = lv_obj_create(box);
  lv_obj_remove_style_all(ring);
  lv_obj_set_size(ring, 2 * RADIUS, 2 * RADIUS);
  lv_obj_center(ring);
  lv_obj_set_style_radius(ring, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_border_width(ring, 2, 0);
  lv_obj_set_style_border_color(ring, lv_palette_darken(LV_PALETTE_GREY, 2), 0);
  lv_obj_remove_flag(ring, LV_OBJ_FLAG_CLICKABLE);

  const lv_color_t accent = lv_theme_get_color_primary(box);

  for (uint8_t i = 0; i < 8; i++) {
    const float angle = i * static_cast<float>(M_PI) / 4.0f;  // clockwise from up
    const float dx = sinf(angle), dy = -cosf(angle);

    spokePoints[i][0] = {static_cast<lv_value_precise_t>(CENTRE), static_cast<lv_value_precise_t>(CENTRE)};
    spokePoints[i][1] = {static_cast<lv_value_precise_t>(CENTRE + dx * (RADIUS - DOT / 2)),
                         static_cast<lv_value_precise_t>(CENTRE + dy * (RADIUS - DOT / 2))};
    spokes[i] = lv_line_create(box);
    lv_line_set_points(spokes[i], spokePoints[i], 2);
    lv_obj_set_style_line_width(spokes[i], 4, 0);
    lv_obj_set_style_line_rounded(spokes[i], true, 0);
    lv_obj_set_style_line_color(spokes[i], accent, 0);
  }

  for (uint8_t i = 0; i < 8; i++) {
    const float angle = i * static_cast<float>(M_PI) / 4.0f;
    dots[i] = lv_obj_create(box);
    lv_obj_remove_style_all(dots[i]);
    lv_obj_set_size(dots[i], DOT, DOT);
    lv_obj_set_pos(dots[i], lroundf(CENTRE + sinf(angle) * RADIUS - DOT / 2.0f),
                   lroundf(CENTRE - cosf(angle) * RADIUS - DOT / 2.0f));
    lv_obj_set_style_radius(dots[i], LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(dots[i], LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(dots[i], lv_palette_darken(LV_PALETTE_GREY, 3), 0);
    lv_obj_set_style_bg_color(dots[i], accent, LV_STATE_CHECKED);
    lv_obj_set_style_border_width(dots[i], 2, 0);
    lv_obj_set_style_border_color(dots[i], lv_palette_main(LV_PALETTE_GREY), 0);
    lv_obj_set_style_border_color(dots[i], lv_color_white(), LV_STATE_CHECKED);
    lv_obj_set_ext_click_area(dots[i], 6);
    lv_obj_add_flag(dots[i], LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(dots[i], onDot, LV_EVENT_CLICKED, reinterpret_cast<void *>(static_cast<uintptr_t>(i)));
  }

  lv_obj_t *hub = lv_obj_create(box);
  lv_obj_remove_style_all(hub);
  lv_obj_set_size(hub, 14, 14);
  lv_obj_center(hub);
  lv_obj_set_style_radius(hub, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_opa(hub, LV_OPA_COVER, 0);
  lv_obj_set_style_bg_color(hub, lv_color_white(), 0);
  lv_obj_remove_flag(hub, LV_OBJ_FLAG_CLICKABLE);
}

void refresh() {
  const uint16_t mask = config::get().directions;
  for (uint8_t i = 0; i < 8; i++) {
    const bool on = mask & (1u << i);
    lv_obj_set_state(dots[i], LV_STATE_CHECKED, on);
    lv_obj_set_flag(spokes[i], LV_OBJ_FLAG_HIDDEN, !on);
  }
}

}  // namespace compass
