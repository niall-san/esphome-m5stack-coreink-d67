#pragma once

#include "esphome/components/display/display_buffer.h"
#include "esphome/components/spi/spi.h"
#include "esphome/core/component.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace m5stack_coreink_d67 {

// ESPHome display driver for the M5Stack CoreInk unit containing the
// Good Display GDEY0154D67 (0154BN-D67-D2) 1.54 inch e-paper panel.
// Controller: SSD1681. Resolution: 200×200, monochrome, SPI.
class M5StackCoreInkD67 : public display::DisplayBuffer,
                           public spi::SPIDevice<spi::BIT_ORDER_MSB_FIRST, spi::CLOCK_POLARITY_LOW,
                                                 spi::CLOCK_PHASE_LEADING, spi::DATA_RATE_2MHZ> {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;
  float get_setup_priority() const override;

  void set_dc_pin(GPIOPin *dc_pin) { this->dc_pin_ = dc_pin; }
  void set_busy_pin(GPIOPin *busy_pin) { this->busy_pin_ = busy_pin; }
  void set_power_hold_pin(GPIOPin *power_hold_pin) { this->power_hold_pin_ = power_hold_pin; }
  void set_reset_pin(GPIOPin *reset_pin) { this->reset_pin_ = reset_pin; }
  void set_reset_duration(uint32_t reset_duration) { this->reset_duration_ = reset_duration; }
  void set_full_update_every(uint32_t full_update_every) { this->full_update_every_ = full_update_every; }

  // Releases the board power latch (GPIO12). Call only when about to cut power —
  // the board loses power immediately after this pin goes low.
  void release_power() {
    if (this->power_hold_pin_ != nullptr) {
      this->power_hold_pin_->digital_write(false);
    }
  }

  display::DisplayType get_display_type() override { return display::DisplayType::DISPLAY_TYPE_BINARY; }

 protected:
  static constexpr int WIDTH = 200;
  static constexpr int HEIGHT = 200;
  static constexpr int BUFFER_SIZE = WIDTH * HEIGHT / 8;

  void draw_absolute_pixel_internal(int x, int y, Color color) override;
  int get_width_internal() override { return WIDTH; }
  int get_height_internal() override { return HEIGHT; }

  void command(uint8_t value);
  void data(uint8_t value);
  void data(const uint8_t *buffer, size_t length);
  void reset_();
  void initialize_();
  void set_ram_area_();
  void set_ram_counter_();
  void display_frame_();
  void display_frame_partial_();
  bool wait_until_idle_();

  GPIOPin *dc_pin_{nullptr};
  GPIOPin *busy_pin_{nullptr};
  GPIOPin *power_hold_pin_{nullptr};
  GPIOPin *reset_pin_{nullptr};
  uint32_t reset_duration_{10};
  // Trigger a full refresh every N updates; 1 means always full. Partial
  // refresh accumulates minor ghosting over time, and a periodic full refresh
  // clears it with a clean OTP waveform cycle.
  uint32_t full_update_every_{30};
  // Counts up mod full_update_every_. Zero means "do a full refresh now."
  uint32_t at_update_{0};
};

}  // namespace m5stack_coreink_d67
}  // namespace esphome

