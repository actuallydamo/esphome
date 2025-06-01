import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_BATTERY_LEVEL,
    CONF_ID,
    CONF_VOLTAGE,
    DEVICE_CLASS_BATTERY,
    DEVICE_CLASS_DURATION,
    DEVICE_CLASS_VOLTAGE,
    ENTITY_CATEGORY_DIAGNOSTIC,
    ICON_WATER_PERCENT,
    STATE_CLASS_MEASUREMENT,
    UNIT_HOUR,
    UNIT_PERCENT,
    UNIT_VOLT,
)

from .. import (
    PETKIT_FOUNTAIN_CHILD_SCHEMA,
    petkit_fountain_ns,
    register_petkit_fountain_child,
)

DEPENDENCIES = ["petkit_fountain"]

CONF_FILTER_PERCENTAGE = "filter_percentage"
CONF_PUMP_RUNTIME = "pump_runtime"

PetkitFountainSensor = petkit_fountain_ns.class_("PetkitFountainSensor", cg.Component)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(PetkitFountainSensor),
            cv.Optional(CONF_BATTERY_LEVEL): sensor.sensor_schema(
                unit_of_measurement=UNIT_PERCENT,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_BATTERY,
                state_class=STATE_CLASS_MEASUREMENT,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
            cv.Optional(CONF_VOLTAGE): sensor.sensor_schema(
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
            cv.Optional(CONF_FILTER_PERCENTAGE): sensor.sensor_schema(
                unit_of_measurement=UNIT_PERCENT,
                accuracy_decimals=0,
                icon=ICON_WATER_PERCENT,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_PUMP_RUNTIME): sensor.sensor_schema(
                unit_of_measurement=UNIT_HOUR,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_DURATION,
                state_class=STATE_CLASS_MEASUREMENT,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
        }
    )
    .extend(PETKIT_FOUNTAIN_CHILD_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code_(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await register_petkit_fountain_child(var, config)

    if battery_level_config := config.get(CONF_BATTERY_LEVEL):
        sens = await sensor.new_sensor(battery_level_config)
        cg.add(var.set_battery_percentage_sensor(sens))

    if voltage_config := config.get(CONF_VOLTAGE):
        sens = await sensor.new_sensor(voltage_config)
        cg.add(var.set_battery_voltage_sensor(sens))

    if filter_percentage_config := config.get(CONF_FILTER_PERCENTAGE):
        sens = await sensor.new_sensor(filter_percentage_config)
        cg.add(var.set_filter_percentage_sensor(sens))

    if pump_runtime_config := config.get(CONF_PUMP_RUNTIME):
        sens = await sensor.new_sensor(pump_runtime_config)
        cg.add(var.set_pump_runtime_sensor(sens))
