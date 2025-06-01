import esphome.codegen as cg
from esphome.components import switch
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    CONF_LED,
    CONF_POWER,
    DEVICE_CLASS_SWITCH,
    ENTITY_CATEGORY_CONFIG,
    ICON_POWER,
)

from .. import (
    PETKIT_FOUNTAIN_CHILD_SCHEMA,
    petkit_fountain_ns,
    register_petkit_fountain_child,
)

ICON_LED_ON = "mdi:led-on"

DEPENDENCIES = ["petkit_fountain"]

PetkitFountainSwitch = petkit_fountain_ns.class_("PetkitFountainSwitch", cg.Component)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(PetkitFountainSwitch),
            cv.Optional(CONF_POWER): switch.switch_schema(
                PetkitFountainSwitch,
                device_class=DEVICE_CLASS_SWITCH,
                icon=ICON_POWER,
                entity_category=ENTITY_CATEGORY_CONFIG,
            ),
            cv.Optional(CONF_LED): switch.switch_schema(
                PetkitFountainSwitch,
                device_class=DEVICE_CLASS_SWITCH,
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

    if power_config := config.get(CONF_POWER):
        sens = await switch.new_switch(power_config)
        cg.add(var.set_power_switch(sens))

    if led_config := config.get(CONF_LED):
        sens = await switch.new_switch(led_config)
        cg.add(var.set_led_switch(sens))
