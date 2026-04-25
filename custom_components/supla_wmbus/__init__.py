from esphome import codegen as cg
from esphome import config_validation as cv
from esphome.core.entity_helpers import register_unit_of_measurement
from esphome.components.script import Script
from esphome.components.wmbus_common.driver_loader.units import units_dict
from esphome.components.wmbus_gateway import DisplayManager
from esphome.components.wmbus_radio import RadioComponent
from esphome.const import CONF_ID
from esphome.cpp_generator import AssignmentExpression

AUTO_LOAD = ["wmbus_meter"]
DEPENDENCIES = ["supla_device"]
CODEOWNERS = ["@kubasaw"]

CONF_RADIO_ID = "radio_id"
CONF_DISPLAY_MANAGER_ID = "manager_id"
CONF_BLINKER_SCRIPT_ID = "blinker_script_id"

SuplaWMBusComponent = cg.esphome_ns.namespace("supla_wmbus_gateway").class_(
    "Component", cg.Component
)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(SuplaWMBusComponent),
        cv.GenerateID(CONF_RADIO_ID): cv.use_id(RadioComponent),
        cv.GenerateID(CONF_DISPLAY_MANAGER_ID): cv.use_id(DisplayManager),
        cv.GenerateID(CONF_BLINKER_SCRIPT_ID): cv.use_id(Script),
    }
)


async def to_code(config):
    cg.add_define("USE_WMBUS_METER_SENSOR")

    radio = await cg.get_variable(config[CONF_RADIO_ID])
    display_manager = await cg.get_variable(config[CONF_DISPLAY_MANAGER_ID])
    blinker_script = await cg.get_variable(config[CONF_BLINKER_SCRIPT_ID])

    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_radio(radio))
    cg.add(var.set_display_manager(display_manager))
    cg.add(var.set_blinker_script(blinker_script))

    cg.add_global(
        AssignmentExpression(
            cg.std_ns.namespace("unordered_map")
            .template(cg.std_string, cg.uint8)
            .operator("const"),
            "",
            "wmbus_uom_idx_map",
            [
                (lcase, register_unit_of_measurement(hcase))
                for lcase, hcase in units_dict().items()
            ],
        )
    )

    await cg.register_component(var, config)
