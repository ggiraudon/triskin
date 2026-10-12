#include "config.h"

#include <Arduino.h>
#include <LittleFS.h>

#include "drivers/display.h"
#include "jiggle/task.h"

namespace {

// File layout: Header, then `count` uint16_t values in FIELDS order, then a
// CRC-32 of everything before it. A file from older firmware has fewer values;
// the fields it lacks keep their defaults.
constexpr char PATH[] = "/settings.bin";
constexpr uint32_t MAGIC = 0x4B535254;  // "TRSK"

struct Header {
  uint32_t magic;
  uint16_t count;
  uint16_t reserved;
};

constexpr size_t MAX_FIELDS = 64;

Settings current;
Settings stored;
bool haveStored = false;

uint32_t crc32(const uint8_t *data, size_t len) {
  uint32_t crc = 0xFFFFFFFFu;
  for (size_t i = 0; i < len; i++) {
    crc ^= data[i];
    for (int bit = 0; bit < 8; bit++) {
      crc = (crc >> 1) ^ (0xEDB88320u & (0u - (crc & 1u)));
    }
  }
  return ~crc;
}

bool same(const Settings &a, const Settings &b) {
  for (size_t i = 0; i < FIELD_COUNT; i++) {
    if (a.*FIELDS[i].member != b.*FIELDS[i].member) {
      return false;
    }
  }
  return true;
}

bool load(Settings &out) {
  File f = LittleFS.open(PATH, "r");
  if (!f) {
    return false;
  }
  uint8_t buf[sizeof(Header) + MAX_FIELDS * sizeof(uint16_t) + sizeof(uint32_t)];
  const size_t len = f.read(buf, sizeof(buf));
  f.close();

  Header h;
  if (len < sizeof(h) + sizeof(uint32_t)) {
    return false;
  }
  memcpy(&h, buf, sizeof(h));
  const size_t body = sizeof(h) + h.count * sizeof(uint16_t);
  if (h.magic != MAGIC || h.count > MAX_FIELDS || len != body + sizeof(uint32_t)) {
    return false;
  }
  uint32_t crc;
  memcpy(&crc, buf + body, sizeof(crc));
  if (crc != crc32(buf, body)) {
    return false;
  }

  out = defaultSettings();
  for (size_t i = 0; i < h.count && i < FIELD_COUNT; i++) {
    memcpy(&(out.*FIELDS[i].member), buf + sizeof(h) + i * sizeof(uint16_t), sizeof(uint16_t));
  }
  sanitize(out);
  return true;
}

void apply() {
  jiggle_task::applySettings(current);
  display::setBrightness(static_cast<uint8_t>(current.brightness));
}

}  // namespace

namespace config {

void begin() {
  LittleFS.begin();  // formats the filesystem on first boot
  haveStored = load(stored);
  current = haveStored ? stored : defaultSettings();
  apply();
}

const Settings &get() { return current; }

void set(Settings s) {
  sanitize(s);
  current = s;
  apply();
}

bool save() {
  if (haveStored && same(current, stored)) {
    return true;
  }
  uint8_t buf[sizeof(Header) + MAX_FIELDS * sizeof(uint16_t) + sizeof(uint32_t)];
  const Header h = {MAGIC, static_cast<uint16_t>(FIELD_COUNT), 0};
  memcpy(buf, &h, sizeof(h));
  size_t len = sizeof(h);
  for (size_t i = 0; i < FIELD_COUNT; i++) {
    memcpy(buf + len, &(current.*FIELDS[i].member), sizeof(uint16_t));
    len += sizeof(uint16_t);
  }
  const uint32_t crc = crc32(buf, len);
  memcpy(buf + len, &crc, sizeof(crc));
  len += sizeof(crc);

  // A flash write halts both cores for a moment; let the screen finish first.
  display::waitIdle();
  File f = LittleFS.open(PATH, "w");
  if (!f) {
    return false;
  }
  const bool ok = f.write(buf, len) == len;
  f.close();
  if (ok) {
    stored = current;
    haveStored = true;
  }
  return ok;
}

}  // namespace config
