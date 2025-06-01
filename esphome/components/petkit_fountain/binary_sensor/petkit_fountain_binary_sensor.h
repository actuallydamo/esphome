#pragma once

#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "../petkit_fountain_child.h"

namespace esphome {
namespace petkit_fountain {

class PetkitFountainBinarySensor : public PetkitFountainChild {
 public:
  void update_state() override;
  void dump_config() override;

  void set_power_binary_sensor(binary_sensor::BinarySensor *sensor) { this->power_binary_sensor_ = sensor; }
  void set_water_missing_binary_sensor(binary_sensor::BinarySensor *sensor) {
    this->water_missing_binary_sensor_ = sensor;
  }
  void set_filter_warning_binary_sensor(binary_sensor::BinarySensor *sensor) {
    this->filter_warning_binary_sensor_ = sensor;
  }
  void set_breakdown_binary_sensor(binary_sensor::BinarySensor *sensor) { this->breakdown_binary_sensor_ = sensor; }
  void set_led_binary_sensor(binary_sensor::BinarySensor *sensor) { this->led_binary_sensor_ = sensor; }

 protected:
  binary_sensor::BinarySensor *power_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *water_missing_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *filter_warning_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *breakdown_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *led_binary_sensor_{nullptr};
};

}  // namespace petkit_fountain
}  // namespace esphome
