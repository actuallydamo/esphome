#pragma once

#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"
#include "../petkit_fountain_child.h"

namespace esphome {
namespace petkit_fountain {

class PetkitFountainSwitch : public PetkitFountainChild {
 public:
  void setup() override;
  void update_state() override;
  void dump_config() override;

  void set_power_switch(switch_::Switch *power_switch) { this->power_switch_ = power_switch; }
  void set_led_switch(switch_::Switch *led_switch) { this->led_switch_ = led_switch; }

 protected:
  void write_state_power_(bool state);
  void write_state_led_(bool state);

  switch_::Switch *power_switch_{nullptr};
  switch_::Switch *led_switch_{nullptr};
};

}  // namespace petkit_fountain
}  // namespace esphome
