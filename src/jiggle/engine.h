// The jiggle engine: decides when the cursor moves and where to. Pure logic:
// it is told the time and whether a host is listening, and answers with the
// mouse movement to send. It never touches hardware.
//
// The device cannot know where the cursor is on screen, so the engine tracks a
// virtual position: the cursor's offset, in mouse counts, from where it was
// when the engine last considered it "home".
//
// Counts are not pixels: hosts apply pointer acceleration, so the distance a
// report moves the cursor depends on how fast the reports arrive. Two paths
// with the same length in counts can differ by hundreds of pixels. The only
// way back to the starting pixel is therefore to replay the outbound reports
// in reverse, which is what the engine does (see Phase).
#pragma once

#include <stdint.h>

#include "settings/settings.h"

namespace jiggle {

enum class State : uint8_t {
  NoHost,   // nothing is listening on USB
  Paused,   // disabled by the user
  Waiting,  // counting down to the next session
  Moving,   // a session is running
};

struct Status {
  State state;
  uint32_t msToNext;  // meaningful while Waiting
  int16_t x, y;       // virtual cursor position
  int8_t dirX;        // -1 / 0 / +1: which way the cursor is travelling sideways
};

struct Delta {
  int8_t dx, dy;
};

class Engine {
 public:
  explicit Engine(uint32_t seed);

  // Replaces the settings. Takes effect at the next stroke; if no session is
  // running, the wait for the next one restarts under the new timing.
  void configure(const Settings &s, uint32_t nowMs);

  // Call at a steady rate (every 10 ms). Returns the movement to send now.
  Delta tick(uint32_t nowMs, bool hostActive);

  // Starts a session at the next tick, even while paused.
  void triggerNow() { trigger_ = true; }

  Status status(uint32_t nowMs) const;

 private:
  struct Point {
    float x, y;
  };

  // A session alternates between generating movement, which is logged, and
  // undoing it by replaying the log backwards with every report negated.
  enum class Phase : uint8_t { Out, Back };

  // 20 s of reports. The out phase is at most half of the longest session
  // (15 s) plus one stroke (4.2 s for the longest stroke at the slowest speed).
  static constexpr uint16_t LOG_SIZE = 2000;

  uint32_t random(uint32_t lo, uint32_t hi);
  void scheduleNext(uint32_t nowMs);
  void startStroke(Point to, uint32_t nowMs);
  void planStroke(uint32_t nowMs);
  void endSession(uint32_t nowMs);
  Delta stepOut(uint32_t nowMs);
  Delta stepBack(uint32_t nowMs);
  bool insideBox(Point p) const;
  bool atHome() const { return sentX_ == 0 && sentY_ == 0; }

  Settings s_;
  uint32_t rng_;

  bool hostActive_ = false;
  bool inSession_ = false;
  Phase phase_ = Phase::Out;
  bool trigger_ = false;
  bool needSchedule_ = true;
  uint32_t nextStartMs_ = 0;
  uint32_t sessionStartMs_ = 0;
  uint32_t sessionLengthMs_ = 0;

  Point from_{0, 0}, to_{0, 0};
  uint32_t strokeStartMs_ = 0;
  uint32_t strokeLengthMs_ = 1;
  int32_t sentX_ = 0, sentY_ = 0;  // virtual position actually sent so far
  int8_t dirX_ = 0;

  Delta log_[LOG_SIZE];
  uint16_t logLen_ = 0;
};

}  // namespace jiggle
