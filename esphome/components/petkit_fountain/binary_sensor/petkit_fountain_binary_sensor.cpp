#include "petkit_fountain_binary_sensor.h"
#include "../petkit_fountain_hub.h"
#include "esphome/core/log.h"

namespace esphome {
namespace petkit_fountain {

static const char *const TAG = "petkit_fountain.binary_sensor";

void PetkitFountainBinarySensor::update_state() {
  if (this->parent_ == nullptr || !this->parent_->is_ready())
    return;

  PetkitFountainHub *hub = (PetkitFountainHub *) this->parent_;

  if (this->power_binary_sensor_ != nullptr) {
    this->power_binary_sensor_->publish_state(hub->get_power_status() != 0);
  }

  if (this->water_missing_binary_sensor_ != nullptr) {
    this->water_missing_binary_sensor_->publish_state(hub->get_warning_water_missing() != 0);
  }

  if (this->filter_warning_binary_sensor_ != nullptr) {
    this->filter_warning_binary_sensor_->publish_state(hub->get_warning_filter() != 0);
  }

  if (this->breakdown_binary_sensor_ != nullptr) {
    this->breakdown_binary_sensor_->publish_state(hub->get_warning_breakdown() != 0);
  }

  if (this->led_binary_sensor_ != nullptr) {
    this->led_binary_sensor_->publish_state(hub->get_led_switch() != 0);
  }
}

void PetkitFountainBinarySensor::dump_config() {
  ESP_LOGCONFIG(TAG, "Petkit Fountain Binary Sensors:");
  LOG_BINARY_SENSOR("  ", "Power Status", this->power_binary_sensor_);
  LOG_BINARY_SENSOR("  ", "Water Missing", this->water_missing_binary_sensor_);
  LOG_BINARY_SENSOR("  ", "Filter Warning", this->filter_warning_binary_sensor_);
  LOG_BINARY_SENSOR("  ", "Breakdown", this->breakdown_binary_sensor_);
  LOG_BINARY_SENSOR("  ", "LED Status", this->led_binary_sensor_);
}

}  // namespace petkit_fountain
}  // namespace esphome
