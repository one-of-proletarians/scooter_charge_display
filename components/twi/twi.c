#include "twi.h"

static i2c_master_bus_handle_t bus_handle;
static i2c_master_bus_config_t bus_conf = {
    .clk_source = I2C_CLK_SRC_DEFAULT,
    .i2c_port = I2C_NUM_0,
    .sda_io_num = 20,
    .scl_io_num = 10,
    .glitch_ignore_cnt = 7,
    .flags.enable_internal_pullup = true,
};

esp_err_t twi_init()
{
    return i2c_new_master_bus(&bus_conf, &bus_handle);
}
