#!/bin/bash
MAIN_PATH="../.."
$MAIN_PATH/lelinkTool.py \
    -a iot.fineat.com \
    -p 9011 \
    -u 10000100131000010011 \
    -k MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCSdNYdqzMcZGsouJHNPDIH4Xy3qMojTn4cnQobMLdXJJTycrIOJnR1GJJQEJ7j7ZcJSH2TMdWpzoxQvIdBmNigxJ1A3fYngtT6bUSqt5lLjk6l9cqPeVMlybVJpUvR/axWwm0i1mdKwfRf2FoC4V12mDBqjem69D2jPlNiBahnPQIDAQAB \
    -s ULyp6Ki3cGiQsHT/7lTPzF7M213oKitlcyIO7dU46E0aA9/OQgPaJ9CRkEBOVpL5weFG6zAnxWq0UtZb6XDDDfagbyYbvS0mmdDJDhxKkRweuz5tSKpKx/wST3gFbAThbsIwXKQ2k16ZJx59AnHTRtNudxDylHCYicCrCz4tWIY= \
    -o ./0x1c2000.bin

$MAIN_PATH/lelinkTool.py \
    -t $MAIN_PATH/product/nxp/nxp.lua \
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

# TODO: master should be mt7687
$MAIN_PATH/lelinkTool.py \
     $MAIN_PATH/boardMW300/boot2.bin 0x4000 \
     $MAIN_PATH/boardMW300/layout.bin 0x2000 \
     $MAIN_PATH/boardMW300/psm.bin 0x4000 \
     $WMSDK/sample_apps/le_demo/bin/le_demo.bin 0x154000 \
     $MAIN_PATH/boardMW300/300_WIFI.bin 0x64000 \
     ./cust.bin 0x7000 \
     -o ./image.bin

