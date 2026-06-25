import esphome.codegen as cg
from esphome import core, pins
import esphome.config_validation as cv
from esphome.components import display, spi
from esphome.const import (
    CONF_BUSY_PIN,
    CONF_DC_PIN,
    CONF_ID,
    CONF_LAMBDA,
    CONF_RESET_DURATION,
    CONF_RESET_PIN,
)

DEPENDENCIES = ["spi"]

CONF_POWER_HOLD_PIN = "power_hold_pin"

m5stack_coreink_d67_ns = cg.esphome_ns.namespace("m5stack_coreink_d67")
M5StackCoreInkD67 = m5stack_coreink_d67_ns.class_(
    "M5StackCoreInkD67",
    cg.PollingComponent,
    spi.SPIDevice,
    display.DisplayBuffer,
)

CONFIG_SCHEMA = (
    display.FULL_DISPLAY_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(M5StackCoreInkD67),
            cv.Optional(CONF_DC_PIN, default="GPIO15"): pins.gpio_output_pin_schema,
            cv.Optional(CONF_BUSY_PIN, default="GPIO4"): pins.gpio_input_pin_schema,
            cv.Optional(CONF_POWER_HOLD_PIN, default="GPIO12"): pins.gpio_output_pin_schema,
            cv.Optional(CONF_RESET_PIN): pins.gpio_output_pin_schema,
            cv.Optional(CONF_RESET_DURATION, default="10ms"): cv.All(
                cv.positive_time_period_milliseconds,
                cv.Range(max=core.TimePeriod(milliseconds=500)),
            ),
        }
    )
    .extend(cv.polling_component_schema("60s"))
    .extend(spi.spi_device_schema())
)

FINAL_VALIDATE_SCHEMA = spi.final_validate_device_schema(
    "m5stack_coreink_d67", require_miso=False, require_mosi=True
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await display.register_display(var, config)
    await spi.register_spi_device(var, config, write_only=True)

    dc = await cg.gpio_pin_expression(config[CONF_DC_PIN])
    cg.add(var.set_dc_pin(dc))

    if CONF_BUSY_PIN in config:
        busy = await cg.gpio_pin_expression(config[CONF_BUSY_PIN])
        cg.add(var.set_busy_pin(busy))

    if CONF_POWER_HOLD_PIN in config:
        power_hold = await cg.gpio_pin_expression(config[CONF_POWER_HOLD_PIN])
        cg.add(var.set_power_hold_pin(power_hold))

    if CONF_RESET_PIN in config:
        reset = await cg.gpio_pin_expression(config[CONF_RESET_PIN])
        cg.add(var.set_reset_pin(reset))

    cg.add(var.set_reset_duration(config[CONF_RESET_DURATION]))

    if CONF_LAMBDA in config:
        lambda_ = await cg.process_lambda(
            config[CONF_LAMBDA],
            [(display.DisplayRef, "it")],
            return_type=cg.void,
        )
        cg.add(var.set_writer(lambda_))
