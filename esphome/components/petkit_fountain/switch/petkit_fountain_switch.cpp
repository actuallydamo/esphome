#include "petkit_fountain_switch.h"
#include "../petkit_fountain_hub.h"
#include "esphome/core/log.h"

namespace esphome {
namespace petkit_fountain {

static const char *const TAG = "petkit_fountain.switch";

void PetkitFountainSwitch::setup() {
  if (this->power_switch_ != nullptr) {
    this->power_switch_->add_on_state_callback([this](bool state) { this->write_state_power_(state); });
  }

  if (this->led_switch_ != nullptr) {
    this->led_switch_->add_on_state_callback([this](bool state) { this->write_state_led_(state); });
  }
}

void PetkitFountainSwitch::update_state() {
  if (this->parent_ == nullptr || !this->parent_->is_ready())
    return;

  PetkitFountainHub *hub = (PetkitFountainHub *) this->parent_;

  if (this->power_switch_ != nullptr) {
    this->power_switch_->publish_state(hub->get_power_status() != 0);
  }

  if (this->led_switch_ != nullptr) {
    this->led_switch_->publish_state(hub->get_led_switch() != 0);
  }
}

void PetkitFountainSwitch::write_state_power_(bool state) {
  if (this->parent_ == nullptr || !this->parent_->is_initialized())
    return;

  ESP_LOGD(TAG, "Setting power state: %s", state ? "ON" : "OFF");
  PetkitFountainHub *hub = (PetkitFountainHub *) this->parent_;
  hub->set_power_status(state);
}

void PetkitFountainSwitch::write_state_led_(bool state) {
  if (this->parent_ == nullptr || !this->parent_->is_initialized())
    return;

  ESP_LOGD(TAG, "Setting LED state: %s", state ? "ON" : "OFF");
  PetkitFountainHub *hub = (PetkitFountainHub *) this->parent_;
  hub->set_led_switch(state);
}

void PetkitFountainSwitch::dump_config() {
  ESP_LOGCONFIG(TAG, "Petkit Fountain Switch:");
  LOG_SWITCH("  ", "Power", this->power_switch_);
  LOG_SWITCH("  ", "LED", this->led_switch_);
}

}  // namespace petkit_fountain
}  // namespace esphome
