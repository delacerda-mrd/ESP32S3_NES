# ES3C28P Board Reference

Complete hardware reference for the QDtech ES3C28P 2.8" IPS ESP32-S3 Display Module.
Compiled from official specification (V1.0, 2025-06-14), schematic, datasheets, SDK example code,
and physical IC markings confirmed via board photography.

Manufacturer: ShenZhen QDtech Electronic Technology Co., LTD (LCDWIKI)
SKU: ES3C28P (capacitive touch) / ES3N28P (no touch)
Document ref: CR2025-MI6872

---

## 1. SoC

| Parameter | Value |
|-----------|-------|
| Chip | ESP32-S3R8 (QFN56, rev v0.2). Marking: `ESP32-S3 / 512025 / R8MTH459000 / FC00MCM255` |
| CPU | Xtensa LX7 32-bit dual-core + LP core |
| Clock | 240 MHz (max) |
| Internal memory | 384 KB ROM + 512 KB SRAM + 16 KB RTC SRAM |
| Embedded PSRAM | 8 MB OPI (AP_3v3 variant, 3.3V) |
| External flash | 16 MB QSPI (Zbit Semi 25VQ128DSJG, vendor ID 0x5E). Marking: `Z KK2544 / 25VQ128DSJG / P4N691` |
| Wi-Fi | 2.4 GHz, 802.11 b/g/n |
| Bluetooth | V5.0 BR/EDR + BLE |
| USB | Native USB-JTAG/CDC (VID 303a, PID 1001) |
| Operating voltage | 3.0–3.6 V |
| Operating temperature | -40 to +85 °C |

No onboard USB-to-UART bridge. Type-C connects directly to ESP32-S3 internal USB.
Win10+ supports USB virtual serial port natively; older Windows does not.

---

## 2. Display

| Parameter | Value |
|-----------|-------|
| Size | 2.8 inch |
| Type | IPS TFT |
| Driver IC | ILI9341V |
| Resolution | 240 x 320 (portrait native) |
| Color depth | 56K (RGB565 typical), 262K max (RGB666) |
| Interface | 4-wire SPI |
| Effective display area | 43.20 (W) x 57.60 (H) mm |
| Pixel size | 0.153 x 0.153 mm |
| Viewing angle | All directions (0'clock) |
| Backlight | White LED x4, 280 cd/m² typical |
| Backlight current | 79 mA |
| Polarizer mode | Transmissive/Positive |

### Display GPIO Mapping

| Signal | GPIO | Notes |
|--------|------|-------|
| LCD_CS | 10 | Chip select, active low |
| LCD_DC (RS) | 46 | Data/command select. High=data, low=command |
| LCD_SCLK | 12 | SPI clock |
| LCD_MOSI (SDA) | 11 | SPI data write |
| LCD_MISO (SDO) | 13 | SPI data read |
| LCD_RST | CHIP_PU | Shared with ESP32-S3 reset (use -1 in code) |
| LCD_BL | 45 | Backlight control. High=on, low=off. PWM supported |

### Backlight Circuit

GPIO45 drives a BSS138 N-channel MOSFET through a 0R series resistor.
HIGH = backlight on, LOW = backlight off. PWM on this pin controls brightness.
The MOSFET switches the LEDK (cathode) lines of the 4 white backlight LEDs.

### SPI Configuration

| Parameter | Tested Value |
|-----------|-------------|
| SPI port | FSPI (SPI2) or HSPI — both work |
| Max SPI clock | 80 MHz (tested in vendor demos) |
| Recommended clock | 40 MHz (conservative) |
| SPI mode | MODE0 |
| Read clock | 20 MHz |

### ILI9341V Init Sequence

The IPS variant requires display inversion ON (`0x21`). Full vendor init:

```
0xCF: 00 C1 30
0xED: 64 03 12 81
0xE8: 85 00 78
0xCB: 39 2C 00 34 02
0xF7: 20
0xEA: 00 00
0xC0: 13              // Power control: VRH
0xC1: 13              // Power control: SAP/BT
0xC5: 22 35           // VCM control
0xC7: BD              // VCM control2
0x21                   // Display Inversion ON (required for IPS)
0x36: 08              // MADCTL: BGR color order, portrait
0xB6: 0A A2           // Display function control
0x3A: 55              // Pixel format: 16-bit RGB565
0xF6: 01 30           // Interface control: MCU
0xB1: 00 1B           // Frame rate: 70 Hz
0xF2: 00              // 3-Gamma function disable
0x26: 01              // Gamma curve 1
0xE0: 0F 35 31 0B 0E 06 49 A7 33 07 0F 03 0C 0A 00  // Positive gamma
0xE1: 00 0A 0F 04 11 08 36 58 4D 07 10 0C 32 34 0F  // Negative gamma
0x11                   // Exit sleep (wait 120ms after)
0x29                   // Display on
```

### MADCTL Rotation Values (reg 0x36)

| Rotation | MADCTL | Orientation | Effective size |
|----------|--------|-------------|----------------|
| 0 | 0x08 | Portrait | 240 x 320 |
| 1 | 0x68 | Landscape | 320 x 240 |
| 2 | 0xC8 | Portrait inverted | 240 x 320 |
| 3 | 0xA8 | Landscape inverted | 320 x 240 |

All values have BGR bit set (bit 3). Firmware uses Arduino_GFX rotation 3.

---

## 3. Capacitive Touch (ES3C28P only)

| Parameter | Value |
|-----------|-------|
| Driver IC | FT6336G (FocalTech). Marking: `FT6336G / 6RS2301B` |
| Interface | I2C |
| I2C address | 0x38 |
| Touch type | Capacitive |
| Max touch points | 2 |
| Structural material | G+F |
| Effective touch area | 43.20 (W) x 57.60 (H) mm |
| Visible window | 43.60±0.15 (W) x 58.05±0.15 (H) mm |

### Touch GPIO Mapping

| Signal | GPIO | Notes |
|--------|------|-------|
| CTP_SCL | 15 | I2C clock (shared bus) |
| CTP_SDA | 16 | I2C data (shared bus) |
| CTP_RST | 18 | Reset, active low |
| CTP_INT | 17 | Interrupt, active low on touch event |

### FT6336G Key Registers

| Register | Function |
|----------|----------|
| 0x00 | Device mode |
| 0x02 | TD_STATUS — low nibble = active finger count |
| 0x03–0x04 | Point 1 X (high nibble of 0x03 + full 0x04) |
| 0x05–0x06 | Point 1 Y (high nibble of 0x05 + full 0x06) |
| 0x09–0x0C | Point 2 X/Y (same format) |
| 0x80 | Touch threshold |
| 0x88 | Active period |
| 0xA0 | Cipher low (chip ID) |
| 0xA4 | Device mode register |
| 0xA5 | Power mode (0x00=active, 0x03=monitor) |
| 0xA8 | FocalTech ID |

Touch coordinates are in physical portrait orientation (240 x 320).
For landscape display (rotation 3), transform: `display_x = phys_y`, `display_y = 239 - phys_x`.

---

## 4. Audio Subsystem

### ES8311 Audio Codec

| Parameter | Value |
|-----------|-------|
| Codec IC | ES8311 (Everest Semiconductor) |
| I2C address | 0x18 (CE pin low) |
| I2C bus | Shared with touch on GPIO15/16 |
| Sample rate | 16 kHz (default in demos) |
| MCLK multiple | 384x sample rate |
| Resolution | 16-bit |

### I2S GPIO Mapping

| Signal | GPIO | Notes |
|--------|------|-------|
| I2S_MCK | 4 | Master clock |
| I2S_BCK (SCK) | 5 | Bit clock |
| I2S_DIN | 6 | Data in to ESP32 (from codec SDOUT/ASDOUT) |
| I2S_WS (LRCK) | 7 | Word select / L-R channel. High=right, low=left |
| I2S_DOUT | 8 | Data out from ESP32 (to codec DSDIN) |

### FM8002E Power Amplifier

| Parameter | Value |
|-----------|-------|
| Amplifier IC | FM8002E (confirmed on-board). Marking: `8002E / CQD826.1N` |
| Enable pin | GPIO1 |
| Enable logic | Active LOW (low = amp on, high = shutdown) |
| Max output | 1.5W (8Ω speaker) or 2W (4Ω speaker) |
| Speaker connector | 1.25mm 2-pin JST |

### MEMS Microphone

| Parameter | Value |
|-----------|-------|
| Mic IC | LMA2718B381-OA7 |
| Type | Downward-facing MEMS silicon microphone |
| Connection | Analog, routed through ES8311 codec (MIC1P/MIC1N) |

---

## 5. MicroSD Card

| Parameter | Value |
|-----------|-------|
| Interface | 4-bit SDIO (SD_MMC) |
| Card type | MicroSD / TF card |

### SD GPIO Mapping

| Signal | GPIO |
|--------|------|
| SD_CLK | 38 |
| SD_CMD | 40 |
| SD_D0 | 39 |
| SD_D1 | 41 |
| SD_D2 | 48 |
| SD_D3 | 47 |

Arduino init: `SD_MMC.setPins(38, 40, 39, 41, 48, 47)` then `SD_MMC.begin()`.

---

## 6. WS2812B RGB LED

| Parameter | Value |
|-----------|-------|
| LED IC | XL-5050RGBC-WS2812B |
| Data pin | GPIO42 |
| LED count | 1 |
| Protocol | NeoPixel (WS2812B, 800 kHz) |
| Power | 5V rail |
| Color order | GRB |

Arduino: `Adafruit_NeoPixel strip(1, 42, NEO_GRB + NEO_KHZ800)`

---

## 7. Battery Management

| Parameter | Value |
|-----------|-------|
| Charger IC | TP4054 |
| Battery type | 3.7V lithium polymer |
| Charging voltage | 4.2–6.5V input (typical 5V via USB) |
| Charging current | 290 mA (module actual), 500 mA max IC rating |
| Saturation voltage | 4.24V |
| Max charging temp | 62°C (module measured) |
| Battery connector | 1.25mm 2-pin JST |
| ADC pin | GPIO9 (ADC1_CH8) |
| Voltage divider | 2x 200KΩ (1:1 divider) |
| ADC formula | Battery mV = ADC voltage reading × 2 |

### Battery ADC Configuration (ESP-IDF)

```c
ADC_UNIT      = ADC_UNIT_1
ADC_CHANNEL   = ADC_CHANNEL_8    // GPIO9 on ESP32-S3
ADC_ATTEN     = ADC_ATTEN_DB_12
ADC_BITWIDTH  = ADC_BITWIDTH_12
```

Battery level estimation (from vendor demo):
- ≤2500 mV → 0%
- 2500–4200 mV → linear scale (v - 2500) / 17
- ≥4200 mV → 100%

---

## 8. Buttons

| Button | GPIO | Function |
|--------|------|----------|
| BOOT | 0 (IO0) | Download mode (hold during power-up) or general key |
| RESET | CHIP_PU | Low-level reset (ESP32-S3 + LCD shared) |

BOOT button: active low with internal pull-up. Press during power-on or with RESET sequence to enter download mode. When not in download mode, usable as a normal GPIO input.

---

## 9. Expansion Connectors (Back of Board)

### Expansion Pin Header (1.25mm 4P JST)

| Pin | GPIO | Notes |
|-----|------|-------|
| 1 | 2 | General IO, can be used as SPI |
| 2 | 3 | General IO, can be used as SPI |
| 3 | 14 | General IO, can be used as SPI |
| 4 | 21 | General IO, can be used as SPI |

These are 4 idle IO ports. Can be used as a secondary SPI bus or for connecting external peripherals.

### I2C Peripheral Header (1.25mm 4P JST)

Shared bus with touch controller and audio codec.
If touch and audio are not in use, GPIO15/16 can serve as general IO.

| Pin | Signal |
|-----|--------|
| 1 | GND |
| 2 | VCC (3.3V) |
| 3 | SDA (GPIO16) |
| 4 | SCL (GPIO15) |

### UART Header (1.25mm 4P JST)

Directly connected to ESP32-S3 UART0. Active low MOSFET (SL2305) + 100Ω series resistors on TX/RX lines.
Requires external USB-to-serial adapter for use.

| Pin | Signal | GPIO |
|-----|--------|------|
| 1 | GND | - |
| 2 | VCC | - |
| 3 | TXD0 | 43 |
| 4 | RXD0 | 44 |

### Speaker Header (1.25mm 2P JST)

Connected to FM8002E amplifier output. Max 1.5W (8Ω) or 2W (4Ω).

### Battery Header (1.25mm 2P JST)

3.7V LiPo battery input. Watch polarity — positive and negative must match.
Charges via TP4054 when USB power is connected.

---

## 10. I2C Bus Sharing

GPIO15 (SCL) and GPIO16 (SDA) are a shared I2C bus used by three devices:

| Device | Address | Function |
|--------|---------|----------|
| FT6336G | 0x38 | Capacitive touch controller |
| ES8311 | 0x18 | Audio codec |
| External | user-defined | I2C expansion header |

I2C speed: 400 kHz. Both lines have 4.7KΩ external pull-ups (R29, R30) to 3.3V.

---

## 11. Power

| Parameter | Value |
|-----------|-------|
| Input | 5V via USB-C |
| LDO (main) | ME6217C33M5G → 3.3V |
| LDO (audio) | ME6217C33M5G → 3.3V (separate, isolated ground) |
| Total current (display only) | 140 mA |
| Total current (all features) | 560 mA |
| Power (display only) | 0.7 W |
| Power (all features) | 2.8 W |
| ESP32-S3 in reset | ~0 mA |

Audio subsystem has its own 3.3V LDO and isolated ground plane (AU_GND) to reduce noise coupling.

---

## 12. Physical Dimensions

### ES3C28P (with touch screen)

| Dimension | mm |
|-----------|-----|
| Module (W x H x D) | 50.00 x 86.00 x 5.60 |
| PCB (W x H) | 50.00 x 86.00 |
| LCD screen (W x H x D) | 50.00±0.2 x 69.20±0.2 x 2.3±0.1 |
| Touch screen (W x H x D) | 50.00±0.1 x 69.20±0.1 x 1.0±0.1 |
| Mounting holes | 4x M2.5, 78.00mm apart (length), 40.00mm apart (width) |
| Weight (with packaging) | 86 g |

### ES3N28P (no touch)

| Dimension | mm |
|-----------|-----|
| Module (W x H x D) | 50.00 x 86.00 x 4.40 |
| Weight (with packaging) | 78 g |

---

## 13. Complete GPIO Allocation Table

Every ESP32-S3 GPIO pin and its function on this board:

| GPIO | Function | Peripheral | Available? |
|------|----------|-----------|------------|
| 0 | BOOT button | Button | Shared (key when not flashing) |
| 1 | Amplifier enable | Audio | In use (low=on) |
| 2 | Expansion pin | Expansion header P4 | Free |
| 3 | Expansion pin | Expansion header P4 | Free |
| 4 | I2S_MCK | Audio (ES8311) | In use |
| 5 | I2S_BCK | Audio (ES8311) | In use |
| 6 | I2S_DIN | Audio (ES8311) | In use |
| 7 | I2S_WS/LRCK | Audio (ES8311) | In use |
| 8 | I2S_DOUT | Audio (ES8311) | In use |
| 9 | BAT_ADC | Battery (ADC1_CH8) | In use (analog) |
| 10 | LCD_CS | Display (ILI9341) | In use |
| 11 | LCD_MOSI | Display (ILI9341) | In use |
| 12 | LCD_SCLK | Display (ILI9341) | In use |
| 13 | LCD_MISO | Display (ILI9341) | In use |
| 14 | Expansion pin | Expansion header P4 | Free |
| 15 | I2C_SCL | Touch + Audio + I2C header | In use (shared) |
| 16 | I2C_SDA | Touch + Audio + I2C header | In use (shared) |
| 17 | CTP_INT | Touch (FT6336G) | In use |
| 18 | CTP_RST | Touch (FT6336G) | In use |
| 19 | USB_D- | USB-C | In use (native USB) |
| 20 | USB_D+ | USB-C | In use (native USB) |
| 21 | Expansion pin | Expansion header P4 | Free |
| 26 | PSRAM CS | Internal OPI PSRAM | Reserved |
| 27–28 | PSRAM/Flash data | Internal buses | Reserved |
| 29 | Flash CS | External QSPI flash | Reserved |
| 30–32 | PSRAM/Flash data | Internal buses | Reserved |
| 33–37 | PSRAM data | Internal OPI PSRAM | Reserved |
| 38 | SD_CLK | MicroSD (SDIO) | In use |
| 39 | SD_D0 | MicroSD (SDIO) | In use |
| 40 | SD_CMD | MicroSD (SDIO) | In use |
| 41 | SD_D1 | MicroSD (SDIO) | In use |
| 42 | RGB_LED | WS2812B | In use |
| 43 | U0TXD | UART0 / UART header | Available if UART unused |
| 44 | U0RXD | UART0 / UART header | Available if UART unused |
| 45 | LCD_BL | Display backlight | In use |
| 46 | LCD_DC | Display (ILI9341) | In use |
| 47 | SD_D3 | MicroSD (SDIO) | In use |
| 48 | SD_D2 | MicroSD (SDIO) | In use |

**Free GPIOs:** 2, 3, 14, 21 (expansion header). Also 43/44 if UART0 is unused.

---

## 14. Key IC Reference

| IC | Part Number | Function | Datasheet in SDK |
|----|-------------|----------|-----------------|
| SoC | ESP32-S3R8 | Main controller | Yes |
| Display | ILI9341V | TFT LCD driver | Yes |
| Touch | FT6336G | Capacitive touch | Yes (+ register map XLSX) |
| Audio codec | ES8311 | I2S audio codec | Yes |
| Amplifier | FM8002E (marking: `8002E`) | Class-D audio amp | Yes |
| Microphone | LMA2718B381-OA7 | MEMS mic | Yes |
| Charger | TP4054 | Li-ion battery charger | Yes |
| LDO (x2) | ME6217C33M5G | 3.3V regulator | Yes |
| RGB LED | XL-5050RGBC-WS2812B | Addressable RGB | Yes |
| Flash | Zbit Semi 25VQ128DSJG (marking: `Z KK2544`) | 16MB QSPI flash | — |
| Backlight FET | BSS138 | N-channel MOSFET | — |
| USB ESD | LESD8LH5.0CT5G | ESD protection | — |
| UART FET | SL2305 | N-channel MOSFET | — |

---

## 15. Arduino/PlatformIO Quick Reference

### PlatformIO board

```ini
board = esp32-s3-devkitc-1
board_build.arduino.memory_type = qio_opi
board_upload.flash_size = 16MB
board_build.partitions = default_16MB.csv
```

### TFT_eSPI User_Setup.h (for TFT_eSPI library)

```c
#define ILI9341_DRIVER
#define TFT_MISO 13
#define TFT_MOSI 11
#define TFT_SCLK 12
#define TFT_CS   10
#define TFT_DC   46
#define TFT_RST  -1
#define TFT_BL   45
#define TFT_BACKLIGHT_ON HIGH
#define SPI_FREQUENCY    40000000
#define SPI_READ_FREQUENCY 20000000
#define USE_HSPI_PORT
```

### Arduino_GFX (GFX Library for Arduino)

```cpp
Arduino_DataBus* bus = new Arduino_ESP32SPI(46, 10, 12, 11, 13); // DC, CS, SCK, MOSI, MISO
Arduino_ILI9341* gfx = new Arduino_ILI9341(bus, -1, 3, true, 240, 320); // rst, rotation, IPS, w, h
```

### Touch init

```cpp
Wire.begin(16, 15);  // SDA, SCL
pinMode(17, INPUT_PULLUP);  // INT
// FT6336G at address 0x38
```

### SD card init

```cpp
SD_MMC.setPins(38, 40, 39, 41, 48, 47);  // CLK, CMD, D0, D1, D2, D3
SD_MMC.begin();
```

### RGB LED

```cpp
#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel strip(1, 42, NEO_GRB + NEO_KHZ800);
```

### Audio playback

```cpp
#define AP_ENABLE 1
#define I2S_MCK 4
#define I2S_BCK 5
#define I2S_WS  7
#define I2S_DOUT 8
#define I2S_DIN  6

pinMode(AP_ENABLE, OUTPUT);
digitalWrite(AP_ENABLE, LOW);  // Enable amplifier
// Init I2C for ES8311 on 0x18, then configure I2S
```

### Battery voltage

```cpp
// GPIO9 = ADC1_CH8, 12-bit, 12dB attenuation
// Voltage divider 2:1 → multiply reading by 2
int battery_mv = adc_voltage_reading * 2;
```

---

## 16. Flashing / Download Mode

1. **Normal method:** Hold BOOT → plug USB (or press RESET while holding BOOT) → release BOOT → flash via USB CDC serial.
2. **If stuck:** Hold BOOT → press RESET → release RESET → release BOOT → device enters download mode.
3. **Port:** `/dev/ttyACM0` (Linux), USB JTAG/serial debug unit (VID 303a, PID 1001).
4. Type-C only works as USB-CDC. No UART bridge chip — requires Win10+ for native support.

---

## 17. Accessories Included

- 1x 4P 1.25mm to 2.54mm terminal wire connector (20cm)
- 1x Type-C data/power cable (~1m)
