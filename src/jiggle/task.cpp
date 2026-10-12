#include "task.h"

#include <Arduino.h>
#include <pico/mutex.h>

#include "usb/hid.h"

namespace {

constexpr uint32_t TICK_MS = 10;  // matches the HID endpoint's 10 ms polling interval

auto_init_mutex(lock);
Settings pendingSettings;
bool settingsPending = false;
bool triggerPending = false;
jiggle::Status published = {jiggle::State::NoHost, 0, 0, 0, 0};

}  // namespace

namespace jiggle_task {

void loop() {
  static jiggle::Engine engine(rp2040.hwrand32());
  static uint32_t lastTick = 0;

  const uint32_t now = millis();
  if (now - lastTick < TICK_MS) {
    delay(1);
    return;
  }
  lastTick = now;

  mutex_enter_blocking(&lock);
  if (settingsPending) {
    engine.configure(pendingSettings, now);
    settingsPending = false;
  }
  if (triggerPending) {
    engine.triggerNow();
    triggerPending = false;
  }
  mutex_exit(&lock);

  const jiggle::Delta d = engine.tick(now, hid::hostActive());
  if (d.dx || d.dy) {
    hid::move(d.dx, d.dy);
  }

  mutex_enter_blocking(&lock);
  published = engine.status(now);
  mutex_exit(&lock);
}

void applySettings(const Settings &s) {
  mutex_enter_blocking(&lock);
  pendingSettings = s;
  settingsPending = true;
  mutex_exit(&lock);
}

void triggerNow() {
  mutex_enter_blocking(&lock);
  triggerPending = true;
  mutex_exit(&lock);
}

jiggle::Status status() {
  mutex_enter_blocking(&lock);
  const jiggle::Status s = published;
  mutex_exit(&lock);
  return s;
}

}  // namespace jiggle_task
