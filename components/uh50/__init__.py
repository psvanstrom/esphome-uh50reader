import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, sensor
from esphome.const import CONF_ID
from esphome import automation

DEPENDENCIES = ["uart", "api"]
AUTO_LOAD = ["sensor"]

uh50_ns = cg.esphome_ns.namespace("uh50")
UH50Component = uh50_ns.class_(
    "UH50Component", cg.Component, uart.UARTDevice
)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(UH50Component),
    }
).extend(uart.UART_DEVICE_SCHEMA).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
