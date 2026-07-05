// LCD driver for ILI9341V (QDtech ES3C28P), using esp_lcd over SPI2.
//
// RST is tied to the chip's own reset line on this board -- never toggled here.

#include <string.h>
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_heap_caps.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_io_spi.h"
#include "spi_lcd.h"

#define PIN_NUM_CS    10
#define PIN_NUM_DC    46
#define PIN_NUM_SCLK  12
#define PIN_NUM_MOSI  11
#define PIN_NUM_MISO  13
#define PIN_NUM_BCKL  45

#define LCD_SPI_HOST  SPI2_HOST
#define LCD_PCLK_HZ   (40 * 1000 * 1000)

#define LCD_WIDTH     320
#define LCD_HEIGHT    240

#define PARALLEL_LINES 16

static esp_lcd_panel_io_handle_t io_handle;
static uint16_t *line_buf[2];
static SemaphoreHandle_t buf_free_sem;

typedef struct {
    uint8_t cmd;
    uint8_t data[16];
    uint8_t len;
    uint16_t delay_ms;
} lcd_init_cmd_t;

static const lcd_init_cmd_t ili9341v_init_seq[] = {
    {0xCF, {0x00, 0xC1, 0x30}, 3, 0},
    {0xED, {0x64, 0x03, 0x12, 0x81}, 4, 0},
    {0xE8, {0x85, 0x00, 0x78}, 3, 0},
    {0xCB, {0x39, 0x2C, 0x00, 0x34, 0x02}, 5, 0},
    {0xF7, {0x20}, 1, 0},
    {0xEA, {0x00, 0x00}, 2, 0},
    {0xC0, {0x13}, 1, 0},
    {0xC1, {0x13}, 1, 0},
    {0xC5, {0x22, 0x35}, 2, 0},
    {0xC7, {0xBD}, 1, 0},
    {0x21, {0}, 0, 0},                 // Display inversion ON (required for IPS)
    {0x36, {0x68}, 1, 0},              // MADCTL: landscape, BGR
    {0xB6, {0x0A, 0xA2}, 2, 0},
    {0x3A, {0x55}, 1, 0},              // 16-bit RGB565
    {0xF6, {0x01, 0x30}, 2, 0},
    {0xB1, {0x00, 0x1B}, 2, 0},
    {0xF2, {0x00}, 1, 0},
    {0x26, {0x01}, 1, 0},
    {0xE0, {0x0F, 0x35, 0x31, 0x0B, 0x0E, 0x06, 0x49, 0xA7, 0x33, 0x07, 0x0F, 0x03, 0x0C, 0x0A, 0x00}, 15, 0},
    {0xE1, {0x00, 0x0A, 0x0F, 0x04, 0x11, 0x08, 0x36, 0x58, 0x4D, 0x07, 0x10, 0x0C, 0x32, 0x34, 0x0F}, 15, 0},
    {0x11, {0}, 0, 120},               // Exit sleep
    {0x29, {0}, 0, 0},                 // Display on
};

static bool color_trans_done(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    BaseType_t hp_task_woken = pdFALSE;
    xSemaphoreGiveFromISR(buf_free_sem, &hp_task_woken);
    return hp_task_woken == pdTRUE;
}

static void set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    uint8_t caset[4] = { x0 >> 8, x0 & 0xFF, x1 >> 8, x1 & 0xFF };
    uint8_t raset[4] = { y0 >> 8, y0 & 0xFF, y1 >> 8, y1 & 0xFF };
    esp_lcd_panel_io_tx_param(io_handle, 0x2A, caset, 4);
    esp_lcd_panel_io_tx_param(io_handle, 0x2B, raset, 4);
}

// Sends `count` pixels of a single (already byte-swapped) color, using the window
// previously set by the caller. First chunk includes the RAMWR command.
static void send_solid_pixels(uint16_t swapped_color, uint32_t count)
{
    uint32_t max_px = LCD_WIDTH * PARALLEL_LINES;
    for (int i = 0; i < 2; i++) {
        uint32_t n = count < max_px ? count : max_px;
        for (uint32_t p = 0; p < n; p++) line_buf[i][p] = swapped_color;
    }

    bool first = true;
    while (count > 0) {
        uint32_t n = count < max_px ? count : max_px;
        int idx = first ? 0 : 1;
        xSemaphoreTake(buf_free_sem, portMAX_DELAY);
        esp_lcd_panel_io_tx_color(io_handle, first ? 0x2C : -1, line_buf[idx], n * 2);
        first = false;
        count -= n;
    }
}

void ili9341_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t rgb565)
{
    uint16_t swapped = (rgb565 >> 8) | (rgb565 << 8);
    set_window(x, y, x + w - 1, y + h - 1);
    send_solid_pixels(swapped, (uint32_t)w * h);
}

void ili9341_write_frame(uint16_t xs, uint16_t ys, uint16_t width, uint16_t height, const uint8_t *data[])
{
    extern uint16_t myPalette[];

    if (data == NULL) {
        ili9341_fill_rect(xs, ys, width, height, 0);
        return;
    }

    set_window(xs, ys, xs + width - 1, ys + height - 1);

    bool first = true;
    for (int y = 0; y < height; y += PARALLEL_LINES) {
        int lines = (height - y < PARALLEL_LINES) ? (height - y) : PARALLEL_LINES;
        int idx = (y / PARALLEL_LINES) % 2;
        uint16_t *buf = line_buf[idx];

        xSemaphoreTake(buf_free_sem, portMAX_DELAY);
        for (int ly = 0; ly < lines; ly++) {
            const uint8_t *src = data[y + ly];
            uint16_t *dst = buf + (size_t)ly * width;
            for (int x = 0; x < width; x++) dst[x] = myPalette[src[x]];
        }
        esp_lcd_panel_io_tx_color(io_handle, first ? 0x2C : -1, buf, (size_t)lines * width * 2);
        first = false;
    }
}

static void backlight_init(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << PIN_NUM_BCKL,
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&io_conf);
    gpio_set_level(PIN_NUM_BCKL, 1);
}

void ili9341_init(void)
{
    spi_bus_config_t buscfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD_WIDTH * 2 * PARALLEL_LINES,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO));

    esp_lcd_panel_io_spi_config_t io_config = {
        .cs_gpio_num = PIN_NUM_CS,
        .dc_gpio_num = PIN_NUM_DC,
        .spi_mode = 0,
        .pclk_hz = LCD_PCLK_HZ,
        .trans_queue_depth = 10,
        .on_color_trans_done = color_trans_done,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_SPI_HOST, &io_config, &io_handle));

    line_buf[0] = heap_caps_malloc(LCD_WIDTH * 2 * PARALLEL_LINES, MALLOC_CAP_DMA);
    line_buf[1] = heap_caps_malloc(LCD_WIDTH * 2 * PARALLEL_LINES, MALLOC_CAP_DMA);
    buf_free_sem = xSemaphoreCreateCounting(2, 2);

    for (size_t i = 0; i < sizeof(ili9341v_init_seq) / sizeof(ili9341v_init_seq[0]); i++) {
        const lcd_init_cmd_t *c = &ili9341v_init_seq[i];
        esp_lcd_panel_io_tx_param(io_handle, c->cmd, c->len ? c->data : NULL, c->len);
        if (c->delay_ms) vTaskDelay(pdMS_TO_TICKS(c->delay_ms));
    }

    backlight_init();
}

/*
** 8x8 font -- only the glyphs the control overlay needs (A, B, S, E, and 4 arrows).
*/
static const uint8_t font8x8[128][8] = {
    ['A'] = {0x18, 0x3C, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x00},
    ['B'] = {0x7C, 0x66, 0x66, 0x7C, 0x66, 0x66, 0x7C, 0x00},
    ['E'] = {0x7E, 0x60, 0x60, 0x78, 0x60, 0x60, 0x7E, 0x00},
    ['S'] = {0x3C, 0x66, 0x60, 0x3C, 0x06, 0x66, 0x3C, 0x00},
    [GLYPH_ARROW_UP]    = {0x18, 0x3C, 0x7E, 0x18, 0x18, 0x18, 0x18, 0x00},
    [GLYPH_ARROW_DOWN]  = {0x18, 0x18, 0x18, 0x18, 0x7E, 0x3C, 0x18, 0x00},
    [GLYPH_ARROW_LEFT]  = {0x08, 0x18, 0x38, 0x7C, 0x7C, 0x38, 0x18, 0x08},
    [GLYPH_ARROW_RIGHT] = {0x10, 0x18, 0x1C, 0x3E, 0x3E, 0x1C, 0x18, 0x10},
};

void ili9341_draw_char(uint16_t x, uint16_t y, char c, uint16_t fg, uint16_t bg, uint8_t scale)
{
    uint16_t sfg = (fg >> 8) | (fg << 8);
    uint16_t sbg = (bg >> 8) | (bg << 8);
    uint16_t w = 8 * scale, h = 8 * scale;
    uint16_t buf[8 * 8 * 4]; // supports up to scale=2 (16x16); callers use scale<=2

    const uint8_t *glyph = font8x8[(unsigned char)c & 0x7F];
    for (int gy = 0; gy < 8; gy++) {
        for (int sy = 0; sy < scale; sy++) {
            int row = gy * scale + sy;
            for (int gx = 0; gx < 8; gx++) {
                uint16_t px = (glyph[gy] & (0x80 >> gx)) ? sfg : sbg;
                for (int sx = 0; sx < scale; sx++) {
                    buf[row * w + gx * scale + sx] = px;
                }
            }
        }
    }

    set_window(x, y, x + w - 1, y + h - 1);
    // Drain both pool tokens first so no line_buf transfer is in flight concurrently,
    // then wait for our own transfer's completion callback before returning, since
    // `buf` is stack-allocated and must not go out of scope mid-DMA.
    xSemaphoreTake(buf_free_sem, portMAX_DELAY);
    xSemaphoreTake(buf_free_sem, portMAX_DELAY);
    esp_lcd_panel_io_tx_color(io_handle, 0x2C, buf, (size_t)w * h * 2);
    xSemaphoreTake(buf_free_sem, portMAX_DELAY);
    xSemaphoreGive(buf_free_sem);
    xSemaphoreGive(buf_free_sem);
}
