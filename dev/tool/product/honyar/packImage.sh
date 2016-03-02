#!/bin/bash

cp -rf "$WMSDK/bin/mw300_rd/le_demo.bin" ./
python ../../packit.py --pack ../../boardMW300/300_HEAD.bin ../../boardMW300/boot2.bin 0x4000 ../../boardMW300/layout.bin 0x2000 ../../boardMW300/psm.bin 0x4000
python ../../packit.py --pack ./image.bin ../../boardMW300/300_HEAD.bin 0xA000 ./le_demo.bin 0x154000 ../../boardMW300/300_WIFI.bin 0x64000 ./0x1c2000.bin 0x1000 ./0x1c3000.bin 0x5000 ./0x1c8000.bin 0x1000 ../../boardMW300/cust.bin 0x37000
