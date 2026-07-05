#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "board_i2c.h"
#include "touch_controller.h"

#define CTP_RST_GPIO 18
#define CTP_INT_GPIO 17
#define FT6336G_ADDR 0x38

// Active-low bit positions, matching the ev[] table in video_audio.c.
#define BIT_SELECT 0
#define BIT_START  3
#define BIT_UP     4
#define BIT_RIGHT  5
#define BIT_DOWN   6
#define BIT_LEFT   7
#define BIT_A      13
#define BIT_B      14

typedef struct {
    uint16_t x, y, w, h;
    int bit;
} touch_zone_t;

// Left bar (D-pad): 4 stacked 48x60 zones. Right bar (buttons): 4 stacked 16x60 zones.
static const touch_zone_t zones[] = {
    { 0,   0, 48, 60, BIT_UP },
    { 0,  60, 48, 60, BIT_LEFT },
    { 0, 120, 48, 60, BIT_RIGHT },
    { 0, 180, 48, 60, BIT_DOWN },
    { 304,   0, 16, 60, BIT_A },
    { 304,  60, 16, 60, BIT_B },
    { 304, 120, 16, 60, BIT_START },
    { 304, 180, 16, 60, BIT_SELECT },
};

void touchInit(void)
{
    gpio_config_t rst_conf = {
        .pin_bit_mask = 1ULL << CTP_RST_GPIO,
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&rst_conf);
    gpio_config_t int_conf = {
        .pin_bit_mask = 1ULL << CTP_INT_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&int_conf);

    board_i2c_init();

    gpio_set_level(CTP_RST_GPIO, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(CTP_RST_GPIO, 1);
    vTaskDelay(pdMS_TO_TICKS(300));
}

// Hit-tests one touch point against the zone table, clearing the matching bit.
// Left-bar/right-bar slop: any touch with x<56 counts as left bar, x>296 as right bar,
// then bucketed by y/60.
static void apply_touch(uint16_t px, uint16_t py, int *word)
{
    int zone_idx = -1;
    if (px < 56) {
        zone_idx = py / 60;
        if (zone_idx > 3) zone_idx = 3;
    } else if (px > 296) {
        zone_idx = 4 + (py / 60);
        if (zone_idx > 7) zone_idx = 7;
    }
    if (zone_idx < 0) return;
    *word &= ~(1 << zones[zone_idx].bit);
}

int touchReadInput(void)
{
    int word = 0xFFFF;

    if (gpio_get_level(CTP_INT_GPIO)) return word; // no touch pending

    uint8_t reg = 0x00;
    uint8_t buf[13];
    esp_err_t err = i2c_master_write_read_device(BOARD_I2C_PORT, FT6336G_ADDR, &reg, 1, buf, sizeof(buf), pdMS_TO_TICKS(50));
    if (err != ESP_OK) return word;

    int touches = buf[2] & 0x0F;
    if (touches > 2) touches = 2;

    for (int t = 0; t < touches; t++) {
        int base = (t == 0) ? 3 : 9;
        uint16_t raw_px = ((buf[base] & 0x0F) << 8) | buf[base + 1];
        uint16_t raw_py = ((buf[base + 2] & 0x0F) << 8) | buf[base + 3];
        // Raw coords are physical portrait (240x320); transform to our landscape layout.
        uint16_t x = raw_py;
        uint16_t y = 239 - raw_px;
        apply_touch(x, y, &word);
    }

    return word;
}
