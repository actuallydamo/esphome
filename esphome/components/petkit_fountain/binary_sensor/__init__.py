import esphome.codegen as cg
from esphome.components import binary_sensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_MOISTURE,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_PROBLEM,
    ENTITY_CATEGORY_CONFIG,
    ENTITY_CATEGORY_DIAGNOSTIC,
)

from .. import (
    PETKIT_FOUNTAIN_CHILD_SCHEMA,
    petkit_fountain_ns,
    register_petkit_fountain_child,
)

ICON_ALERT = "mdi:alert"
ICON_LED_ON = "mdi:led-on"
ICON_WATER_ALERT = "mdi:water-alert"
ICON_WATER_CHECK = "mdi:water-check"

DEPENDENCIES = ["petkit_fountain"]

CONF_POWER_STATUS = "power_status"
CONF_WATER_MISSING = "water_missing"
CONF_FILTER_WARNING = "filter_warning"
CONF_BREAKDOWN = "breakdown"
CONF_LED_STATUS = "led_status"

PetkitFountainBinarySensor = petkit_fountain_ns.class_(
    "PetkitFountainBinarySensor", cg.Component
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(PetkitFountainBinarySensor),
            cv.Optional(CONF_POWER_STATUS): binary_sensor.binary_sensor_schema(
                PetkitFountainBinarySensor,
                device_class=DEVICE_CLASS_POWER,
            ),
            cv.Optional(CONF_WATER_MISSING): binary_sensor.binary_sensor_schema(
                PetkitFountainBinarySensor,
                device_class=DEVICE_CLASS_MOISTURE,
                icon=ICON_WATER_ALERT,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
            cv.Optional(CONF_FILTER_WARNING): binary_sensor.binary_sensor_schema(
                PetkitFountainBinarySensor,
                device_class=DEVICE_CLASS_PROBLEM,
                icon=ICON_WATER_CHECK,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
            cv.Optional(CONF_BREAKDOWN): binary_sensor.binary_sensor_schema(
                PetkitFountainBinarySensor,
                device_class=DEVICE_CLASS_PROBLEM,
                icon=ICON_ALERT,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
            cv.Optional(CONF_LED_STATUS): binary_sensor.binary_sensor_schema(
                PetkitFountainBinarySensor,
                icon=ICON_LED_ON,
                entity_category=ENTITY_CATEGORY_CONFIG,
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

    if power_status_config := config.get(CONF_POWER_STATUS):
        sens = await binary_sensor.new_binary_sensor(power_status_config)
        cg.add(var.set_power_binary_sensor(sens))

    if water_missing_config := config.get(CONF_WATER_MISSING):
        sens = await binary_sensor.new_binary_sensor(water_missing_config)
        cg.add(var.set_water_missing_binary_sensor(sens))

    if filter_warning_config := config.get(CONF_FILTER_WARNING):
        sens = await binary_sensor.new_binary_sensor(filter_warning_config)
        cg.add(var.set_filter_warning_binary_sensor(sens))

    if breakdown_config := config.get(CONF_BREAKDOWN):
        sens = await binary_sensor.new_binary_sensor(breakdown_config)
        cg.add(var.set_breakdown_binary_sensor(sens))

    if led_status_config := config.get(CONF_LED_STATUS):
        sens = await binary_sensor.new_binary_sensor(led_status_config)
        cg.add(var.set_led_binary_sensor(sens))
