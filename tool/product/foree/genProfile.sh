#!/bin/bash
MAIN_PATH="../.."
$MAIN_PATH/lelinkTool.py \
    -a 10.154.252.130 \
    -p 5546 \
    -u 10000100131000910011 \
    -k MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCG4t2/2PLzIaiX06dOVH6eZsMcX8PyodprdawPvHo0QtISX88UAR4COTzbXi2ypf4uyY1/UP27co/ZE09mXA7bqwrPBRKpJDYJWMi3bSIOvLynvRC2WcHJ3g1YS8uLP9yxR0yuA0YdgbFJOzvokhOuGxcALAY7AnGMjkPah2QJzwIDAQAB \
    -s ZCJsiQsjcukvZdCFDD78LrH1vTS8LnQc1cbfdnPWeiEuKdofRq8Q4KZ0w7Y33vCwHC9iiVpi0ZavtyIlB/ObezFcj5SeIalZKIukL/BRlNqx0HRDbKvikIdQeWsiD5ui4h/M4uh7FYXRUfJx6xrFsYKuGqmgN/MG0S4KyDXjx1I= \
    -o ./0x1c2000.bin

$MAIN_PATH/lelinkTool.py \
    -t $MAIN_PATH/product/foree/foree.lua \
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
     $MAIN_PATH/boardMW300/boot2.bin 0x4000 \
     $MAIN_PATH/boardMW300/layout.bin 0x2000 \
     $MAIN_PATH/boardMW300/psm.bin 0x4000 \
     $WMSDK/sample_apps/le_demo/bin/le_demo.bin 0x154000 \
     $MAIN_PATH/boardMW300/300_WIFI.bin 0x64000 \
     ./cust.bin 0x7000 \
     -o ./image.bin

