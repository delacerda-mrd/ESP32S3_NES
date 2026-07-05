# ESP32S3_NES — Project Context for Claude Code

## SESSION START PROTOCOL (do this first, every session)
1. Read `docs/JOURNAL.md` — top section is **Current Status**: what works, what's broken, what we were doing last.
2. Read `docs/HARDWARE.md` if the task touches pins, display, touch, or audio.
3. Do NOT re-derive project context by exploring; these docs are the source of truth.

## SESSION END PROTOCOL
Before the user leaves (or when they say "wrap up" / "save state"), update
`docs/JOURNAL.md`: refresh the Current Status block and append a dated session
entry (what was tried, what happened, next steps). This is what makes resuming work.

## What this project is
Port of the Nofrendo NES emulator to the **QDtech ES3C28P** board:
ESP32-S3R8 (8MB octal PSRAM, 16MB flash), 2.8" 320x240 ILI9341V IPS SPI LCD,
FT6336G capacitive touch (on-screen controls), ES8311 audio codec + FM8002E amp.
ESP-IDF v5.x CMake project. Upstream ancestry: espressif/esp32-nesemu.

## Layout
- `main/main.c` — entry point; mmaps ROM from the `nesgame` partition (type 0x40) and calls `nofrendo_main()`
- `components/nofrendo/` — the emulator core (GPLv2, mostly don't touch)
- `components/nofrendo-esp32/` — ALL the hardware glue; this is where troubleshooting happens:
  - `spi_lcd.c/.h` — ILI9341V driver
  - `video_audio.c` — frame blitting, I2S/ES8311 audio, `osd.c` wiring
  - `touch_controller.c/.h` — FT6336G input → NES pad mapping
  - `board_i2c.c/.h` — shared I2C bus (touch @0x38, codec @0x18)
  - `Kconfig.projbuild` — menuconfig hardware options
- `partitions.csv` — custom table; ROM lives at **0x100000**, 3MB
- `flashrom.sh <rom.nes>` — flashes a ROM to the game partition

## Build / flash / debug commands
```bash
idf.py set-target esp32s3        # once, after full clean
idf.py build
idf.py -p /dev/ttyACM0 flash monitor   # macOS: /dev/cu.usbmodem*
./flashrom.sh <rom.nes>          # ROM goes to 0x100000
```
- Board has **no UART bridge** — native USB-CDC (USB Serial/JTAG console).
  If the port won't appear: hold BOOT while plugging in / pressing RESET.
- Find the port on macOS: `ls /dev/cu.usbmodem*`

## Key hardware facts (full detail in docs/HARDWARE.md)
- LCD SPI: CS=10, DC=46, SCLK=12, MOSI=11, MISO=13, BL=45. LCD RST tied to chip reset.
- NES 256px frame drawn at x=48; left 48px = D-pad touch zone, right 16px = A/B/Start/Select bar.
- Amp enable: GPIO1, **active low**.
- PSRAM is octal @80MHz; flash is QIO 16MB @80MHz. CPU 240MHz.

## Conventions
- Don't commit unless asked. Don't edit `components/nofrendo/` core unless the bug is provably there.
- sdkconfig is checked in; prefer changing `sdkconfig.defaults` + menuconfig-visible options in Kconfig.projbuild.
- New durable discoveries (pin corrections, register quirks, timing fixes) go in `docs/HARDWARE.md`; narrative debugging history goes in `docs/JOURNAL.md`.
