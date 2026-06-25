#include "m5stack_coreink_d67.h"

#include "esphome/core/application.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include <cinttypes>

namespace esphome {
namespace m5stack_coreink_d67 {

static const char *const TAG = "m5stack_coreink_d67";

// Partial-refresh waveform LUT loaded into the SSD1681 via command 0x32.
//
// Source: cross-checked between the Good Display GDEY0154D67 ESP32 sample
// (A32-GDEY0154D67-FP4G-20250212) and the ESPHome waveshare_epaper driver for
// the 2.9in V2R2 panel, which shares the same SSD1681 controller family. Both
// sources agree on the waveform bytes.
//
// Structure of the 159-byte write to register 0x32:
//
//   Bytes   0-59  VS waveform table: 5 LUT entries × 12 bytes each.
//             LUT0 (White→White), LUT1 (Black→White), LUT2 (White→Black),
//             LUT3 (Black→Black), LUT4 (VCOM). Each byte packs four 2-bit
//             voltage codes: 0b00=GND, 0b01=VSH1 (positive), 0b10=VSL
//             (negative), 0b11=VSH2.
//   Bytes 60-143  TP timing table: 12 entries × 7 bytes each.
//             Bytes 0-5 per entry: phase durations TP_A..TP_F in frame
//             periods. Byte 6 per entry: RP repeat count. Only entries 0-2
//             are non-zero here; the rest are inactive.
//   Bytes 144-152  Gate scan and end-sequence options.
//   Bytes 153-158  Voltage settings written inline with the 0x32 register:
//             VCOM, VSH1, VSH2, VSL and related parameters. These values
//             are from the SSD1681 family reference and have not been
//             independently validated against the GDEY0154D67 datasheet.
//
// The active portion of the timing table drives pixels for 10 frame periods
// (0x0A in TP_A of entry 0, repeat once) then two 1-frame settling phases.
// This is a deliberately light waveform: fast but leaves minor ghosting that
// is cleared by the periodic full refresh (see full_update_every).
static const uint8_t LUT_PARTIAL_SIZE = 159;
static const uint8_t LUT_PARTIAL[LUT_PARTIAL_SIZE] = {
    // VS waveform table (bytes 0-59): 5 LUTs × 12 bytes
    // LUT0 White→White: brief VSH1 pulse then GND (pixel stays white)
    0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // LUT1 Black→White: VSL then VSL (drive toward white)
    0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // LUT2 White→Black: VSH1 then VSH1 (drive toward black)
    0x40, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // LUT3 Black→Black: GND then VSL (pixel stays black)
    0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // LUT4 VCOM: all GND
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // TP timing table (bytes 60-143): 12 entries × 7 bytes (TP_A..F, RP)
    0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,  // entry 0: 10 frames, repeat 1
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // entry 1: 1 frame settle
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // entry 2: 1 frame settle
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // entries 3-11: inactive
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // Gate scan and end-sequence options (bytes 144-152)
    0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x00, 0x00, 0x00,
    // Voltage settings written inline: VCOM, VSH1, VSH2, VSL, ... (bytes 153-158)
    0x22, 0x17, 0x41, 0xB0, 0x32, 0x36,
};

void M5StackCoreInkD67::setup() {
  this->init_internal_(BUFFER_SIZE);

  // Assert the CoreInk power latch first. GPIO12 must be held high or the
  // board loses power during setup before the firmware can re-assert it.
  if (this->power_hold_pin_ != nullptr) {
    this->power_hold_pin_->setup();
    this->power_hold_pin_->digital_write(true);
  }

  this->dc_pin_->setup();
  this->dc_pin_->digital_write(false);

  if (this->busy_pin_ != nullptr) {
    this->busy_pin_->setup();
  }

  if (this->reset_pin_ != nullptr) {
    this->reset_pin_->setup();
    this->reset_pin_->digital_write(true);
  }

  this->spi_setup();
  this->reset_();
  this->initialize_();
  this->fill(Color::WHITE);
}

void M5StackCoreInkD67::update() {
  this->do_update_();
  bool full = (this->full_update_every_ == 1 || this->at_update_ == 0);
  ESP_LOGD(TAG, "%s update", full ? "Full" : "Partial");
  if (full) {
    this->display_frame_();
  } else {
    this->display_frame_partial_();
  }
  this->at_update_ = (this->at_update_ + 1) % this->full_update_every_;
}

void M5StackCoreInkD67::dump_config() {
  ESP_LOGCONFIG(TAG, "M5Stack CoreInk D67 / GDEY0154D67");
  LOG_DISPLAY("  ", "Display", this);
  LOG_PIN("  DC Pin: ", this->dc_pin_);
  LOG_PIN("  BUSY Pin: ", this->busy_pin_);
  LOG_PIN("  Power Hold Pin: ", this->power_hold_pin_);
  LOG_PIN("  Reset Pin: ", this->reset_pin_);
  ESP_LOGCONFIG(TAG, "  Full Update Every: %" PRIu32, this->full_update_every_);
  LOG_UPDATE_INTERVAL(this);
}

float M5StackCoreInkD67::get_setup_priority() const { return setup_priority::PROCESSOR; }

void M5StackCoreInkD67::draw_absolute_pixel_internal(int x, int y, Color color) {
  if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) {
    return;
  }

  // ESPHome uses top-left origin with Y increasing downward. The SSD1681 RAM
  // counter starts at Y=199 and decrements toward Y=0, where Y=0 corresponds
  // to the physical top of the display in the CoreInk mounted orientation.
  // Inverting Y maps ESPHome's coordinate space onto the RAM layout:
  //   ESPHome (x, y=0)   → pos near buffer end   → RAM Y=0 → physical top
  //   ESPHome (x, y=199) → pos near buffer start  → RAM Y=199 → physical bottom
  const uint32_t pos = x + (HEIGHT - 1 - y) * WIDTH;

  // Each byte holds 8 pixels, MSB first. Bit 7 of byte N is pixel N*8,
  // which maps to RAM X address N*8 (SSD1681 clocks data MSB-first).
  const uint32_t index = pos / 8;
  const uint8_t bit = 0x80 >> (pos % 8);

  // SSD1681 monochrome RAM convention: bit 1 = white, bit 0 = black.
  if (color.is_on()) {
    this->buffer_[index] |= bit;
  } else {
    this->buffer_[index] &= ~bit;
  }
}

void M5StackCoreInkD67::command(uint8_t value) {
  this->dc_pin_->digital_write(false);  // DC low = command
  this->enable();
  this->write_byte(value);
  this->disable();
}

void M5StackCoreInkD67::data(uint8_t value) {
  this->dc_pin_->digital_write(true);  // DC high = data
  this->enable();
  this->write_byte(value);
  this->disable();
}

void M5StackCoreInkD67::data(const uint8_t *buffer, size_t length) {
  this->dc_pin_->digital_write(true);
  this->enable();
  this->write_array(buffer, length);
  this->disable();
}

void M5StackCoreInkD67::reset_() {
  if (this->reset_pin_ == nullptr) {
    return;
  }
  this->reset_pin_->digital_write(false);
  delay(this->reset_duration_);
  this->reset_pin_->digital_write(true);
  delay(this->reset_duration_);
}

void M5StackCoreInkD67::initialize_() {
  // Initialization follows Good Display's GDEY0154D67 ESP32 sample.

  this->command(0x12);  // SWRESET: soft-reset all registers to default
  this->wait_until_idle_();

  // Driver Output Control: set the number of gate lines and scan direction.
  // Data bytes: MUX[7:0]=0xC7 (199 = 200 lines - 1), MUX[8]=0, GD/SM/TB=0.
  // TB=0 selects the gate scan direction that places G0 at the physical top
  // of the display in the CoreInk mounting orientation.
  this->command(0x01);
  this->data(0xC7);
  this->data(0x00);
  this->data(0x00);

  // Data Entry Mode: 0x01 = X-increment, Y-decrement, address counter
  // follows X. Each byte written advances X by one pixel-byte; after each
  // complete row (25 bytes for 200 pixels), Y decrements automatically.
  // Y starts at 199 and counts down to 0, so the first bytes written go to
  // the bottom of the display and the last bytes go to the top.
  this->command(0x11);
  this->data(0x01);

  this->set_ram_area_();
  this->set_ram_counter_();

  // Border Waveform Control: 0x05 = VSS fixed level. Sets the pixel at the
  // display border to GND during full refresh, giving a clean white edge.
  this->command(0x3C);
  this->data(0x05);

  // Temperature Sensor Selection: 0x80 = use the built-in temperature sensor.
  // The SSD1681 OTP waveform table is temperature-compensated; selecting the
  // internal sensor allows the controller to self-adjust refresh timing.
  this->command(0x18);
  this->data(0x80);
}

void M5StackCoreInkD67::set_ram_area_() {
  // Set RAM X address window in bytes (not pixels): 0x00 to 0x18 = 0 to 24,
  // covering 25 bytes × 8 bits = 200 pixels per row.
  this->command(0x44);
  this->data(0x00);
  this->data(0x18);

  // Set RAM Y address window: start=199 (0xC7,0x00), end=0 (0x00,0x00).
  // With Y-decrement mode the controller writes from Y=199 down to Y=0.
  // Two bytes per value because the SSD1681 supports up to 296 gate lines.
  this->command(0x45);
  this->data(0xC7);  // Y start low byte (199)
  this->data(0x00);  // Y start high byte
  this->data(0x00);  // Y end low byte (0)
  this->data(0x00);  // Y end high byte
}

void M5StackCoreInkD67::set_ram_counter_() {
  // Set the internal address counters to the start of the window so the next
  // data write begins at X=0, Y=199 (bottom row in Y-decrement mode).
  this->command(0x4E);  // RAM X address counter
  this->data(0x00);

  this->command(0x4F);  // RAM Y address counter
  this->data(0xC7);     // 199 low byte
  this->data(0x00);     // 199 high byte
  this->wait_until_idle_();
}

void M5StackCoreInkD67::display_frame_() {
  this->set_ram_area_();
  this->set_ram_counter_();

  this->command(0x24);  // Write Black/White RAM (the image shown on screen)
  this->data(this->buffer_, BUFFER_SIZE);

  // Write the same image to the Red/Previous RAM (0x26). The SSD1681 uses
  // this as the "old frame" reference when computing partial-refresh pixel
  // transitions. Keeping it in sync after every full refresh ensures the
  // next partial refresh applies the correct waveform (WW/WB/BW/BB) to
  // each pixel rather than treating everything as unchanged.
  this->set_ram_counter_();
  this->command(0x26);
  this->data(this->buffer_, BUFFER_SIZE);

  // Display Update Control 2 (0x22) + Master Activation (0x20).
  // 0xF7: enable clock, enable analog, load temperature value, load OTP LUT,
  // run display update sequence, disable clock when done. This is the full
  // hardware-defined refresh sequence; no custom LUT is needed.
  this->command(0x22);
  this->data(0xF7);
  this->command(0x20);
  this->wait_until_idle_();
}

void M5StackCoreInkD67::display_frame_partial_() {
  // A hardware reset (if the pin is wired) puts the controller back into a
  // known state before loading the custom LUT. If no reset pin is configured
  // the call is a no-op; the controller is already awake because this driver
  // never enters deep sleep.
  this->reset_();

  // Load the custom partial-refresh waveform LUT into the controller.
  // This replaces the OTP LUT for the next display update only.
  this->command(0x32);
  this->data(LUT_PARTIAL, LUT_PARTIAL_SIZE);

  // Write Display Option Register. This 10-byte sequence is taken directly
  // from the Good Display GDEY0154D67 / WaveshareEPaper2P9InV2R2 partial
  // refresh reference; byte 5 = 0x40 enables partial display mode on the
  // SSD1681. The exact semantics of the other bytes are not documented in the
  // public SSD1681 datasheet but the sequence is consistent across sources.
  this->command(0x37);
  this->data(0x00);
  this->data(0x00);
  this->data(0x00);
  this->data(0x00);
  this->data(0x00);
  this->data(0x40);
  this->data(0x00);
  this->data(0x00);
  this->data(0x00);
  this->data(0x00);

  // Border Waveform Control: 0x80 = VCOM DC GS transition for partial mode.
  // Different from full refresh (0x05 = VSS fixed), which would cause a
  // visible border flicker on every partial update.
  this->command(0x3C);
  this->data(0x80);

  // Display Update Control 2: 0xC0 = enable clock + enable analog only,
  // with no display update. This powers up the high-voltage driver circuits
  // so they are ready before we write image data and trigger the update.
  this->command(0x22);
  this->data(0xC0);
  this->command(0x20);
  this->wait_until_idle_();

  this->set_ram_area_();
  this->set_ram_counter_();

  // Write the new image to Black/White RAM. The controller will compare this
  // against the previous frame stored in RAM 0x26 to select the waveform
  // (WW/WB/BW/BB) for each pixel when the update fires below.
  this->command(0x24);
  this->data(this->buffer_, BUFFER_SIZE);

  // Display Update Control 2: 0x0F = run display update using the loaded
  // custom LUT, then disable analog and clock when done. This is the
  // partial-refresh equivalent of the 0xF7 full-refresh sequence.
  this->command(0x22);
  this->data(0x0F);
  this->command(0x20);
  this->wait_until_idle_();

  // Sync the previous-frame RAM (0x26) to match what we just wrote to 0x24.
  // The next partial refresh will compare against this to determine which
  // pixels changed. Without this sync, 0x26 drifts out of date and the
  // controller applies no-change (WW/BB) waveforms to pixels that actually
  // did change, leaving ghosting that never clears until the next full refresh.
  this->set_ram_counter_();
  this->command(0x26);
  this->data(this->buffer_, BUFFER_SIZE);
}

bool M5StackCoreInkD67::wait_until_idle_() {
  // BUSY is active high on the GDEY0154D67: high = busy, low = ready.
  // If no busy pin is configured, fall back to a fixed 100 ms delay.
  if (this->busy_pin_ == nullptr) {
    delay(100);
    return true;
  }

  const uint32_t start = millis();
  while (this->busy_pin_->digital_read()) {
    if (millis() - start > 10000) {
      ESP_LOGW(TAG, "Timed out waiting for BUSY to clear");
      return false;
    }
    App.feed_wdt();
    delay(10);
  }
  return true;
}

}  // namespace m5stack_coreink_d67
}  // namespace esphome
