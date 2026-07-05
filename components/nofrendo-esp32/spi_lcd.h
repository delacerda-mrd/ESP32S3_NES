// LCD driver for ILI9341V (QDtech ES3C28P), using esp_lcd over SPI2.
#ifndef _DRIVER_SPI_LCD_H_
#define _DRIVER_SPI_LCD_H_
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

// Glyph codes for arrows, used with ili9341_draw_char() in addition to normal ASCII.
#define GLYPH_ARROW_UP    0x01
#define GLYPH_ARROW_DOWN  0x02
#define GLYPH_ARROW_LEFT  0x03
#define GLYPH_ARROW_RIGHT 0x04

void ili9341_init(void);

// data[y][x] are 8-bit indices into myPalette[], or NULL to fill the region black.
void ili9341_write_frame(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *data[]);

// rgb565 is normal (non-byte-swapped) RGB565; the driver swaps internally.
void ili9341_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t rgb565);

// 8x8 font scaled by an integer factor. fg/bg are normal RGB565.
void ili9341_draw_char(uint16_t x, uint16_t y, char c, uint16_t fg, uint16_t bg, uint8_t scale);

#ifdef __cplusplus
}
#endif

#endif
