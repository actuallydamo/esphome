---
applyTo: '**/*.cpp,**/*.h'
---
# ESPHome C++ Coding Instructions

## C++ Component Structure

Components in ESPHome follow a consistent structure:

- Components are in their own namespaces within the `esphome` namespace
- Typically inherit from `Component` or `PollingComponent`
- Implement required methods: `setup()`, `loop()`, `dump_config()`, `get_setup_priority()`

## C++ Code Style

ESPHome follows the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) with these modifications:

- Function, method, and variable names: `lower_snake_case`
- Class/struct/enum names: `UpperCamelCase`
- Constants: `UPPER_SNAKE_CASE`
- Fields: `protected` and `lower_snake_case_with_trailing_underscore_` (avoid `private`)
- Use descriptive variable/function names
- Prefix class members with `this->`
- Use two spaces for indentation, not tabs
- Avoid `#define`, use constants or enums instead
- Use `using type_t = int;` instead of `typedef int type_t;`
- Wrap lines at 120 characters maximum

## Sample C++ Component

### C++ Header (`example_component.h`):

```cpp
#pragma once

#include "esphome/core/component.h"

namespace esphome {
namespace example_component {

class ExampleComponent : public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  void set_foo(bool foo) { this->foo_ = foo; }
  void set_bar(std::string bar) { this->bar_ = bar; }
  void set_baz(int baz) { this->baz_ = baz; }

 protected:
  bool foo_{false};
  std::string bar_{};
  int baz_{0};
};

}  // namespace example_component
}  // namespace esphome
```

### C++ Implementation (`example_component.cpp`):

```cpp
#include "esphome/core/log.h"
#include "example_component.h"

namespace esphome {
namespace example_component {

static const char *TAG = "example_component.component";

void ExampleComponent::setup() {
  // Initialization code here
  // Do not block!
}

void ExampleComponent::loop() {
  // Code here runs every loop iteration
  // Must not block!
}

void ExampleComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "Example component");
  ESP_LOGCONFIG(TAG, "  foo = %s", TRUEFALSE(this->foo_));
  ESP_LOGCONFIG(TAG, "  bar = %s", this->bar_.c_str());
  ESP_LOGCONFIG(TAG, "  baz = %i", this->baz_);
}

}  // namespace example_component
}  // namespace esphome
```

## Common ESPHome C++ API Functions

When generating C++ code, prefer using these common ESPHome patterns and APIs:

- For logging: `ESP_LOGD(TAG, "Message")`, `ESP_LOGW(TAG, "Warning")`, `ESP_LOGE(TAG, "Error")`
- For configuration logging: `ESP_LOGCONFIG(TAG, "Config item: %s", value.c_str())`
- For component registration: `App.register_component(this)`
- For scheduling future code execution: `this->set_timeout(timeout_ms, std::bind(&Class::method, this))`
- For periodic updates: Use `PollingComponent` and implement `update()`
