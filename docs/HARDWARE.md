# Hardware Reference — QDtech ES3C28P

Durable hardware facts only. When debugging reveals a correction (wrong pin,
init quirk, timing constraint), fix it HERE so it's never re-discovered.

## Board
- **QDtech ES3C28P**: ESP32-S3R8, 16MB QIO flash @80MHz, 8MB octal PSRAM @80MHz
- No UART bridge — console is native **USB Serial/JTAG** (USB-CDC).
  Download mode: hold BOOT while plugging in USB or pressing RESET.

## Display — ILI9341V, 2.8" 320x240 IPS, SPI
| Signal | GPIO |
|--------|------|
| CS     | 10   |
| DC     | 46   |
| SCLK   | 12   |
| MOSI   | 11   |
| MISO   | 13   |
| BL     | 45   |
| RST    | tied to chip reset (no GPIO) |

- Landscape 320x240. NES frame (256px wide) drawn at x=48.
- Driver: `components/nofrendo-esp32/spi_lcd.c`

## Touch — FT6336G capacitive, I2C addr 0x38
- Driver: `components/nofrendo-esp32/touch_controller.c`
- On-screen controls: left 48px column = D-pad; right 16px column = A/B/Start/Select.
  Zones drawn once at startup by the LCD code; touch maps coordinates → NES pad bits.

## Audio — ES8311 codec (I2C addr 0x18) + FM8002E amp
- I2S from ESP32-S3 → ES8311 DAC → FM8002E speaker amp
- **Amp enable: GPIO1, active LOW**
- Codec driver pulled via IDF Component Manager: `espressif/es8311` (see `idf_component.yml`)
- Code: `components/nofrendo-esp32/video_audio.c`

## I2C bus (shared: touch + codec)
- Setup in `components/nofrendo-esp32/board_i2c.c`
- Pins: see `board_i2c.h` / Kconfig — record them here once confirmed on hardware.

## Flash map (`partitions.csv`)
| Partition | Type      | Offset   | Size  |
|-----------|-----------|----------|-------|
| nvs       | data/nvs  | 0x9000   | 24KB  |
| phy_init  | data/phy  | 0xf000   | 4KB   |
| factory   | app       | 0x10000  | 896KB |
| nesgame   | 0x40/0x01 | 0x100000 | 3MB   |

ROM is not built in — flash one with `./flashrom.sh <rom.nes>` (writes to 0x100000).
`main.c` finds the partition by type 0x40 and mmaps it.

## Gotchas discovered (append as found)
- _none yet_
