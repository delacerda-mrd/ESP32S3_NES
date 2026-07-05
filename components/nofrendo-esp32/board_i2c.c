#include "board_i2c.h"

#define I2C_SDA_GPIO 16
#define I2C_SCL_GPIO 15

static bool initialized;

void board_i2c_init(void)
{
    if (initialized) return;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_SDA_GPIO,
        .scl_io_num = I2C_SCL_GPIO,
        .sda_pullup_en = GPIO_PULLUP_DISABLE, // board has 4.7k external pull-ups
        .scl_pullup_en = GPIO_PULLUP_DISABLE,
        .master.clk_speed = 400000,
    };
    ESP_ERROR_CHECK(i2c_param_config(BOARD_I2C_PORT, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(BOARD_I2C_PORT, conf.mode, 0, 0, 0));
    initialized = true;
}
