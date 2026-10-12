#include "character.h"

#include "assets/character.h"

namespace character {

namespace {

constexpr uint32_t FRAME_BYTES = CHARACTER_W * CHARACTER_H * 2;
constexpr uint32_t FRAME_MS = 1000 / CHARACTER_FPS;
constexpr uint32_t FADE_MS = 3 * FRAME_MS;
// tick() runs several times per frame so that a new frame is noticed promptly;
// checking only once per frame period measurably dropped frames (12.8 fps of 15).
constexpr uint32_t TICK_MS = FRAME_MS / 4;
constexpr lv_opa_t STILL_DIM = LV_OPA_50;

struct Loop {
  uint16_t first, count;  // position in character_frames
};
constexpr Loop IDLE = {0, CHARACTER_IDLE_FRAMES};
constexpr Loop SHAKE = {CHARACTER_IDLE_FRAMES, CHARACTER_SHAKE_FRAMES};

lv_image_dsc_t frames[CHARACTER_IDLE_FRAMES + CHARACTER_SHAKE_FRAMES];

// `front` plays the current loop. During a cross-fade `back` holds the last
// frame of the loop being left, and front fades in over it.
lv_obj_t *front = nullptr;
lv_obj_t *back = nullptr;

Mood mood = Mood::Still;
const Loop *loop = &IDLE;
uint16_t frame = 0;         // within the loop
uint32_t loopStartMs = 0;   // when frame 0 of the loop was (or would have been) shown
bool fading = false;
uint32_t fadeStartMs = 0;
uint32_t shownCount = 0;

void show(uint16_t index) {
  frame = index;
  shownCount++;
  lv_image_set_src(front, &frames[loop->first + frame]);
}

void endFade() {
  fading = false;
  lv_obj_set_style_image_opa(front, LV_OPA_COVER, 0);
  lv_obj_add_flag(back, LV_OBJ_FLAG_HIDDEN);
}

void tick(lv_timer_t *) {
  const uint32_t now = lv_tick_get();
  if (fading && now - fadeStartMs >= FADE_MS) {
    endFade();
  }
  if (mood == Mood::Still) {
    return;
  }
  // Derived from the clock, so a slow frame is skipped, not played late.
  const uint16_t index = ((now - loopStartMs) / FRAME_MS) % loop->count;
  if (index != frame) {
    if (fading) {
      // Step the cross-fade with the frames: each change redraws the screen.
      lv_obj_set_style_image_opa(front, (now - fadeStartMs) * LV_OPA_COVER / FADE_MS, 0);
    }
    show(index);
  }
}

void switchLoop(const Loop *next) {
  if (next == loop) {
    return;
  }
  lv_image_set_src(back, &frames[loop->first + frame]);
  lv_obj_remove_flag(back, LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_style_image_opa(front, LV_OPA_TRANSP, 0);
  loop = next;
  loopStartMs = fadeStartMs = lv_tick_get();
  fading = true;
  show(0);
}

}  // namespace

void create(lv_obj_t *parent) {
  for (size_t i = 0; i < sizeof(frames) / sizeof(frames[0]); i++) {
    frames[i] = {};
    frames[i].header.magic = LV_IMAGE_HEADER_MAGIC;
    frames[i].header.cf = LV_COLOR_FORMAT_RGB565;
    frames[i].header.w = CHARACTER_W;
    frames[i].header.h = CHARACTER_H;
    frames[i].header.stride = CHARACTER_W * 2;
    frames[i].data_size = FRAME_BYTES;
    frames[i].data = character_frames + i * FRAME_BYTES;
  }

  back = lv_image_create(parent);
  lv_obj_add_flag(back, LV_OBJ_FLAG_HIDDEN);
  front = lv_image_create(parent);
  lv_obj_set_style_image_recolor(front, lv_color_black(), 0);
  lv_obj_set_style_image_recolor_opa(front, STILL_DIM, 0);
  show(0);
  lv_timer_create(tick, TICK_MS, nullptr);
}

void setMood(Mood next) {
  if (next == mood) {
    return;
  }
  if (mood == Mood::Still) {
    // Carry on from the frame it froze on.
    loopStartMs = lv_tick_get() - frame * FRAME_MS;
    lv_obj_set_style_image_recolor_opa(front, LV_OPA_TRANSP, 0);
  }
  mood = next;
  if (mood == Mood::Still) {
    endFade();  // never freeze on a half-blended frame
    lv_obj_set_style_image_recolor_opa(front, STILL_DIM, 0);
  } else {
    switchLoop(mood == Mood::Shaking ? &SHAKE : &IDLE);
  }
}

uint32_t framesShown() { return shownCount; }

}  // namespace character
