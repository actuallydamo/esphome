#include "petkit_fountain_select.h"
#include "../petkit_fountain_hub.h"
#include "../petkit_fountain_const.h"
#include "esphome/core/log.h"

namespace esphome {
namespace petkit_fountain {

static const char *const TAG = "petkit_fountain.select";

void PetkitFountainSelect::setup() {
  if (this->mode_select_ != nullptr) {
    this->mode_select_->add_on_state_callback(
        [this](const std::string &value, size_t index) { this->on_mode_change_(value); });
  }
}

void PetkitFountainSelect::update_state() {
  if (this->parent_ == nullptr || !this->parent_->is_ready())
    return;

  PetkitFountainHub *hub = (PetkitFountainHub *) this->parent_;

  if (this->mode_select_ != nullptr) {
    switch (hub->get_mode()) {
      case MODE_NORMAL:
        this->mode_select_->publish_state("Normal");
        break;
      case MODE_SMART:
        this->mode_select_->publish_state("Smart");
        break;
      default:
        ESP_LOGW(TAG, "Unknown mode value: %d", hub->get_mode());
        break;
    }
  }
}

void PetkitFountainSelect::on_mode_change_(const std::string &value) {
  if (this->parent_ == nullptr || !this->parent_->is_initialized())
    return;

  PetkitFountainHub *hub = (PetkitFountainHub *) this->parent_;

  if (value == "Normal") {
    hub->set_mode(MODE_NORMAL);
  } else if (value == "Smart") {
    hub->set_mode(MODE_SMART);
  } else {
    ESP_LOGW(TAG, "Unknown mode value: %s", value.c_str());
  }
}

void PetkitFountainSelect::dump_config() {
  ESP_LOGCONFIG(TAG, "Petkit Fountain Select:");
  LOG_SELECT("  ", "Mode", this->mode_select_);
}

}  // namespace petkit_fountain
}  // namespace esphome
