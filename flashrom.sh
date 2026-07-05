#!/bin/bash
esptool.py --chip esp32s3 --port /dev/ttyACM0 write_flash 0x100000 "$1"
