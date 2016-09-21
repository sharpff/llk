#!/bin/bash
# MAIN_PATH="../.."
# $MAIN_PATH/lelinkTool.py \
#     -a 10.154.252.130 \
#     -p 5546 \
#     -u a2077ecbcd0e4684a247 \
#     -k MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCuVNstUYV2RLwLG7ec0xgv3Mk/vPRZ9ph3NmyYP0flDzPpAVOHSYL9e9kiG1dO3e2QIZxcWcLjcDnYVyzhrW04covZiSwWS9vENHHFh5wOagkOyjSImp8PxgzOYlaP94/J1bI+UphNrQSo+OWYQKSdm4HUvVNcpMvYruXtWkOdmQIDAQAB \
#     -s R03bQyHT5tPsxqwPfFUeALfVaUtqQEX99dwnWFei9O1DWuA+A5j7alPThx5CNMT+aEIcfYIGSQVOJcmc7waQSkNzTcnzmpxWgZo8FXsrsQ3VrC61+FO5VVxZi7g20u1qfxd2Df6ulwZEC9rHC1NYRQd6EZQUoZz3j+1ZRGJRYCc= \
#     -o ./0x1c2000.bin

MAIN_PATH="../.."
$MAIN_PATH/lelinkTool.py \
    -a iot.fineat.com \
    -p 9011 \
    -u 10000100011000010022 \
    -k MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCEW7N+AnWIkhGMGUfLFBdMd3txwA2D7jSeDq8xpuw80ru8XPFxM1Zc3fNy9Ps+F853uadfNr8JMZyr1lxbLmldTug9IusZVqZzqDYMbiVpjxFtp2WpXuQgiSJ14TuF/N4eGsgcB0KikXWum+ZlOtfCgpWjSKDsJgL2O95/UBwr1wIDAQAB \
    -s asFxYWh+q61Fii5AhPOG3NSiCe5FzZcfeWho0qj0uq8y9oLIrWmXMT/xEX92eVqaH2x8pKepgyhzdNaqa27ugfijEm7i6Awmmrd0KOjwhAxSXwWmPYFb7RVgufEqSRKIETLG/FysJoIkAb6Rs70AdGz7oUYhxjRLt5Bxwhh3cT0= \
    -o ./0x1c2000.bin

$MAIN_PATH/lelinkTool.py \
    -t ./dingdingMT7687.lua \
    -o ./0x1c3000.bin
# $MAIN_PATH/lelinkTool.py \
#     -t $MAIN_PATH/testLua/basic.lua \
#     -o ./0x1c3000.bin

$MAIN_PATH/lelinkTool.py \
    --genpriv \
    --ssid ff \
    --passwd fengfeng2qiqi \
    -o ./0x1c8000.bin

$MAIN_PATH/lelinkTool.py \
    -t $MAIN_PATH/product/AI/genIt/helloworld.lua \
    -o ./0x1c9000.bin

$MAIN_PATH/lelinkTool.py \
    ./0x1c2000.bin 0x1000 \
    ./0x1c3000.bin 0x5000 \
    ./0x1c8000.bin 0x1000 \
    -o ./cust.bin

$MAIN_PATH/lelinkTool.py \
    $MAIN_PATH/boardMW300/boot2.bin 0x4000 \
    $MAIN_PATH/boardMW300/layout.bin 0x2000 \
    $MAIN_PATH/boardMW300/psm.bin 0x4000 \
    $MAIN_PATH/le_demo.bin 0x154000 \
    $MAIN_PATH/boardMW300/300_WIFI.bin 0x64000 \
    ./cust.bin 0x3E000 \
    -o ./image.bin

