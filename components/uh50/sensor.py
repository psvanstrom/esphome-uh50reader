import inspect

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.components import button, sensor, uart
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_ENERGY,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_VOLUME,
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_TOTAL_INCREASING,
)

CONF_UART_OUT_ID = "uart_out_id"
CONF_CUMULATIVE_ENERGY = "cumulative_energy"
CONF_CUMULATIVE_VOLUME = "cumulative_volume"
CONF_CURRENT_POWER = "current_power"
CONF_FLOW_RATE = "flow_rate"
CONF_FLOW_TEMP = "flow_temp"
CONF_RETURN_TEMP = "return_temp"
CONF_DIFF_TEMP = "diff_temp"
CONF_READ_BUTTON = "read_button"

DEPENDENCIES = ["uart"]
AUTO_LOAD = ["sensor", "button"]

uh50_ns = cg.esphome_ns.namespace("uh50")
UH50Reader = uh50_ns.class_("UH50Reader", cg.PollingComponent, uart.UARTDevice)
UH50ReadAction = uh50_ns.class_("UH50ReadAction", automation.Action)
UH50ReadButton = uh50_ns.class_("UH50ReadButton", button.Button)

ENERGY_SCHEMA = sensor.sensor_schema(
    accuracy_decimals=3,
    state_class=STATE_CLASS_TOTAL_INCREASING,
    device_class=DEVICE_CLASS_ENERGY,
    unit_of_measurement="MWh",
)
VOLUME_SCHEMA = sensor.sensor_schema(
    accuracy_decimals=3,
    state_class=STATE_CLASS_TOTAL_INCREASING,
    device_class=DEVICE_CLASS_VOLUME,
    unit_of_measurement="m³",
)
POWER_SCHEMA = sensor.sensor_schema(
    accuracy_decimals=2,
    state_class=STATE_CLASS_MEASUREMENT,
    device_class=DEVICE_CLASS_POWER,
    unit_of_measurement="kW",
)
FLOW_RATE_SCHEMA = sensor.sensor_schema(
    accuracy_decimals=3,
    state_class=STATE_CLASS_MEASUREMENT,
    unit_of_measurement="m³/h",
)
TEMP_SCHEMA = sensor.sensor_schema(
    accuracy_decimals=2,
    state_class=STATE_CLASS_MEASUREMENT,
    device_class=DEVICE_CLASS_TEMPERATURE,
    unit_of_measurement="°C",
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(UH50Reader),
            cv.Required(CONF_UART_OUT_ID): cv.use_id(uart.UARTComponent),
            cv.Optional(CONF_READ_BUTTON): button.button_schema(
                UH50ReadButton,
                icon="mdi:meter-electric",
            ),
            cv.Optional(CONF_CUMULATIVE_ENERGY): ENERGY_SCHEMA,
            cv.Optional(CONF_CUMULATIVE_VOLUME): VOLUME_SCHEMA,
            cv.Optional(CONF_CURRENT_POWER): POWER_SCHEMA,
            cv.Optional(CONF_FLOW_RATE): FLOW_RATE_SCHEMA,
            cv.Optional(CONF_FLOW_TEMP): TEMP_SCHEMA,
            cv.Optional(CONF_RETURN_TEMP): TEMP_SCHEMA,
            cv.Optional(CONF_DIFF_TEMP): TEMP_SCHEMA,
        }
    )
    .extend(cv.polling_component_schema("30min"))
    .extend(uart.UART_DEVICE_SCHEMA)
)

UH50_READ_ACTION_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.use_id(UH50Reader),
    }
)


# play() only kicks off the async read via the component's loop(); it does not
# defer play_next_(), so the action itself completes before returning ->
# synchronous=True. Newer ESPHome versions warn when this is unset; older ones
# don't accept the argument, so only pass it when supported.
_READ_ACTION_KWARGS = {}
if "synchronous" in inspect.signature(automation.register_action).parameters:
    _READ_ACTION_KWARGS["synchronous"] = True


@automation.register_action(
    "uh50.read",
    UH50ReadAction,
    UH50_READ_ACTION_SCHEMA,
    **_READ_ACTION_KWARGS,
)
async def uh50_read_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    uart_out = await cg.get_variable(config[CONF_UART_OUT_ID])
    cg.add(var.set_uart_out(uart_out))

    if read_button_config := config.get(CONF_READ_BUTTON):
        btn = await button.new_button(read_button_config)
        await cg.register_parented(btn, config[CONF_ID])

    if CONF_CUMULATIVE_ENERGY in config:
        sens = await sensor.new_sensor(config[CONF_CUMULATIVE_ENERGY])
        cg.add(var.set_cumulative_energy_sensor(sens))
    if CONF_CUMULATIVE_VOLUME in config:
        sens = await sensor.new_sensor(config[CONF_CUMULATIVE_VOLUME])
        cg.add(var.set_cumulative_volume_sensor(sens))
    if CONF_CURRENT_POWER in config:
        sens = await sensor.new_sensor(config[CONF_CURRENT_POWER])
        cg.add(var.set_current_power_sensor(sens))
    if CONF_FLOW_RATE in config:
        sens = await sensor.new_sensor(config[CONF_FLOW_RATE])
        cg.add(var.set_flow_rate_sensor(sens))
    if CONF_FLOW_TEMP in config:
        sens = await sensor.new_sensor(config[CONF_FLOW_TEMP])
        cg.add(var.set_flow_temp_sensor(sens))
    if CONF_RETURN_TEMP in config:
        sens = await sensor.new_sensor(config[CONF_RETURN_TEMP])
        cg.add(var.set_return_temp_sensor(sens))
    if CONF_DIFF_TEMP in config:
        sens = await sensor.new_sensor(config[CONF_DIFF_TEMP])
        cg.add(var.set_diff_temp_sensor(sens))
