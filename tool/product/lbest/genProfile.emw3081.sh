#!/bin/bash
#--ssid FAST_D908EC \
#--passwd 1234Abcd@126.com \
#    --ssid TP-LINK_lelink2 \
#    --passwd 987654321 \

MAIN_PATH="../.."
$MAIN_PATH/lelinkTool.py \
    -a iot.test.leinlife.com \
    -p 9003 \
    -u 10000100141001010012 \
    -k MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDCWJPxgQdYW/IkmVph6n1cPSreVDbJt+dl9ATFhcXWEh2+4586oRxn9ozC9IVzU06a9XND4BcIP5Ftm7kNykL9qQLssZn/3emNB7fOWDIICdest/bVZ7pIAB6QlEGKa1CteWHV8227EgUqVLjNLkh6eV95HePIwWb08CCLcXh9JwIDAQAB \
    -s Zy+gtBpUaFU1lVldO0vzEb72btQ6mBePGE0KOYscUBQZFBenW/+SB0cPXxfRHb33ifB5UMcLPNY75rwDnKdt54ICibWx87MSUxxnARYStCKvEgc2sCokoIGySGiJf28KQ+FiKHRk4VHiej/tm3noN+l/FmvmDWbX7jGFVajNHlc= \
    -o ./0x1c2000.bin

$MAIN_PATH/lelinkTool.py \
    -t lbest.lua \
    -o ./0x1c3000.bin

$MAIN_PATH/lelinkTool.py \
    --genpriv \
    --wmode sap \
    -o ./0x1c8000.bin

$MAIN_PATH/lelinkTool.py \
    ./0x1c2000.bin 0x1000 \
    ./0x1c3000.bin 0x5000 \
    ./0x1c8000.bin 0x1000 \
    -o ./cust.bin

$MAIN_PATH/lelinkTool.py \
    $MAIN_PATH/boardEMW3081/emw3081_ram_1.p.bin  0xB000 \
    $MAIN_PATH/boardEMW3081/emw3081_bootloader.bin 0x8000 \
    $MAIN_PATH/boardEMW3081/mico_app.bin 0xba800 \
    $MAIN_PATH/boardEMW3081/emw3081_ate.bin 0x112800 \
    cust.bin 0x20000 \
    -o ./lelink_3081_full.bin


