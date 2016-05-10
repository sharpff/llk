#!/bin/bash
MAIN_PATH="../.."
$MAIN_PATH/lelinkTool.py \
    -a 115.182.63.167 \
    -p 9002 \
    -u 10000100111000810008 \
    -k MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCH1BhSuBWnfUGxeVHIFvZlPRXU4NtlCoi3sNi21p5igwTcI2tpqwokluxTLZKKi/DOFqmUfYKNBrVCLDotOf72qneyjR4XjhX51DvJA5RLbSGeYNqjcxFybhz0XBGCQZdski9Zu7ZnucfBytbUp1hFN4tPuvn44803LdKyTfmgXwIDAQAB \
    -s S9NlbpiG1DqvPo9ebwtv/7vTYzeUJZj/Ab5m4adbEky7hLHsjRz4I1BiQgFqUGtcI4cWS9XZp0FN1JJxnx2J0EZT8qjM4y6eGZd/WWchpeccoIbGgkKm+YMTr0IiQzMeXUgixGgitxNQnB7DBg35mlYOJ1qNjYDcedSGo1I2wt0= \
    -o ./0x1c2000.bin

$MAIN_PATH/lelinkTool.py \
    -t $MAIN_PATH/product/foree/foree.lua \
    -o ./0x1c3000.bin

$MAIN_PATH/lelinkTool.py \
    --genpriv \
    -o ./0x1c8000.bin

$MAIN_PATH/lelinkTool.py \
    ./0x1c2000.bin 0x1000 \
    ./0x1c3000.bin 0x5000 \
    ./0x1c8000.bin 0x1000 \
    -o ./cust.bin

$MAIN_PATH/lelinkTool.py \
     $MAIN_PATH/boardMW300/boot2.bin 0x4000 \
     $MAIN_PATH/boardMW300/layout.bin 0x2000 \
     $MAIN_PATH/boardMW300/psm.bin 0x4000 \
     $WMSDK/sample_apps/le_demo/bin/le_demo.bin 0x154000 \
     $MAIN_PATH/boardMW300/300_WIFI.bin 0x64000 \
     ./cust.bin 0x7000 \
     -o ./image.bin

