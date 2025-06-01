#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "../petkit_fountain_child.h"

namespace esphome {
namespace petkit_fountain {

class PetkitFountainSensor : public PetkitFountainChild {
 public:
  void update_state() override;
  void dump_config() override;

  void set_battery_percentage_sensor(sensor::Sensor *sensor) { this->battery_percentage_sensor_ = sensor; }
  void set_battery_voltage_sensor(sensor::Sensor *sensor) { this->battery_voltage_sensor_ = sensor; }
  void set_filter_percentage_sensor(sensor::Sensor *sensor) { this->filter_percentage_sensor_ = sensor; }
  void set_pump_runtime_sensor(sensor::Sensor *sensor) { this->pump_runtime_sensor_ = sensor; }

 protected:
  sensor::Sensor *battery_percentage_sensor_{nullptr};
  sensor::Sensor *battery_voltage_sensor_{nullptr};
  sensor::Sensor *filter_percentage_sensor_{nullptr};
  sensor::Sensor *pump_runtime_sensor_{nullptr};
};

}  // namespace petkit_fountain
}  // namespace esphome
