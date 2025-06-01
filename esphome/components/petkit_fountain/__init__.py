import esphome.codegen as cg
from esphome.components import ble_client
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_RECEIVE_TIMEOUT

CODEOWNERS = ["@damo"]
DEPENDENCIES = ["ble_client"]
MULTI_CONF = True
CONF_PETKIT_FOUNTAIN_ID = "petkit_fountain_id"

petkit_fountain_ns = cg.esphome_ns.namespace_("petkit_fountain")
PetkitFountainHub = petkit_fountain_ns.class_(
    "PetkitFountainHub", ble_client.BLEClientNode, cg.PollingComponent
)

CONFIG_SCHEMA = (
    cv.COMPONENT_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(PetkitFountainHub),
            cv.Optional(
                CONF_RECEIVE_TIMEOUT, default="5s"
            ): cv.positive_time_period_milliseconds,
        }
    )
    .extend(ble_client.BLE_CLIENT_SCHEMA)
    .extend(cv.polling_component_schema("15s"))
)

PETKIT_FOUNTAIN_CHILD_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_PETKIT_FOUNTAIN_ID): cv.use_id(PetkitFountainHub),
    }
)


async def register_petkit_fountain_child(var, config):
    """Register a child component with the hub."""
    parent = await cg.get_variable(config[CONF_PETKIT_FOUNTAIN_ID])
    cg.add(parent.register_child(var))


async def to_code_(config):
    """Generate the code for the PetkitFountainHub component."""
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await ble_client.register_ble_node(var, config)
    if (receive_timeout := config.get(CONF_RECEIVE_TIMEOUT)) is not None:
        cg.add(var.set_status_timeout(receive_timeout))
