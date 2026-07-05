// Shared I2C bus (touch + audio codec) for the ES3C28P board.
// Uses the legacy driver/i2c.h API because the available es8311 component (v1.0.0)
// only accepts a raw i2c_port_t, not the newer i2c_master_bus_handle_t.
#ifndef _BOARD_I2C_H_
#define _BOARD_I2C_H_

#include "driver/i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BOARD_I2C_PORT I2C_NUM_0

// Idempotent; installs the driver on first call (port 0, SDA=16, SCL=15, 400kHz).
void board_i2c_init(void);

#ifdef __cplusplus
}
#endif

#endif
