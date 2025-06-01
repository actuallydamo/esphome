#pragma once

#include "esphome/core/component.h"
#include "esphome/components/button/button.h"
#include "../petkit_fountain_child.h"

namespace esphome {
namespace petkit_fountain {

class PetkitFountainButton : public PetkitFountainChild {
 public:
  void setup() override;
  void dump_config() override;

  void set_reset_filter_button(button::Button *button) { this->reset_filter_button_ = button; }

 protected:
  void on_reset_filter_press_();

  button::Button *reset_filter_button_{nullptr};
};

}  // namespace petkit_fountain
}  // namespace esphome
