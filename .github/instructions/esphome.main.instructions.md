---
applyTo: '**'
---
# GitHub Copilot Instructions for ESPHome

This document provides instructions for GitHub Copilot to help with ESPHome development, ensuring that generated code follows the project's standards and best practices.

## Project Overview

ESPHome is an open-source firmware for IoT devices that allows users to control and monitor their devices without requiring them to write code. The system uses simple YAML configuration files to define device behavior and supports a wide range of sensors, displays, and other components.

## Code Structure

### Components Architecture

ESPHome components follow a specific structure:

- **Directory Structure**:
  ```
  components/
  ├── example_component/
  │   ├── __init__.py         # Python configuration validation and code generation
  │   ├── example_component.h # C++ header file
  │   ├── example_component.cpp # C++ implementation
  ```

### ESPHome-Specific Rules

- Don't use `AUTO_LOAD` for main platform components (`sensor`, `binary_sensor`, etc.)
- Components must use provided abstractions and inherit from `Component` or `PollingComponent`
- Components should not directly access other components
- For time tracking, use `App.get_loop_component_start_time()` rather than `millis()`
- Code in `loop()`, `update()`, and `setup()` must not block:
  - Avoid `delay()` calls, especially delays longer than 10ms
  - Implement state machines for operations requiring long waits
  - Use `set_timeout` when appropriate
- All entities must be optional in configuration
- Avoid hardcoded values; use constants instead
- Use comments only to explain complexity or provide a brief summary of what a class, method, etc. is doing; avoid large blocks of commented code

### External Libraries

- Avoid external libraries when possible
- If using a library, wrap it to use ESPHome's communication abstractions
- Avoid libraries that maintain global state or variables

## Testing and CI

When suggesting test code, remember ESPHome uses:
- Unit tests in the `tests/unit_tests` directory
- Component tests in the `tests/component_tests` directory
- Full CI pipeline with clang-format and other checks

You can suggest running CI checks locally with:
```bash
script/quicklint
```
