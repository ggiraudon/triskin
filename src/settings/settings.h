// Everything the user can configure. Plain data with no hardware dependencies.
#pragma once

#include <stddef.h>
#include <stdint.h>

enum PathMode : uint16_t {
  PATH_WANDER = 0,        // random strokes confined to the bounding box
  PATH_OUT_AND_BACK = 1,  // one stroke out, the same stroke back, repeat
};

// Every field is a uint16_t so that one table (FIELDS) can describe them all:
// name, range and default. Booleans are 0 / 1.
struct Settings {
  uint16_t enabled;         // 0 = paused

  uint16_t intervalRandom;  // gap between sessions: 0 = fixed, 1 = random
  uint16_t intervalS;       // fixed gap, seconds
  uint16_t intervalMinS;    // random gap bounds, seconds
  uint16_t intervalMaxS;

  uint16_t durationRandom;  // session length: 0 = fixed, 1 = random
  uint16_t durationDs;      // fixed length, tenths of a second
  uint16_t durationMinDs;   // random length bounds, tenths of a second
  uint16_t durationMaxDs;

  uint16_t directions;      // bit n allows the direction n * 45 degrees clockwise from up
  uint16_t amplitude;       // stroke length, mouse counts
  uint16_t speed;           // 0 slow, 1 medium, 2 fast

  uint16_t pathMode;        // PathMode
  uint16_t boxW;            // bounding box around the starting point, mouse counts
  uint16_t boxH;
  uint16_t returnHome;      // end each session where it started

  uint16_t brightness;      // percent
  uint16_t dimAfterS;       // dim the screen after this long idle; 0 = never
};

struct Field {
  const char *name;
  uint16_t Settings::*member;
  uint16_t min, max, def;
};

// The order of this table is the order values are stored in flash: only ever
// append to it.
extern const Field FIELDS[];
extern const size_t FIELD_COUNT;

Settings defaultSettings();
const Field *findField(const char *name);

// Forces every value into range and every min <= max.
void sanitize(Settings &s);
