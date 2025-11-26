#!/bin/bash

HEX_DIR=./_bin/xiao_nrf52840_ble_sense
LATEST_HEX=$(find "$HEX_DIR" -type f -name '*s140*.hex' -printf '%T@ %p\n' | sort -nr | head -n1 | cut -d' ' -f2-)

if [ -z "$LATEST_HEX" ]; then
    echo "No .hex file found in $HEX_DIR"
    exit 1
fi

echo "$LATEST_HEX"

openocd -f ~/stlink_nrf52.cfg -c "init; halt; flash write_image erase \"$LATEST_HEX\" 0x26000 verify reset exit"    #set WORKAREASIZE 0 or set WORKAREASIZE 0x400 in config
