#pragma once

#include "esphome/core/component.h"
#include "esphome/components/select/select.h"
#include "../petkit_fountain_child.h"

namespace esphome {
namespace petkit_fountain {

class PetkitFountainSelect : public PetkitFountainChild {
 public:
  void setup() override;
  void update_state() override;
  void dump_config() override;

  void set_mode_select(select::Select *select) { this->mode_select_ = select; }

 protected:
  void on_mode_change_(const std::string &value);

  select::Select *mode_select_{nullptr};
};

}  // namespace petkit_fountain
}  // namespace esphome
