import esphome.codegen as cg
from esphome.components import select
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_MODE, ENTITY_CATEGORY_CONFIG

from .. import (
    PETKIT_FOUNTAIN_CHILD_SCHEMA,
    petkit_fountain_ns,
    register_petkit_fountain_child,
)

ICON_DIRECT_WAVES = "mdi:water-waves"

DEPENDENCIES = ["petkit_fountain"]

PetkitFountainSelect = petkit_fountain_ns.class_("PetkitFountainSelect", cg.Component)

# Define mode options
MODE_OPTIONS = ["Normal", "Smart"]

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(PetkitFountainSelect),
            cv.Optional(CONF_MODE): select.select_schema(
                PetkitFountainSelect,
                icon=ICON_DIRECT_WAVES,
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

    if mode_config := config.get(CONF_MODE):
        sens = await select.new_select(mode_config, options=MODE_OPTIONS)
        cg.add(var.set_mode_select(sens))
