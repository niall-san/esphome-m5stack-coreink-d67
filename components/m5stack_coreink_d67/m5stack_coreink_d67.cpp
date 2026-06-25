#include "m5stack_coreink_d67.h"

#include "esphome/core/application.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace m5stack_coreink_d67 {

static const char *const TAG = "m5stack_coreink_d67";

void M5StackCoreInkD67::setup() {
  this->init_internal_(BUFFER_SIZE);

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
  this->display_frame_();
}

void M5StackCoreInkD67::dump_config() {
  ESP_LOGCONFIG(TAG, "M5Stack CoreInk D67 / GDEY0154D67");
  LOG_DISPLAY("  ", "Display", this);
  LOG_PIN("  DC Pin: ", this->dc_pin_);
  LOG_PIN("  BUSY Pin: ", this->busy_pin_);
  LOG_PIN("  Power Hold Pin: ", this->power_hold_pin_);
  LOG_PIN("  Reset Pin: ", this->reset_pin_);
  LOG_UPDATE_INTERVAL(this);
}

float M5StackCoreInkD67::get_setup_priority() const { return setup_priority::PROCESSOR; }

void M5StackCoreInkD67::draw_absolute_pixel_internal(int x, int y, Color color) {
  if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) {
    return;
  }

  // The CoreInk mounts this D67 panel opposite the vendor sample's native RAM
  // orientation. Keep ESPHome lambdas in normal screen coordinates.
  const int ram_x = WIDTH - 1 - x;
  const int ram_y = HEIGHT - 1 - y;
  const uint32_t pos = ram_x + ram_y * WIDTH;
  const uint32_t index = pos / 8;
  const uint8_t bit = 0x80 >> (pos % 8);

  if (color.is_on()) {
    this->buffer_[index] |= bit;  // 1 = physical white on SSD1681 monochrome RAM.
  } else {
    this->buffer_[index] &= ~bit;  // 0 = physical black.
  }
}

void M5StackCoreInkD67::command(uint8_t value) {
  this->dc_pin_->digital_write(false);
  this->enable();
  this->write_byte(value);
  this->disable();
}

void M5StackCoreInkD67::data(uint8_t value) {
  this->dc_pin_->digital_write(true);
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
  // Full-refresh setup follows Good Display's GDEY0154D67 ESP32 sample:
  // X increments across each row while Y starts at the last gate line and
  // decrements toward zero.
  this->command(0x12);  // SWRESET
  this->wait_until_idle_();

  this->command(0x01);  // Driver output control
  this->data(0xC7);     // 200 - 1
  this->data(0x00);
  this->data(0x00);

  this->command(0x11);  // Data entry mode: X increment, Y decrement
  this->data(0x01);

  this->set_ram_area_();
  this->set_ram_counter_();

  this->command(0x3C);  // Border waveform
  this->data(0x05);

  this->command(0x18);  // Read built-in temperature sensor
  this->data(0x80);
}

void M5StackCoreInkD67::set_ram_area_() {
  this->command(0x44);  // Set RAM X address start/end, in bytes
  this->data(0x00);
  this->data(0x18);  // 200 / 8 - 1

  this->command(0x45);  // Set RAM Y address start/end
  this->data(0xC7);  // 200 - 1
  this->data(0x00);
  this->data(0x00);
  this->data(0x00);
}

void M5StackCoreInkD67::set_ram_counter_() {
  this->command(0x4E);  // RAM X address counter
  this->data(0x00);

  this->command(0x4F);  // RAM Y address counter
  this->data(0xC7);  // 200 - 1
  this->data(0x00);
  this->wait_until_idle_();
}

void M5StackCoreInkD67::display_frame_() {
  this->set_ram_area_();
  this->set_ram_counter_();

  this->command(0x24);  // Write black/white image RAM
  this->data(this->buffer_, BUFFER_SIZE);

  this->command(0x22);  // Display update sequence
  this->data(0xF7);
  this->command(0x20);  // Activate display update
  this->wait_until_idle_();

  // Good Display's standalone examples enter deep sleep after updates. CoreInk
  // exposes no reset pin in our known wiring, and SSD1681 deep sleep normally
  // needs hardware reset to wake, so keep the panel awake for ESPHome refreshes.
}

bool M5StackCoreInkD67::wait_until_idle_() {
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
