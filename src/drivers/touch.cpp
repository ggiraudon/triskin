#include "touch.h"

#include <Arduino.h>
#include <Wire.h>

#include "board/pins.h"

namespace {

constexpr uint8_t REG_FINGER_NUM = 0x02;  // followed by XposH, XposL, YposH, YposL
constexpr uint8_t REG_IRQ_CTL = 0xFA;
constexpr uint8_t REG_DIS_AUTO_SLEEP = 0xFE;

bool writeReg(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(i2c_addr::TOUCH);
  Wire.write(reg);
  Wire.write(value);
  return Wire.endTransmission() == 0;
}

bool readRegs(uint8_t reg, uint8_t *out, uint8_t len) {
  Wire.beginTransmission(i2c_addr::TOUCH);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) {
    return false;
  }
  if (Wire.requestFrom(i2c_addr::TOUCH, len) != len) {
    return false;
  }
  for (uint8_t i = 0; i < len; i++) {
    out[i] = static_cast<uint8_t>(Wire.read());
  }
  return true;
}

struct {
  bool active;
  int16_t x0, y0, x1, y1;
  uint32_t startMs, lengthMs;
} sim = {};

// LVGL rotates pointer coordinates itself, so everything handed to it is in
// the panel's native orientation.
lv_point_t toPanel(int32_t x, int32_t y) {
  if (lcd::FLIPPED) {
    return {lcd::WIDTH - 1 - x, lcd::HEIGHT - 1 - y};
  }
  return {x, y};
}

void readCb(lv_indev_t *, lv_indev_data_t *data) {
  static lv_point_t last = {0, 0};
  if (sim.active) {
    const uint32_t elapsed = millis() - sim.startMs;
    const float t = elapsed >= sim.lengthMs ? 1.0f : static_cast<float>(elapsed) / sim.lengthMs;
    last = toPanel(lroundf(sim.x0 + (sim.x1 - sim.x0) * t), lroundf(sim.y0 + (sim.y1 - sim.y0) * t));
    sim.active = elapsed < sim.lengthMs;
    data->state = sim.active ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
    data->point = last;
    return;
  }
  uint8_t r[5];
  if (readRegs(REG_FINGER_NUM, r, sizeof(r)) && r[0] > 0) {
    last.x = min<int32_t>(((r[1] & 0x0F) << 8) | r[2], lcd::WIDTH - 1);
    last.y = min<int32_t>(((r[3] & 0x0F) << 8) | r[4], lcd::HEIGHT - 1);
    data->state = LV_INDEV_STATE_PRESSED;
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
  data->point = last;  // LVGL wants the last known position on release
}

}  // namespace

namespace touch {

lv_indev_t *begin() {
  Wire.setSDA(pins::I2C_SDA);
  Wire.setSCL(pins::I2C_SCL);
  Wire.begin();
  Wire.setClock(400000);

  // Report continuously while touched, and never auto-sleep: an idle CST816D
  // otherwise stops answering I2C until the next touch wakes it.
  if (!writeReg(REG_IRQ_CTL, 0x60) || !writeReg(REG_DIS_AUTO_SLEEP, 0x01)) {
    return nullptr;
  }

  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, readCb);
  return indev;
}

void simulate(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint32_t ms) {
  sim = {true, x0, y0, x1, y1, millis(), ms};
}

}  // namespace touch
