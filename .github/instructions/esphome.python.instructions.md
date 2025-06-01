---
applyTo: '**/*.py'
---
# ESPHome Python Coding Instructions

## Python Module Structure

Python modules in ESPHome (typically `__init__.py` files) follow a consistent structure:

- Defines `CONFIG_SCHEMA` for YAML validation
- Implements `to_code()` method for C++ code generation
- May define other component metadata like `DEPENDENCIES`, `CODEOWNERS`, etc.

## Python Code Style

For Python code in ESPHome:

- Configuration keys should be defined as constants in the form `CONF_XYZ`
- Use Python's walrus operator for optional config (except for boolean values):
  ```python
  if sensor_config := config.get(CONF_SENSOR):
      # do something
  ```

## Sample Python Module

### Python (`__init__.py`):

```python
import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import CONF_ID

CONF_FOO = "foo"
CONF_BAR = "bar"
CONF_BAZ = "baz"

example_component_ns = cg.esphome_ns.namespace("example_component")
ExampleComponent = example_component_ns.class_("ExampleComponent", cg.Component)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(ExampleComponent),
    cv.Required(CONF_FOO): cv.boolean,
    cv.Optional(CONF_BAR): cv.string,
    cv.Optional(CONF_BAZ): cv.int_range(0, 255),
})

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    cg.add(var.set_foo(config[CONF_FOO]))
    if bar := config.get(CONF_BAR):
        cg.add(var.set_bar(bar))
    if baz := config.get(CONF_BAZ):
        cg.add(var.set_baz(baz))
```

## Common Code Generation Patterns

When generating Python code for ESPHome components:

- Always use `cv.Schema` for validating configuration
- Use `cv.Required` for mandatory fields and `cv.Optional` for optional fields
- Register components with `await cg.register_component(var, config)`
- Generate code that sets properties on the C++ object using `cg.add(var.set_property(value))`
- Use the component's namespace for all related types and constants
