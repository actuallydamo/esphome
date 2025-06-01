#include "petkit_fountain_sensor.h"
#include "../petkit_fountain_hub.h"
#include "esphome/core/log.h"

namespace esphome {
namespace petkit_fountain {

static const char *const TAG = "petkit_fountain.sensor";

void PetkitFountainSensor::update_state() {
  if (this->parent_ == nullptr || !this->parent_->is_ready())
    return;

  PetkitFountainHub *hub = (PetkitFountainHub *) this->parent_;

  if (this->battery_percentage_sensor_ != nullptr) {
    this->battery_percentage_sensor_->publish_state(hub->get_battery_percentage());
  }

  if (this->battery_voltage_sensor_ != nullptr) {
    this->battery_voltage_sensor_->publish_state(hub->get_battery_voltage());
  }

  if (this->filter_percentage_sensor_ != nullptr) {
    this->filter_percentage_sensor_->publish_state(hub->get_filter_percentage() * 100);
  }

  if (this->pump_runtime_sensor_ != nullptr) {
    // Convert from seconds to hours for better readability
    this->pump_runtime_sensor_->publish_state(hub->get_pump_runtime() / 3600.0f);
  }
}

void PetkitFountainSensor::dump_config() {
  ESP_LOGCONFIG(TAG, "Petkit Fountain Sensors:");
  LOG_SENSOR("  ", "Battery Percentage", this->battery_percentage_sensor_);
  LOG_SENSOR("  ", "Battery Voltage", this->battery_voltage_sensor_);
  LOG_SENSOR("  ", "Filter Percentage", this->filter_percentage_sensor_);
  LOG_SENSOR("  ", "Pump Runtime", this->pump_runtime_sensor_);
}

}  // namespace petkit_fountain
}  // namespace esphome
