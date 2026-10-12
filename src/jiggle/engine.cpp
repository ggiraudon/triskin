#include "engine.h"

#include <math.h>

namespace jiggle {

namespace {

constexpr float SPEED_COUNTS_PER_S[] = {120.0f, 350.0f, 900.0f};
constexpr uint32_t MIN_STROKE_MS = 120;
constexpr int32_t MAX_REPORT = 127;  // a HID mouse report carries -127..127 per axis
constexpr float DIAGONAL = 0.70710678f;

// Unit vectors for the 8 directions, clockwise from up. Screen y grows downward.
constexpr float DIR_X[8] = {0, DIAGONAL, 1, DIAGONAL, 0, -DIAGONAL, -1, -DIAGONAL};
constexpr float DIR_Y[8] = {-1, -DIAGONAL, 0, DIAGONAL, 1, DIAGONAL, 0, -DIAGONAL};

float ease(float t) { return t * t * (3.0f - 2.0f * t); }

int32_t clampReport(int32_t v) {
  return v > MAX_REPORT ? MAX_REPORT : (v < -MAX_REPORT ? -MAX_REPORT : v);
}

}  // namespace

Engine::Engine(uint32_t seed) : s_(defaultSettings()), rng_(seed ? seed : 0x9E3779B9u) {}

uint32_t Engine::random(uint32_t lo, uint32_t hi) {
  rng_ ^= rng_ << 13;
  rng_ ^= rng_ >> 17;
  rng_ ^= rng_ << 5;
  return hi <= lo ? lo : lo + rng_ % (hi - lo + 1);
}

void Engine::configure(const Settings &s, uint32_t nowMs) {
  const bool pausedNow = s_.enabled && !s.enabled;
  s_ = s;
  if (!inSession_) {
    scheduleNext(nowMs);
  } else if (pausedNow) {
    sessionLengthMs_ = 0;  // wind the running session down after its current stroke
  }
}

void Engine::scheduleNext(uint32_t nowMs) {
  const uint32_t gapS = s_.intervalRandom ? random(s_.intervalMinS, s_.intervalMaxS) : s_.intervalS;
  nextStartMs_ = nowMs + gapS * 1000u;
  needSchedule_ = false;
}

void Engine::endSession(uint32_t nowMs) {
  inSession_ = false;
  dirX_ = 0;
  scheduleNext(nowMs);
}

bool Engine::insideBox(Point p) const {
  return fabsf(p.x) <= s_.boxW / 2.0f && fabsf(p.y) <= s_.boxH / 2.0f;
}

void Engine::startStroke(Point to, uint32_t nowMs) {
  from_ = {static_cast<float>(sentX_), static_cast<float>(sentY_)};
  to_ = {roundf(to.x), roundf(to.y)};
  const float distance = hypotf(to_.x - from_.x, to_.y - from_.y);
  const uint32_t ms = static_cast<uint32_t>(distance / SPEED_COUNTS_PER_S[s_.speed] * 1000.0f);
  strokeLengthMs_ = ms < MIN_STROKE_MS ? MIN_STROKE_MS : ms;
  strokeStartMs_ = nowMs;
}

void Engine::planStroke(uint32_t nowMs) {
  const Point here = {static_cast<float>(sentX_), static_cast<float>(sentY_)};
  const float reach = s_.amplitude;

  // Try the allowed directions in a random order.
  uint8_t order[8];
  uint8_t count = 0;
  for (uint8_t d = 0; d < 8; d++) {
    if (s_.directions & (1u << d)) {
      order[count++] = d;
    }
  }
  for (uint8_t i = count; i > 1; i--) {
    const uint8_t j = static_cast<uint8_t>(random(0, i - 1));
    const uint8_t swap = order[i - 1];
    order[i - 1] = order[j];
    order[j] = swap;
  }

  if (s_.pathMode == PATH_OUT_AND_BACK) {
    startStroke({here.x + reach * DIR_X[order[0]], here.y + reach * DIR_Y[order[0]]}, nowMs);
    return;
  }

  for (uint8_t i = 0; i < count; i++) {
    const Point to = {here.x + reach * DIR_X[order[i]], here.y + reach * DIR_Y[order[i]]};
    if (insideBox(to)) {
      startStroke(to, nowMs);
      return;
    }
  }
  // Every allowed direction leaves the box. Heading home is always permitted,
  // one ordinary stroke at a time: a single long stroke would be faster, and
  // pointer acceleration would carry the cursor well past home in pixels.
  // If already home, the box is smaller than one stroke: go as far as it allows.
  if (!atHome()) {
    const float distance = hypotf(here.x, here.y);
    const float keep = distance > reach ? 1.0f - reach / distance : 0.0f;
    startStroke({here.x * keep, here.y * keep}, nowMs);
    return;
  }
  Point to = {reach * DIR_X[order[0]], reach * DIR_Y[order[0]]};
  const float halfW = s_.boxW / 2.0f, halfH = s_.boxH / 2.0f;
  to.x = to.x > halfW ? halfW : (to.x < -halfW ? -halfW : to.x);
  to.y = to.y > halfH ? halfH : (to.y < -halfH ? -halfH : to.y);
  startStroke(to, nowMs);
}

Delta Engine::stepOut(uint32_t nowMs) {
  const bool outAndBack = s_.pathMode == PATH_OUT_AND_BACK;
  const bool returns = outAndBack || s_.returnHome;

  // Each report is the gap between where the stroke says the cursor should be
  // and where the reports so far have put it, so rounding never accumulates.
  float t = static_cast<float>(nowMs - strokeStartMs_) / strokeLengthMs_;
  if (t > 1.0f) {
    t = 1.0f;
  }
  const float k = ease(t);
  const int32_t wantX = lroundf(from_.x + (to_.x - from_.x) * k);
  const int32_t wantY = lroundf(from_.y + (to_.y - from_.y) * k);
  const Delta d = {static_cast<int8_t>(clampReport(wantX - sentX_)),
                   static_cast<int8_t>(clampReport(wantY - sentY_))};
  sentX_ += d.dx;
  sentY_ += d.dy;
  if (d.dx) {
    dirX_ = d.dx > 0 ? 1 : -1;
  }
  // Ticks without movement are logged too: the replay has to keep the timing.
  if (returns && logLen_ < LOG_SIZE) {
    log_[logLen_++] = d;
  }

  const bool logFull = logLen_ == LOG_SIZE;
  const bool strokeDone = t >= 1.0f && sentX_ == lroundf(to_.x) && sentY_ == lroundf(to_.y);
  if (strokeDone || logFull) {
    // A wander that has to come back spends half the session going out and
    // the other half retracing; an out-and-back turns round after every stroke.
    const uint32_t outBudget = (returns && !outAndBack) ? sessionLengthMs_ / 2 : sessionLengthMs_;
    const bool outOver = outAndBack || logFull || nowMs - sessionStartMs_ >= outBudget;
    if (!outOver) {
      planStroke(nowMs);
    } else if (returns) {
      phase_ = Phase::Back;
    } else {
      endSession(nowMs);
    }
  }
  return d;
}

Delta Engine::stepBack(uint32_t nowMs) {
  Delta d = {0, 0};
  if (logLen_ > 0) {
    const Delta out = log_[--logLen_];
    d = {static_cast<int8_t>(-out.dx), static_cast<int8_t>(-out.dy)};
    sentX_ += d.dx;
    sentY_ += d.dy;
    if (d.dx) {
      dirX_ = d.dx > 0 ? 1 : -1;
    }
  }
  if (logLen_ == 0) {
    const bool more =
        s_.pathMode == PATH_OUT_AND_BACK && nowMs - sessionStartMs_ < sessionLengthMs_;
    if (more) {
      phase_ = Phase::Out;
      planStroke(nowMs);
    } else {
      endSession(nowMs);
    }
  }
  return d;
}

Delta Engine::tick(uint32_t nowMs, bool hostActive) {
  hostActive_ = hostActive;
  if (!hostActive) {
    // Whatever was sent is lost to us; start afresh when a host returns.
    inSession_ = false;
    sentX_ = sentY_ = 0;
    logLen_ = 0;
    dirX_ = 0;
    needSchedule_ = true;
    return {0, 0};
  }

  if (!inSession_) {
    if (needSchedule_) {
      scheduleNext(nowMs);
    }
    const bool due = s_.enabled && static_cast<int32_t>(nowMs - nextStartMs_) >= 0;
    if (!trigger_ && !due) {
      return {0, 0};
    }
    trigger_ = false;
    inSession_ = true;
    phase_ = Phase::Out;
    logLen_ = 0;
    sessionStartMs_ = nowMs;
    sessionLengthMs_ =
        100u * (s_.durationRandom ? random(s_.durationMinDs, s_.durationMaxDs) : s_.durationDs);
    planStroke(nowMs);
  }

  return phase_ == Phase::Out ? stepOut(nowMs) : stepBack(nowMs);
}

Status Engine::status(uint32_t nowMs) const {
  Status st{};
  st.x = static_cast<int16_t>(sentX_);
  st.y = static_cast<int16_t>(sentY_);
  if (!hostActive_) {
    st.state = State::NoHost;
  } else if (inSession_) {
    st.state = State::Moving;
    st.dirX = dirX_;
  } else if (!s_.enabled) {
    st.state = State::Paused;
  } else {
    st.state = State::Waiting;
    const int32_t left = static_cast<int32_t>(nextStartMs_ - nowMs);
    st.msToNext = left > 0 ? static_cast<uint32_t>(left) : 0;
  }
  return st;
}

}  // namespace jiggle
