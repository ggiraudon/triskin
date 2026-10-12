#include "display.h"

#include <Arduino.h>
#include <SPI.h>
#include <hardware/dma.h>
#include <hardware/spi.h>

#include "board/pins.h"

namespace {

// clk_peri is 150 MHz, so this lands on 37.5 MHz; the next step up (75 MHz) is
// beyond the ST7789's 62.5 MHz write limit.
const SPISettings spiSettings(40'000'000, MSBFIRST, SPI_MODE0);

// LVGL renders the screen strip by strip into these, alternating between them:
// while one strip is on its way to the panel by DMA, the next is being drawn
// into the other buffer. A fifth of the screen per strip keeps redraws of small
// widgets to one transfer without spending much RAM.
constexpr size_t DRAW_BUF_LINES = lcd::HEIGHT / 5;
alignas(4) uint8_t drawBufA[lcd::WIDTH * DRAW_BUF_LINES * 2];
alignas(4) uint8_t drawBufB[lcd::WIDTH * DRAW_BUF_LINES * 2];

// The SPI class's own transferAsync() is not used: it learns that a transfer
// has finished from an interrupt, and when a flash write (a settings save)
// overlapped a transfer that interrupt never arrived, leaving the UI waiting
// forever. This transfer is transmit-only and its completion is read straight
// from the DMA and SPI hardware.
spi_inst_t *const lcdSpi = spi0;  // the peripheral behind the Arduino `SPI` object
int dmaChannel = -1;
bool pixelsInFlight = false;

// Blocks until the pixel transfer in progress, if any, has left the SPI bus.
void finishPixels() {
  if (!pixelsInFlight) {
    return;
  }
  dma_channel_wait_for_finish_blocking(dmaChannel);
  while (spi_is_busy(lcdSpi)) {
  }
  hw_clear_bits(&spi_get_hw(lcdSpi)->dmacr, SPI_SSPDMACR_TXDMAE_BITS);
  // Nobody read the bytes clocked in while transmitting: empty the receive
  // FIFO and clear the overrun it reported, so the next blocking transfer
  // starts clean.
  while (spi_is_readable(lcdSpi)) {
    (void)spi_get_hw(lcdSpi)->dr;
  }
  spi_get_hw(lcdSpi)->icr = SPI_SSPICR_RORIC_BITS;
  digitalWrite(pins::LCD_CS, HIGH);
  SPI.endTransaction();
  pixelsInFlight = false;
}

void send(const uint8_t *cmd, size_t cmdSize, const uint8_t *data, size_t dataSize) {
  SPI.beginTransaction(spiSettings);
  digitalWrite(pins::LCD_CS, LOW);
  digitalWrite(pins::LCD_DC, LOW);
  SPI.transfer(cmd, nullptr, cmdSize);
  digitalWrite(pins::LCD_DC, HIGH);
  if (dataSize) {
    SPI.transfer(data, nullptr, dataSize);
  }
  digitalWrite(pins::LCD_CS, HIGH);
  SPI.endTransaction();
}

void sendCmd(lv_display_t *, const uint8_t *cmd, size_t cmdSize, const uint8_t *param,
             size_t paramSize) {
  finishPixels();
  send(cmd, cmdSize, param, paramSize);
}

// Starts sending a strip and returns while it is still going out. LVGL calls
// waitForPixels() before it reuses the buffer or talks to the panel again.
void sendColor(lv_display_t *, const uint8_t *cmd, size_t cmdSize, uint8_t *pixels, size_t size) {
  finishPixels();
  // LVGL's RGB565 is little-endian in memory; the panel wants the high byte first.
  lv_draw_sw_rgb565_swap(pixels, size / 2);
  SPI.beginTransaction(spiSettings);
  digitalWrite(pins::LCD_CS, LOW);
  digitalWrite(pins::LCD_DC, LOW);
  SPI.transfer(cmd, nullptr, cmdSize);
  digitalWrite(pins::LCD_DC, HIGH);

  dma_channel_config c = dma_channel_get_default_config(dmaChannel);
  channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
  channel_config_set_read_increment(&c, true);
  channel_config_set_write_increment(&c, false);
  channel_config_set_dreq(&c, spi_get_dreq(lcdSpi, true));
  hw_set_bits(&spi_get_hw(lcdSpi)->dmacr, SPI_SSPDMACR_TXDMAE_BITS);
  dma_channel_configure(dmaChannel, &c, &spi_get_hw(lcdSpi)->dr, pixels, size, true);
  pixelsInFlight = true;
}

void waitForPixels(lv_display_t *) { finishPixels(); }

}  // namespace

namespace display {

lv_display_t *begin() {
  pinMode(pins::LCD_BL, OUTPUT);
  digitalWrite(pins::LCD_BL, LOW);
  pinMode(pins::LCD_CS, OUTPUT);
  digitalWrite(pins::LCD_CS, HIGH);
  pinMode(pins::LCD_RST, OUTPUT);

  // The panel is write-only. SPI0's default RX pin is GPIO16, which is the
  // panel's DC line here, so SPI must not claim it.
  SPI.setRX(NOPIN);
  SPI.setSCK(pins::LCD_SCK);
  SPI.setTX(pins::LCD_MOSI);
  SPI.begin();
  pinMode(pins::LCD_DC, OUTPUT);
  dmaChannel = dma_claim_unused_channel(true);

  digitalWrite(pins::LCD_RST, LOW);
  delay(20);
  digitalWrite(pins::LCD_RST, HIGH);
  delay(150);

  lv_display_t *disp = lv_st7789_create(lcd::WIDTH, lcd::HEIGHT, LV_LCD_FLAG_NONE, sendCmd, sendColor);
  lv_st7789_set_invert(disp, true);  // IPS panel
  if (lcd::FLIPPED) {
    lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_180);
  }
  lv_display_set_buffers(disp, drawBufA, drawBufB, sizeof(drawBufA), LV_DISPLAY_RENDER_MODE_PARTIAL);
  lv_display_set_flush_wait_cb(disp, waitForPixels);
  // Start a redraw soon after something changes, not up to the default 20 ms
  // later: a full-screen animation frame has little time to spare.
  lv_timer_set_period(lv_display_get_refr_timer(disp), 5);
  return disp;
}

void waitIdle() { finishPixels(); }

void setBrightness(uint8_t percent) {
  analogWrite(pins::LCD_BL, map(constrain(percent, 0, 100), 0, 100, 0, 255));
}

}  // namespace display
