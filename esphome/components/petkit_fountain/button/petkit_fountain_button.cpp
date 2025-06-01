#include "petkit_fountain_button.h"
#include "../petkit_fountain_hub.h"
#include "esphome/core/log.h"

namespace esphome {
namespace petkit_fountain {

static const char *const TAG = "petkit_fountain.button";

void PetkitFountainButton::setup() {
  if (this->reset_filter_button_ != nullptr) {
    this->reset_filter_button_->add_on_press_callback([this]() { this->on_reset_filter_press_(); });
  }
}

void PetkitFountainButton::on_reset_filter_press_() {
  if (this->parent_ == nullptr || !this->parent_->is_initialized())
    return;

  ESP_LOGI(TAG, "Resetting filter");
  PetkitFountainHub *hub = (PetkitFountainHub *) this->parent_;
  hub->reset_filter();
}

void PetkitFountainButton::dump_config() {
  ESP_LOGCONFIG(TAG, "Petkit Fountain Button:");
  LOG_BUTTON("  ", "Reset Filter", this->reset_filter_button_);
}

}  // namespace petkit_fountain
}  // namespace esphome
