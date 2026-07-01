#pragma once
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"
#include "esphome/core/log.h"

// Reads battery voltage from the AXP192 PMIC on the M5Stack CoreInk D67.
// Only reads registers — setup() is intentionally empty so no power rail
// configuration is changed and the AXP192's existing state is left untouched.
//
// Battery voltage ADC: register 0x78 (high 8 bits) + 0x79 (low 4 bits in [7:4])
// 12-bit value, LSB = 1.1 mV.
//
// Level % uses a linear approximation: 3.0 V = 0%, 4.2 V = 100%.
// (AXP192 low-voltage cutoff is ~3.0 V, hence the lower bound.)

namespace esphome {
namespace axp192_battery {

static const char *const TAG = "axp192_battery";

class AXP192BatterySensor : public PollingComponent, public i2c::I2CDevice {
 public:
  void set_voltage_sensor(sensor::Sensor *s) { voltage_sensor_ = s; }
  void set_level_sensor(sensor::Sensor *s) { level_sensor_ = s; }

  float get_setup_priority() const override { return setup_priority::DATA; }

  void setup() override {}

  void update() override {
    uint8_t high = 0, low = 0;
    if (this->read_register(0x78, &high, 1) != i2c::ERROR_OK ||
        this->read_register(0x79, &low, 1) != i2c::ERROR_OK) {
      ESP_LOGW(TAG, "Failed to read AXP192 battery voltage");
      return;
    }
    uint16_t raw = ((uint16_t)high << 4) | (low >> 4);
    float voltage = raw * 0.0011f;
    float level = std::max(0.0f, std::min(100.0f, (voltage - 3.0f) / 1.2f * 100.0f));
    ESP_LOGD(TAG, "Battery: %.2f V  %.0f%%", voltage, level);
    if (voltage_sensor_ != nullptr)
      voltage_sensor_->publish_state(voltage);
    if (level_sensor_ != nullptr)
      level_sensor_->publish_state(level);
  }

 protected:
  sensor::Sensor *voltage_sensor_{nullptr};
  sensor::Sensor *level_sensor_{nullptr};
};

}  // namespace axp192_battery
}  // namespace esphome
