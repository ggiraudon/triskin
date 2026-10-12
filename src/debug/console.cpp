#include "console.h"

#include <Arduino.h>
#include <lvgl.h>

#include "debug/stack.h"
#include "drivers/touch.h"
#include "jiggle/task.h"
#include "settings/config.h"
#include "ui/character.h"

namespace {

void sendScreenshot() {
  lv_draw_buf_t *snap = lv_snapshot_take(lv_screen_active(), LV_COLOR_FORMAT_RGB565);
  if (!snap) {
    Serial.println("SHOT ERR");
    return;
  }
  const lv_image_header_t &h = snap->header;
  Serial.printf("SHOT %u %u %u\n", static_cast<unsigned>(h.w), static_cast<unsigned>(h.h),
                static_cast<unsigned>(h.stride));
  Serial.write(snap->data, static_cast<size_t>(h.stride) * h.h);
  Serial.flush();
  lv_draw_buf_destroy(snap);
}

void printSettings() {
  const Settings &s = config::get();
  for (size_t i = 0; i < FIELD_COUNT; i++) {
    Serial.printf("%s=%u\n", FIELDS[i].name, static_cast<unsigned>(s.*FIELDS[i].member));
  }
}

void setSetting(const String &args) {
  const int space = args.indexOf(' ');
  const Field *field = space > 0 ? findField(args.substring(0, space).c_str()) : nullptr;
  if (!field) {
    Serial.println("ERR usage: set <name> <number>");
    return;
  }
  Settings s = config::get();
  s.*field->member = static_cast<uint16_t>(constrain(args.substring(space + 1).toInt(), 0L, 65535L));
  config::set(s);
  Serial.printf("%s=%u\n", field->name, static_cast<unsigned>(config::get().*field->member));
}

void printStatus() {
  static const char *const names[] = {"nohost", "paused", "waiting", "moving"};
  const jiggle::Status st = jiggle_task::status();
  Serial.printf("state=%s next_ms=%lu x=%d y=%d min_stack=%d heap=%d frames=%lu\n",
                names[static_cast<uint8_t>(st.state)], static_cast<unsigned long>(st.msToNext), st.x,
                st.y, stack::minFree(), rp2040.getFreeHeap(),
                static_cast<unsigned long>(character::framesShown()));
}

// "tap x y", "hold x y ms" and "swipe x0 y0 x1 y1 ms".
void simulateTouch(const String &line) {
  long n[5] = {};
  const int got = sscanf(line.c_str(), "%*s %ld %ld %ld %ld %ld", &n[0], &n[1], &n[2], &n[3], &n[4]);
  if (line.startsWith("tap ") && got == 2) {
    touch::simulate(n[0], n[1], n[0], n[1], 60);
  } else if (line.startsWith("hold ") && got == 3) {
    touch::simulate(n[0], n[1], n[0], n[1], n[2]);
  } else if (line.startsWith("swipe ") && got == 5) {
    touch::simulate(n[0], n[1], n[2], n[3], n[4]);
  } else {
    Serial.println("ERR usage: tap x y | hold x y ms | swipe x0 y0 x1 y1 ms");
    return;
  }
  Serial.println("OK");
}

void run(const String &line) {
  if (line == "shot") {
    sendScreenshot();
  } else if (line == "get") {
    printSettings();
  } else if (line.startsWith("set ")) {
    setSetting(line.substring(4));
  } else if (line == "save") {
    Serial.println(config::save() ? "OK" : "ERR save failed");
  } else if (line == "defaults") {
    config::set(defaultSettings());
    Serial.println("OK");
  } else if (line == "status") {
    printStatus();
  } else if (line == "go") {
    jiggle_task::triggerNow();
    Serial.println("OK");
  } else if (line.startsWith("tap ") || line.startsWith("hold ") || line.startsWith("swipe ")) {
    simulateTouch(line);
  } else if (line == "reboot") {
    Serial.println("OK");
    Serial.flush();
    rp2040.reboot();
  } else if (line.length()) {
    Serial.printf("ERR unknown command: %s\n", line.c_str());
  }
}

}  // namespace

namespace console {

void poll() {
  static String line;
  while (Serial.available()) {
    const char c = static_cast<char>(Serial.read());
    if (c == '\n' || c == '\r') {
      run(line);
      line = "";
    } else if (line.length() < 64) {
      line += c;
    }
  }
}

}  // namespace console
