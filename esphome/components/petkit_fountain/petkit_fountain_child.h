#pragma once

#include "esphome/core/component.h"

namespace esphome {
namespace petkit_fountain {

class PetkitFountainHub;

/**
 * Base class for all Petkit Fountain components.
 * All platform-specific components (sensors, switches, etc.) should inherit from this class.
 */
class PetkitFountainChild : public Component {
 public:
  void set_parent(PetkitFountainHub *parent) { this->parent_ = parent; }
  virtual void update_state();

 protected:
  PetkitFountainHub *parent_{nullptr};
};

}  // namespace petkit_fountain
}  // namespace esphome
