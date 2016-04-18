#!/bin/bash
MAIN_PATH="../.."
$MAIN_PATH/lelinkTool.py \
    -a 10.154.252.130 \
    -p 5546 \
    -u 10000100051000710010 \
    -k MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDBJ0U4kFnSWA4u9/s+1zEaZK0MfWxhrSfFJcNiMgKvSMnuxrYEg1w5eZ+YvaM97zakfhH9l+Aneqz71ol+jPtvGoYARvGbQ6yFixokDZNi8LnrUr+d94xQYfA7boKbtruKiFQrXjMO+xg6oWbIesnfYPEqfZZj9ntpwooZhHKwWQIDAQAB \
    -s msgAop3Gkl4x8br2cVZZEp2bZQt5E7ERg0tUgCcg34AIsijRsfLArXDv1SrXn6lyo8SrTcGvIHkqVecfrzVwK3RfBTCGo+9PZZOx6kAIS0joBuVYnx9LK5WIUhztEh4mgK/+x8cMkzGph+K33a6xRZo6yFMZI26g1Qik8Bw0bZE= \
    -o ./0x1c2000.bin

$MAIN_PATH/lelinkTool.py \
    -t vanst86.lua \
    -o ./0x1c3000.bin

$MAIN_PATH/lelinkTool.py \
    --genpriv \
    --ssid Xiaomi_A7DD \
    --passwd 987654321 \
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

