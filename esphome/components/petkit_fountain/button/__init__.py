import esphome.codegen as cg
from esphome.components import button
import esphome.config_validation as cv
from esphome.const import CONF_ID, ENTITY_CATEGORY_CONFIG

from .. import (
    PETKIT_FOUNTAIN_CHILD_SCHEMA,
    petkit_fountain_ns,
    register_petkit_fountain_child,
)

ICON_WATER_PUMP = "mdi:water-pump"

DEPENDENCIES = ["petkit_fountain"]

CONF_RESET_FILTER = "reset_filter"

PetkitFountainButton = petkit_fountain_ns.class_("PetkitFountainButton", cg.Component)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(PetkitFountainButton),
            cv.Optional(CONF_RESET_FILTER): button.button_schema(
                PetkitFountainButton,
                icon=ICON_WATER_PUMP,
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

    if reset_filter_config := config.get(CONF_RESET_FILTER):
        sens = await button.new_button(reset_filter_config)
        cg.add(var.set_reset_filter_button_(sens))
