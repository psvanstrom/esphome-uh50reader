import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_ENERGY,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_WATER,
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_TOTAL_INCREASING,
    UNIT_CELSIUS,
    UNIT_KILOWATT,
    UNIT_HOUR,
    UNIT_EMPTY,
)
from . import uh50_ns, UH50Component

CONF_UH50_ID = "uh50_id"
CONF_CUMULATIVE_ACTIVE_IMPORT = "cumulative_active_import"
CONF_CUMULATIVE_VOLUME = "cumulative_volume"
CONF_CURRENT_POWER = "current_power"
CONF_FLOW_RATE = "flow_rate"
CONF_TEMPERATURE_FLOW = "temperature_flow"
CONF_TEMPERATURE_RETURN = "temperature_return"
CONF_TEMPERATURE_DIFF = "temperature_diff"
CONF_OPERATING_HOURS = "operating_hours"
CONF_MAX_POWER_PERIOD = "max_power_period"
CONF_MAX_FLOW_PERIOD = "max_flow_period"
CONF_MAX_TEMP_FLOW_PERIOD = "max_temp_flow_period"
CONF_MAX_TEMP_RETURN_PERIOD = "max_temp_return_period"
CONF_PREV_MONTH_ENERGY = "prev_month_energy"
CONF_PREV_MONTH_VOLUME = "prev_month_volume"
CONF_ERROR_CODE = "error_code"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_UH50_ID): cv.use_id(UH50Component),
        cv.Optional(CONF_CUMULATIVE_ACTIVE_IMPORT): sensor.sensor_schema(
            unit_of_measurement="MWh",
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_ENERGY,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ),
        cv.Optional(CONF_CUMULATIVE_VOLUME): sensor.sensor_schema(
            unit_of_measurement="m³",
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_WATER,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ),
        cv.Optional(CONF_CURRENT_POWER): sensor.sensor_schema(
            unit_of_measurement=UNIT_KILOWATT,
            accuracy_decimals=2,
            device_class=DEVICE_CLASS_POWER,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_FLOW_RATE): sensor.sensor_schema(
            unit_of_measurement="m³/h",
            accuracy_decimals=3,
            state_class=STATE_CLASS_MEASUREMENT,
            icon="mdi:water-pump",
        ),
        cv.Optional(CONF_TEMPERATURE_FLOW): sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=2,
            device_class=DEVICE_CLASS_TEMPERATURE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_TEMPERATURE_RETURN): sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=2,
            device_class=DEVICE_CLASS_TEMPERATURE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_TEMPERATURE_DIFF): sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=2,
            device_class=DEVICE_CLASS_TEMPERATURE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_OPERATING_HOURS): sensor.sensor_schema(
            unit_of_measurement=UNIT_HOUR,
            accuracy_decimals=0,
            state_class=STATE_CLASS_TOTAL_INCREASING,
            icon="mdi:clock-outline",
        ),
        cv.Optional(CONF_MAX_POWER_PERIOD): sensor.sensor_schema(
            unit_of_measurement=UNIT_KILOWATT,
            accuracy_decimals=2,
            icon="mdi:flash-alert",
        ),
        cv.Optional(CONF_MAX_FLOW_PERIOD): sensor.sensor_schema(
            unit_of_measurement="m³/h",
            accuracy_decimals=3,
            icon="mdi:chart-line-variant",
        ),
        cv.Optional(CONF_MAX_TEMP_FLOW_PERIOD): sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=2,
            icon="mdi:thermometer-chevron-up",
        ),
        cv.Optional(CONF_MAX_TEMP_RETURN_PERIOD): sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=2,
            icon="mdi:thermometer-chevron-down",
        ),
        cv.Optional(CONF_PREV_MONTH_ENERGY): sensor.sensor_schema(
            unit_of_measurement="MWh",
            accuracy_decimals=3,
            icon="mdi:calendar-check",
        ),
        cv.Optional(CONF_PREV_MONTH_VOLUME): sensor.sensor_schema(
            unit_of_measurement="m³",
            accuracy_decimals=3,
            icon="mdi:water-check",
        ),
        cv.Optional(CONF_ERROR_CODE): sensor.sensor_schema(
            unit_of_measurement=UNIT_EMPTY,
            accuracy_decimals=0,
            icon="mdi:alert-circle",
        ),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_UH50_ID])

    if CONF_CUMULATIVE_ACTIVE_IMPORT in config:
        s = await sensor.new_sensor(config[CONF_CUMULATIVE_ACTIVE_IMPORT])
        cg.add(hub.set_cumulative_active_import_sensor(s))
    if CONF_CUMULATIVE_VOLUME in config:
        s = await sensor.new_sensor(config[CONF_CUMULATIVE_VOLUME])
        cg.add(hub.set_cumulative_volume_sensor(s))
    if CONF_CURRENT_POWER in config:
        s = await sensor.new_sensor(config[CONF_CURRENT_POWER])
        cg.add(hub.set_current_power_sensor(s))
    if CONF_FLOW_RATE in config:
        s = await sensor.new_sensor(config[CONF_FLOW_RATE])
        cg.add(hub.set_flow_rate_sensor(s))
    if CONF_TEMPERATURE_FLOW in config:
        s = await sensor.new_sensor(config[CONF_TEMPERATURE_FLOW])
        cg.add(hub.set_temperature_flow_sensor(s))
    if CONF_TEMPERATURE_RETURN in config:
        s = await sensor.new_sensor(config[CONF_TEMPERATURE_RETURN])
        cg.add(hub.set_temperature_return_sensor(s))
    if CONF_TEMPERATURE_DIFF in config:
        s = await sensor.new_sensor(config[CONF_TEMPERATURE_DIFF])
        cg.add(hub.set_temperature_diff_sensor(s))
    if CONF_OPERATING_HOURS in config:
        s = await sensor.new_sensor(config[CONF_OPERATING_HOURS])
        cg.add(hub.set_operating_hours_sensor(s))
    if CONF_MAX_POWER_PERIOD in config:
        s = await sensor.new_sensor(config[CONF_MAX_POWER_PERIOD])
        cg.add(hub.set_max_power_period_sensor(s))
    if CONF_MAX_FLOW_PERIOD in config:
        s = await sensor.new_sensor(config[CONF_MAX_FLOW_PERIOD])
        cg.add(hub.set_max_flow_period_sensor(s))
    if CONF_MAX_TEMP_FLOW_PERIOD in config:
        s = await sensor.new_sensor(config[CONF_MAX_TEMP_FLOW_PERIOD])
        cg.add(hub.set_max_temp_flow_period_sensor(s))
    if CONF_MAX_TEMP_RETURN_PERIOD in config:
        s = await sensor.new_sensor(config[CONF_MAX_TEMP_RETURN_PERIOD])
        cg.add(hub.set_max_temp_return_period_sensor(s))
    if CONF_PREV_MONTH_ENERGY in config:
        s = await sensor.new_sensor(config[CONF_PREV_MONTH_ENERGY])
        cg.add(hub.set_prev_month_energy_sensor(s))
    if CONF_PREV_MONTH_VOLUME in config:
        s = await sensor.new_sensor(config[CONF_PREV_MONTH_VOLUME])
        cg.add(hub.set_prev_month_volume_sensor(s))
    if CONF_ERROR_CODE in config:
        s = await sensor.new_sensor(config[CONF_ERROR_CODE])
        cg.add(hub.set_error_code_sensor(s))
