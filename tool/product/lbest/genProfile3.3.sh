#!/bin/bash
MAIN_PATH="../.."
$MAIN_PATH/lelinkTool.py \
    -a 115.182.63.167 \
    -p 9002 \
    -u 10000100141001010012 \
    -k MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDCWJPxgQdYW/IkmVph6n1cPSreVDbJt+dl9ATFhcXWEh2+4586oRxn9ozC9IVzU06a9XND4BcIP5Ftm7kNykL9qQLssZn/3emNB7fOWDIICdest/bVZ7pIAB6QlEGKa1CteWHV8227EgUqVLjNLkh6eV95HePIwWb08CCLcXh9JwIDAQAB \
    -s Zy+gtBpUaFU1lVldO0vzEb72btQ6mBePGE0KOYscUBQZFBenW/+SB0cPXxfRHb33ifB5UMcLPNY75rwDnKdt54ICibWx87MSUxxnARYStCKvEgc2sCokoIGySGiJf28KQ+FiKHRk4VHiej/tm3noN+l/FmvmDWbX7jGFVajNHlc= \
    -o ./0x1c2000.bin

$MAIN_PATH/lelinkTool.py \
    -t lbest.lua \
    -o ./0x1c3000.bin

$MAIN_PATH/lelinkTool.py \
    --genpriv \
    --ssid ff \
    --passwd fengfeng2qiqi \
    -o ./0x1c8000.bin

$MAIN_PATH/lelinkTool.py \
    ./0x1c2000.bin 0x1000 \
    ./0x1c3000.bin 0x5000 \
    ./0x1c8000.bin 0x1000 \
    -o ./cust.bin

$MAIN_PATH/lelinkTool.py \
     $MAIN_PATH/boardMW300_3.3.30/boot2.bin 0x4000 \
     $MAIN_PATH/boardMW300_3.3.30/layout.bin 0x2000 \
     $MAIN_PATH/boardMW300_3.3.30/psm.bin 0x4000 \
     $WMSDK33//bin/mw300_defconfig/board/le_demo.bin 0x154000 \
     $MAIN_PATH/boardMW300_3.3.30/300_WIFI.bin 0x64000 \
     ./cust.bin 0x7000 \
     -o ./image.bin

    # 测试
    #-a 10.154.252.130 \
    #-p 5546 \

    # 外网, 内网ip
    #-a 10.182.192.19 \
    #-p 9002 \

    # 外网
    #-a 115.182.63.167 \
    #-p 9002 \
