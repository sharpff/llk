#!/bin/bash
MAIN_PATH="../.."

# 配置服务器及设备认证
$MAIN_PATH/lelinkTool.py \
    -a 10.154.252.130 \
    -p 5546 \
    -u 10000100111000810008 \
    -k MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCH1BhSuBWnfUGxeVHIFvZlPRXU4NtlCoi3sNi21p5igwTcI2tpqwokluxTLZKKi/DOFqmUfYKNBrVCLDotOf72qneyjR4XjhX51DvJA5RLbSGeYNqjcxFybhz0XBGCQZdski9Zu7ZnucfBytbUp1hFN4tPuvn44803LdKyTfmgXwIDAQAB \
    -s S9NlbpiG1DqvPo9ebwtv/7vTYzeUJZj/Ab5m4adbEky7hLHsjRz4I1BiQgFqUGtcI4cWS9XZp0FN1JJxnx2J0EZT8qjM4y6eGZd/WWchpeccoIbGgkKm+YMTr0IiQzMeXUgixGgitxNQnB7DBg35mlYOJ1qNjYDcedSGo1I2wt0= \
    -o ./0x1c2000.bin

# 配置固件脚本
$MAIN_PATH/lelinkTool.py \
    -t ./kfr.lua \
    -o ./0x1c3000.bin

# 配置wifi网络
$MAIN_PATH/lelinkTool.py \
    --genpriv \
    --ssid ff \
    --passwd fengfeng2qiqi \
    -o ./0x1c8000.bin

# 合并配置数据
$MAIN_PATH/lelinkTool.py \
    ./0x1c2000.bin 0x1000 \
    ./0x1c3000.bin 0x5000 \
    ./0x1c8000.bin 0x1000 \
    -o ./cust.bin

# 固件与配置数据合并生成最终的image.bin
#cp $MAIN_PATH/../../sample_apps/le_demo/bin/le_demo.bin ./ -f
$MAIN_PATH/lelinkTool.py \
    $MAIN_PATH/boardMW300/boot2.bin 0x4000 \
    $MAIN_PATH/boardMW300/layout.bin 0x2000 \
    $MAIN_PATH/boardMW300/psm.bin 0x4000 \
    $MAIN_PATH/le_demo.bin 0x154000 \
    $MAIN_PATH/boardMW300/300_WIFI.bin 0x64000 \
    ./cust.bin 0x7000 \
    -o ./image.bin
