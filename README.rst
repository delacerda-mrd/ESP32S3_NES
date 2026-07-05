ESP32-NESEMU, a Nintendo Entertainment System emulator for the ESP32-S3
========================================================================

This is a port of Nofrendo, a Nintendo Entertainment System emulator, targeting the
QDtech ES3C28P (ESP32-S3R8, 2.8" ILI9341V IPS LCD, FT6336G capacitive touch, ES8311
audio codec). On-screen touch controls replace the original PSX-controller input.

Warning
-------

This is a proof-of-concept and not an official application note. As such, this code is entirely unsupported by Espressif.


Compiling
---------

This code is an ESP-IDF v5.x CMake project, targeting the esp32s3 chip::

    idf.py set-target esp32s3
    idf.py build

The build pulls in the ``espressif/es8311`` component via the IDF Component Manager.


Flashing
--------

The board has no UART bridge -- it uses the ESP32-S3's native USB-CDC, appearing as
``/dev/ttyACM0`` on Linux. Hold BOOT while plugging in USB (or while pressing RESET) to
enter download mode if needed::

    idf.py -p /dev/ttyACM0 flash monitor


Display
-------

Fixed to the ES3C28P's onboard 320x240 landscape ILI9341V IPS SPI LCD (CS=10, DC=46,
SCLK=12, MOSI=11, MISO=13, BL=45; RST is tied to the chip's own reset line).

The 256px-wide NES frame is drawn at x=48 on the 320-wide screen. The remaining space
is used for on-screen controls: a 48px D-pad bar on the left, and a 16px A/B/Start/Select
button bar on the right, both drawn once at startup and driven by the capacitive
touch panel (FT6336G, I2C address 0x38).


Audio
-----

ES8311 codec (I2C address 0x18) + I2S, driving the onboard FM8002E amplifier
(enabled via GPIO1, active low).


ROM
---
This NES emulator does not come with a ROM. Please supply your own and flash to address
0x00100000 using ``flashrom.sh <rom.nes>``.


Copyright
---------

Code in this repository is Copyright (C) 2016 Espressif Systems, licensed under the Apache License 2.0 as described in the file LICENSE. Code in the
components/nofrendo is Copyright (c) 1998-2000 Matthew Conte (matt@conte.com) and licensed under the GPLv2.
