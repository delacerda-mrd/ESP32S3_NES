# ESP32-2432S028 (CYD) Board Reference

Complete hardware reference for the ESP32-2432S028 "Cheap Yellow Display" 2.8" TFT ESP32 Display Module.
Compiled from official schematic (V0.1, 2022-07-09/15), manufacturer datasheets, community documentation
(github.com/witnessmenow/ESP32-Cheap-Yellow-Display), and physical IC markings confirmed via board photography.

Manufacturer: Shenzhen Jingcai Intelligent Co., Ltd (Sunton)
PCB model: ESP32-2432S028
Community name: CYD (Cheap Yellow Display)

---

## 1. SoC

| Parameter | Value |
|-----------|-------|
| Module | ESP-32S (ESP32-WROOM-32). Marking on module: `ESP-32S` |
| Chip | ESP32-D0WDQ6 (dual-core Xtensa LX6) |
| Clock | 240 MHz (max) |
| Internal memory | 448 KB ROM + 520 KB SRAM |
| PSRAM | None |
| External flash | 4 MB QSPI (Winbond 25Q32JVSSIQ) |
| Wi-Fi | 2.4 GHz, 802.11 b/g/n |
| Bluetooth | V4.2 BR/EDR + BLE |
| USB | Via CH340C USB-to-UART bridge (not native USB) |
| Operating voltage | 3.0–3.6 V (core), 5V via USB input |
| Operating temperature | -40 to +85 °C |

Onboard CH340C provides USB-to-UART. Requires CH340 driver on some OS versions.
Micro-USB connector only (no USB-C on standard variant).

### Color Inversion (Verified)

The single-USB CYD displays inverted colors by default with ILI9341_2_DRIVER.
Add `TFT_INVERSION_ON` to fix. In TFT_eSPI User_Setup.h: `#define TFT_INVERSION_ON`.
In PlatformIO build_flags: `-DTFT_INVERSION_ON=1`.

### CYD2USB Variant Note

A variant also labeled ESP32-2432S028 exists with dual USB ports (Micro-USB + USB-C).
Differences: inverted display colors (requires software gamma fix), USB-C port non-functional
with C-to-C cables due to missing CC line resistors.

---

## 2. USB-to-UART Bridge

| Parameter | Value |
|-----------|-------|
| IC | CH340C (WCH / Nanjing Qinheng). Marking: `WCH CH340C / 205695F` |
| Reference | U6 |
| Crystal | CN5010W |
| Interface | USB 2.0 Full Speed |
| Baud rate | Up to 2 Mbps |
| Auto-reset | Yes — DTR/RTS via S9013 NPN transistors (T1, T2) to EN + IO0 |

Auto-reset circuit uses two S9013 NPN transistors with 10KΩ pull-ups (R1, R14) on IO0 and RST.
Known issue: C4 (0.1µF) between EN and GND may cause boot-mode detection failures on some units.
Fix: replace C4 with 1–10µF.

---

## 3. Display

| Parameter | Value |
|-----------|-------|
| Size | 2.8 inch |
| Type | TFT (not IPS) |
| Driver IC | ILI9341 |
| Resolution | 240 x 320 (portrait native) |
| Color depth | 65K (RGB565) |
| Interface | 4-wire SPI (HSPI bus) |
| Backlight | White LED, active high via MOSFET |

### Display GPIO Mapping

| Signal | GPIO | Notes |
|--------|------|-------|
| TFT_CS | 15 | Chip select, active low |
| TFT_DC (RS) | 2 | Data/command select. High=data, low=command |
| TFT_SCLK | 14 | SPI clock (HSPI CLK) |
| TFT_MOSI (SDI) | 13 | SPI data write (HSPI MOSI) |
| TFT_MISO (SDO) | 12 | SPI data read (HSPI MISO) |
| TFT_RST | -1 | Tied to board RST (CHIP_PU), use -1 in code |
| TFT_BL | 21 | Backlight control. High=on, low=off |

### Backlight Circuit

GPIO21 drives an AO3402 N-channel MOSFET (Q2) to switch the backlight LEDs.
HIGH = backlight on, LOW = backlight off. PWM on this pin controls brightness.

**Important:** Do not connect external devices to GPIO21 (P3 connector pin) — it controls the backlight.

### SPI Configuration

| Parameter | Value |
|-----------|-------------|
| SPI port | HSPI (SPI2) |
| Recommended SPI clock | 55 MHz (TFT_eSPI default) |
| Read clock | 20 MHz |
| SPI mode | MODE0 |
| Touch SPI clock | 2.5 MHz |

### MADCTL Rotation Values (reg 0x36)

| Rotation | MADCTL | Orientation | Effective size |
|----------|--------|-------------|----------------|
| 0 | 0x08 | Portrait | 240 x 320 |
| 1 | 0x68 | Landscape | 320 x 240 |
| 2 | 0xC8 | Portrait inverted | 240 x 320 |
| 3 | 0xA8 | Landscape inverted | 320 x 240 |

---

## 4. Resistive Touch (XPT2046)

| Parameter | Value |
|-----------|-------|
| Driver IC | XPT2046 (Shenzhen XPT). Marking: `XPT XPT2046 / A8ECC8` |
| Reference | U3 |
| Interface | SPI (separate from display SPI) |
| Touch type | Resistive (4-wire) |
| Max touch points | 1 |

### Touch GPIO Mapping

| Signal | GPIO | Notes |
|--------|------|-------|
| TP_CLK | 25 | SPI clock |
| TP_CS | 33 | Chip select |
| TP_DIN (MOSI) | 32 | SPI data in to XPT2046 |
| TP_OUT (MISO) | 39 | SPI data out from XPT2046 (input-only GPIO) |
| TP_IRQ | 36 | Interrupt, active low on touch (input-only GPIO) |

### SPI Bus Conflict

The CYD uses three separate SPI consumers on two hardware SPI buses:
- **HSPI:** Display (ILI9341)
- **VSPI:** SD card
- **Touch (XPT2046):** Uses yet another set of GPIOs

Display and SD card each occupy one hardware SPI bus. Touch must use software (bit-bang) SPI
if all three peripherals are needed simultaneously. Library: `XPT2046_Bitbang_Slim`.

### Touch Calibration

Resistive touch requires calibration. Raw ADC values (0–4095) must be mapped to pixel coordinates.
Calibration varies per unit. Typical approach: use TFT_eSPI calibration sketch, store 5 calibration
values (one per corner + rotation).

### Touch Axis Mapping — Landscape Rotation=1 (Verified)

With bit-bang SPI and display rotation=1 (320×240 landscape), the XPT2046 raw axes map as:
- Raw Y channel → screen X (not inverted): `map(rawY, 200, 3800, 0, 319)`
- Raw X channel → screen Y (not inverted): `map(rawX, 300, 3900, 0, 239)`

The min/max values (200/3800, 300/3900) are typical and may need per-unit tuning.

---

## 5. Audio

| Parameter | Value |
|-----------|-------|
| Amplifier IC | 8002B (SC8002B/FM8002A family). Marking: `8002B / CNC221H` |
| Reference | U5 |
| Input pin | GPIO26 (DAC2) |
| Speaker connector | PH 2.0mm 2-pin JST (P4) |
| Max output | ~1W (8Ω speaker) |
| Shutdown pin | Directly tied (always on when powered) |

### Audio Circuit

GPIO26 is an ESP32 DAC output. The signal passes through an RC filter network
(R4=4.7KΩ, R7=47KΩ, C8=0.38µF, R9=68KΩ) into the 8002B amplifier.
The amplifier SHUTDOWN pin is not connected to a GPIO — it's tied active via passive components.
Audio output is mono, driven by DAC (no I2S on this board).

Speaker connector is labeled "SPEAK" on the PCB back. Most 8Ω speakers with 1.25mm or 2.0mm
JST connectors are compatible.

---

## 6. MicroSD Card

| Parameter | Value |
|-----------|-------|
| Interface | SPI (VSPI bus) |
| Card type | MicroSD / TF card |

### SD GPIO Mapping

| Signal | GPIO | Notes |
|--------|------|-------|
| SD_CS (SS) | 5 | Chip select |
| SD_SCK | 18 | SPI clock (VSPI CLK) |
| SD_MISO | 19 | SPI data read (VSPI MISO) |
| SD_MOSI | 23 | SPI data write (VSPI MOSI) |

Arduino init: `SD.begin(5)` using default VSPI bus.

---

## 7. RGB LED

| Parameter | Value |
|-----------|-------|
| Type | Common-anode discrete RGB LED |
| Logic | Active LOW (LOW = on, HIGH = off) |

### RGB LED GPIO Mapping

| Color | GPIO | Notes |
|-------|------|-------|
| Red | 4 | Active low |
| Green | 16 | Active low |
| Blue | 17 | Active low |

**Not a WS2812B.** This is a standard common-anode RGB LED with individual GPIO control per color.
Each GPIO sinks current through the LED. `digitalWrite(pin, LOW)` turns that color ON.
PWM on these pins allows color mixing and brightness control.

---

## 8. Light Dependent Resistor (LDR)

| Parameter | Value |
|-----------|-------|
| Type | CdS photoresistor (GT36516) |
| Reference | R21 |
| ADC pin | GPIO34 (ADC1_CH6, input-only) |
| Circuit | Voltage divider with R15 (1MΩ) and R19 (1MΩ) |

The LDR is soldered on the PCB front, near the display. Reads ambient light level.
Higher ADC value = more light. Use `analogRead(34)` to read.

---

## 9. Buttons

| Button | GPIO | Function |
|--------|------|----------|
| BOOT | 0 (IO0) | Download mode (hold during reset) or general input |
| RESET | EN (CHIP_PU) | Hardware reset |

BOOT button: active low with internal pull-up. Connected through the auto-reset circuit
(S9013 transistors T1/T2) for automatic programming via serial DTR/RTS.

---

## 10. Expansion Connectors (Back of Board)

### P3 — GPIO Expansion Header (1.25mm JST, 4-pin)

| Pin | Signal | Notes |
|-----|--------|-------|
| 1 | GND | Ground |
| 2 | IO35 | Input-only GPIO (no pull-up/down) |
| 3 | IO22 | General GPIO (also I2C SCL on CN1) |
| 4 | IO21 | **Backlight control** — do NOT use for external devices |

**Warning:** IO21 on P3 is the backlight pin. Connecting external hardware to it will
interfere with the display backlight.

### CN1 — I2C/GPIO Expansion Header (1.25mm JST, 4-pin)

| Pin | Signal | Notes |
|-----|--------|-------|
| 1 | GND | Ground |
| 2 | 3.3V | 3.3V power output |
| 3 | IO22 (SDA) | I2C data (shared with P3) |
| 4 | IO27 (SCL) | I2C clock |

Located nearest the MicroSD card slot. Can be used for I2C peripherals (e.g., Wii Nunchuck,
sensors). No external pull-up resistors needed — internal pull-ups are sufficient.

### P1 — Serial/Power Header (4-pin)

| Pin | Signal | GPIO |
|-----|--------|------|
| 1 | 5V | — |
| 2 | RXD | 3 |
| 3 | TXD | 1 |
| 4 | GND | — |

IO1 (TXD) and IO3 (RXD) may be usable as GPIO after boot, but this is not guaranteed.
Primary use is serial programming/debug via CH340C.

### P4 — Speaker Header (PH 2.0mm JST, 2-pin)

Connected to 8002B amplifier output. Labeled "SPEAK" on PCB.

---

## 11. Power

| Parameter | Value |
|-----------|-------|
| Input | 5V via Micro-USB |
| LDO (ESP32) | AMS1117-3.3 (U1) → 3.3V (labeled 3.3V-ESP) |
| LDO (Display) | AMS1117-3.3 (separate) → 3.3V (labeled 3.3V-TFT) |
| Protection | AO3401 P-channel MOSFET (Q1) reverse polarity protection |
| Decoupling | Multiple 0.1µF / 10µF / 16V ceramics |

Two separate AMS1117-3.3 LDOs: one for the ESP32 module, one for the TFT display.
5V input passes through AO3401 MOSFET for reverse-polarity protection.

---

## 12. Physical Dimensions

| Dimension | mm |
|-----------|-----|
| PCB (W x H) | 50.0 x 78.0 |
| Corner radius | 4.0 |
| Mounting holes | 4x, at 4mm inset from each corner |

---

## 13. Complete GPIO Allocation Table

Every ESP32 GPIO pin and its function on this board:

| GPIO | Function | Peripheral | Available? |
|------|----------|-----------|------------|
| 0 | BOOT button | Button / Auto-reset | Shared (key when not flashing) |
| 1 | U0TXD | Serial (P1 header) | In use (serial TX) |
| 2 | TFT_DC (RS) | Display (ILI9341) | In use |
| 3 | U0RXD | Serial (P1 header) | In use (serial RX) |
| 4 | RGB LED Red | LED | In use (active low) |
| 5 | SD_CS | MicroSD (VSPI) | In use |
| 6–11 | — | Internal flash (SPI) | Reserved / not exposed |
| 12 | TFT_MISO | Display (ILI9341, HSPI) | In use |
| 13 | TFT_MOSI | Display (ILI9341, HSPI) | In use |
| 14 | TFT_SCLK | Display (ILI9341, HSPI) | In use |
| 15 | TFT_CS | Display (ILI9341) | In use |
| 16 | RGB LED Green | LED | In use (active low) |
| 17 | RGB LED Blue | LED | In use (active low) |
| 18 | SD_SCK | MicroSD (VSPI) | In use |
| 19 | SD_MISO | MicroSD (VSPI) | In use |
| 21 | TFT_BL | Backlight (MOSFET Q2) | In use (also on P3 header) |
| 22 | Expansion | P3 + CN1 headers | Free (I2C SDA capable) |
| 23 | SD_MOSI | MicroSD (VSPI) | In use |
| 25 | TP_CLK | Touch (XPT2046) | In use |
| 26 | Speaker DAC | Audio (8002B amp) | In use (DAC2) |
| 27 | Expansion | CN1 header | Free (I2C SCL capable) |
| 32 | TP_DIN | Touch (XPT2046) | In use |
| 33 | TP_CS | Touch (XPT2046) | In use |
| 34 | LDR ADC | Light sensor (ADC1_CH6) | In use (input-only) |
| 35 | Expansion | P3 header | Free (input-only) |
| 36 | TP_IRQ | Touch (XPT2046) | In use (input-only) |
| 39 | TP_OUT | Touch (XPT2046) | In use (input-only) |

**Free GPIOs:** 22 (P3/CN1), 27 (CN1), 35 (P3, input-only). Also 1/3 if serial is unused after boot.

---

## 14. Key IC Reference

| IC | Part Number | Marking (confirmed) | Function | Reference |
|----|-------------|---------------------|----------|-----------|
| SoC module | ESP32-WROOM-32 | `ESP-32S` | Main controller | U2 |
| Display driver | ILI9341 | — (under display FPC) | TFT LCD driver | — |
| Touch controller | XPT2046 | `XPT XPT2046 / A8ECC8` | Resistive touch ADC | U3 |
| USB-UART bridge | CH340C | `WCH CH340C / 205695F` | USB to serial | U6 |
| Audio amplifier | 8002B | `8002B / CNC221H` | Class-D mono amp | U5 |
| LDR | GT36516 | — | Photoresistor | R21 |
| Flash | 25Q32JVSSIQ | — (inside ESP module) | 4MB QSPI flash | — |
| LDO (x2) | AMS1117-3.3 | — | 3.3V regulator | U1 + unnamed |
| Backlight FET | AO3402 | — | N-channel MOSFET | Q2 |
| Power FET | AO3401 | — | P-channel MOSFET (reverse polarity) | Q1 |
| Auto-reset (x2) | S9013 | — | NPN transistors (DTR/RTS) | T1, T2 |

---

## 15. Arduino/PlatformIO Quick Reference

### PlatformIO board

```ini
[env:cyd]
platform = espressif32
board = esp32dev
framework = arduino
upload_speed = 115200
monitor_speed = 115200
```

### TFT_eSPI User_Setup.h

```c
#define ILI9341_2_DRIVER
#define TFT_WIDTH  240
#define TFT_HEIGHT 320
#define TFT_MISO   12
#define TFT_MOSI   13
#define TFT_SCLK   14
#define TFT_CS     15
#define TFT_DC     2
#define TFT_RST    -1
#define TFT_BL     21
#define TFT_BACKLIGHT_ON HIGH
#define TFT_INVERSION_ON
#define SPI_FREQUENCY       55000000
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY  2500000
#define USE_HSPI_PORT
#define TOUCH_CS   33
```

### Touch init (TFT_eSPI built-in)

```cpp
// XPT2046 is handled by TFT_eSPI when TOUCH_CS is defined
tft.setTouch(calData);  // calData from calibration sketch
```

### Touch init (standalone XPT2046_Bitbang)

```cpp
#include <XPT2046_Bitbang.h>
XPT2046_Bitbang touchscreen(32, 25, 39, 33);  // MOSI, CLK, MISO, CS
```

### SD card init

```cpp
#include <SD.h>
SD.begin(5);  // CS on GPIO5, uses VSPI by default
```

### RGB LED

```cpp
#define LED_R 4
#define LED_G 16
#define LED_B 17
// Active LOW — LOW = on, HIGH = off
pinMode(LED_R, OUTPUT);
pinMode(LED_G, OUTPUT);
pinMode(LED_B, OUTPUT);
digitalWrite(LED_R, HIGH);  // Red off
digitalWrite(LED_G, HIGH);  // Green off
digitalWrite(LED_B, HIGH);  // Blue off
```

### Audio (DAC)

```cpp
// GPIO26 = DAC_CHANNEL_2
dacWrite(26, value);  // 0-255, 8-bit DAC output
```

### LDR (ambient light)

```cpp
int light = analogRead(34);  // 0-4095, higher = brighter
```

---

## 16. Flashing / Download Mode

1. **Automatic:** PlatformIO/Arduino IDE toggles DTR/RTS via CH340C auto-reset circuit. Just click Upload.
2. **Manual if auto-reset fails:** Hold BOOT → press RESET → release RESET → release BOOT → flash.
3. **Port:** `/dev/ttyUSB0` (Linux), COM port (Windows). Requires CH340 driver.
4. **Upload speed:** 115200 recommended (some units fail at higher speeds).
5. **Known issue:** If board enters download mode randomly or fails to reset, replace C4 (0.1µF on EN line) with 1–10µF.

---

## 17. Key Differences from ES3C28P (QDtech 2.8")

| Feature | CYD (ESP32-2432S028) | ES3C28P |
|---------|---------------------|---------|
| SoC | ESP32 (LX6, no PSRAM) | ESP32-S3R8 (LX7, 8MB PSRAM) |
| Flash | 4 MB | 16 MB |
| USB | CH340C bridge (Micro-USB) | Native USB-CDC (Type-C) |
| Display | ILI9341 TFT | ILI9341V IPS |
| Touch | Resistive (XPT2046, SPI) | Capacitive (FT6336G, I2C) |
| Audio | DAC → 8002B (no codec) | ES8311 I2S codec → FM8002E |
| Microphone | None | MEMS (via ES8311) |
| SD card | SPI (VSPI) | 4-bit SDIO |
| RGB LED | Discrete common-anode (3 GPIO) | WS2812B addressable (1 GPIO) |
| Battery | None | TP4054 charger + ADC |
| LDR | Yes (GPIO34) | None |
| I2C header | Yes (IO22/IO27) | Yes (IO15/IO16, shared bus) |
| Free GPIOs | 22, 27, 35 | 2, 3, 14, 21 |
| Bluetooth | 4.2 | 5.0 |
| Wi-Fi | 2.4 GHz | 2.4 GHz |
| Price | ~$7–10 | ~$15–20 |
