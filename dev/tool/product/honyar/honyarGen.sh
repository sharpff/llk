#!/bin/bash
MAIN_PATH="../.."
$MAIN_PATH/lelinkTool.py \
    -a 10.204.28.134 \
    -p 9002 \
    -u 10000100101000010007 \
    -k MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCrTxpVJHzbLoxSk7CzBaeyg4Czz7rANfqZUu7bD57C4fGbmfdaMEG4VfuMxWYw08OLW/f735FwVpP89YPOcjrRW/o/p76UfIS84KZCkMkFi/2InfNTm+ep2tuOlCpq7C6TTdX04MJsTpfbERqKnfuSBRbg12OEtpmRmcTmYqjosQIDAQAB \
    -s hY+ZAdXoFBVNowMuqAWjhoUubQucdhDVpa8JzAFD6HTR7Jkz1cUvrvaMh/OFi8LgTBinHDaHpnZBvfDl95SFyeJBpLFM1p9G0m46ZVESkeGPjQJkxd5pjYFhzbpDSlBzMWGfYPlWrpgZNGeWJ5hVc8lu/lW2OSMrI0Z5xADMq3Q= \
    -o ./0x1c2000.bin

$MAIN_PATH/lelinkTool.py \
    -t $MAIN_PATH/product/honyar/honyar.lua \
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

