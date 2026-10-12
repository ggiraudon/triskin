#include "settings.h"

#include <string.h>

const Field FIELDS[] = {
    {"enabled", &Settings::enabled, 0, 1, 1},
    {"intervalRandom", &Settings::intervalRandom, 0, 1, 0},
    {"intervalS", &Settings::intervalS, 1, 3600, 60},
    {"intervalMinS", &Settings::intervalMinS, 1, 3600, 30},
    {"intervalMaxS", &Settings::intervalMaxS, 1, 3600, 120},
    {"durationRandom", &Settings::durationRandom, 0, 1, 0},
    {"durationDs", &Settings::durationDs, 2, 300, 20},
    {"durationMinDs", &Settings::durationMinDs, 2, 300, 10},
    {"durationMaxDs", &Settings::durationMaxDs, 2, 300, 40},
    {"directions", &Settings::directions, 1, 255, 255},
    // Small by default, and out-and-back so the cursor never strays further than
    // this: someone working at the computer should not notice the jiggle.
    {"amplitude", &Settings::amplitude, 1, 500, 3},
    {"speed", &Settings::speed, 0, 2, 1},
    {"pathMode", &Settings::pathMode, 0, 1, PATH_OUT_AND_BACK},
    {"boxW", &Settings::boxW, 20, 2000, 300},
    {"boxH", &Settings::boxH, 20, 2000, 300},
    {"returnHome", &Settings::returnHome, 0, 1, 1},
    {"brightness", &Settings::brightness, 5, 100, 80},
    {"dimAfterS", &Settings::dimAfterS, 0, 3600, 0},
};
const size_t FIELD_COUNT = sizeof(FIELDS) / sizeof(FIELDS[0]);

Settings defaultSettings() {
  Settings s{};
  for (size_t i = 0; i < FIELD_COUNT; i++) {
    s.*FIELDS[i].member = FIELDS[i].def;
  }
  return s;
}

const Field *findField(const char *name) {
  for (size_t i = 0; i < FIELD_COUNT; i++) {
    if (strcmp(FIELDS[i].name, name) == 0) {
      return &FIELDS[i];
    }
  }
  return nullptr;
}

void sanitize(Settings &s) {
  for (size_t i = 0; i < FIELD_COUNT; i++) {
    uint16_t &v = s.*FIELDS[i].member;
    if (v < FIELDS[i].min) {
      v = FIELDS[i].min;
    } else if (v > FIELDS[i].max) {
      v = FIELDS[i].max;
    }
  }
  if (s.intervalMaxS < s.intervalMinS) {
    s.intervalMaxS = s.intervalMinS;
  }
  if (s.durationMaxDs < s.durationMinDs) {
    s.durationMaxDs = s.durationMinDs;
  }
}
