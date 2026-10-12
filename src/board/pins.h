// Pin map for the Waveshare RP2350-Touch-LCD-2, read from the board schematic.
#pragma once

#include <stdint.h>

namespace pins {

// ST7789T3 on SPI0. Write-only: the panel has no MISO and TE is not connected.
constexpr uint8_t LCD_BL = 15;  // active high through an NPN, PWM-dimmable
constexpr uint8_t LCD_DC = 16;
constexpr uint8_t LCD_CS = 17;
constexpr uint8_t LCD_SCK = 18;
constexpr uint8_t LCD_MOSI = 19;
constexpr uint8_t LCD_RST = 20;  // also resets the touch controller

// I2C0, shared by the CST816D touch controller and the QMI8658 IMU.
constexpr uint8_t I2C_SDA = 12;
constexpr uint8_t I2C_SCL = 13;
constexpr uint8_t TOUCH_INT = 29;  // active low
constexpr uint8_t IMU_INT1 = 14;

// microSD on SPI1 (unused).
constexpr uint8_t SD_MISO = 24;
constexpr uint8_t SD_CS = 25;
constexpr uint8_t SD_SCK = 26;
constexpr uint8_t SD_MOSI = 27;

// Battery voltage through a 200k/100k divider: Vbat = 3 * Vadc.
constexpr uint8_t BAT_ADC = 28;

}  // namespace pins

namespace i2c_addr {
constexpr uint8_t TOUCH = 0x15;
constexpr uint8_t IMU = 0x6B;
}  // namespace i2c_addr

namespace lcd {
constexpr uint16_t WIDTH = 240;
constexpr uint16_t HEIGHT = 320;
// True rotates the picture (and touch) by 180 degrees. Needed to read upright
// with the USB-C connector at the top.
constexpr bool FLIPPED = true;
}  // namespace lcd
