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
    -a iot.test.leinlife.com \
    -p 9003 \
    -u 10000100011000010025 \
    -k MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCUVTvHWZF3K1vLqBTcKF8bP9PbT4RpAbg4d25aN26QSkqU3KN204HM72kwSWMLn0jMZ1VTS+aHyNg0L+D6vjj28YxP8uEgFv2RaJ1H9rdgzqVUNWDuXHqvUaM+NbgUN+fq5Eope54ErPp9s85SlQBAvrQPeAyYfij+3wKnp/JDYwIDAQAB \
    -s fwv5dgO+3KyZiEIEXoYwgFPYneYPo3MX0NDQUCxuQwrWre5PbXb+TArPUgvxaV4hZUICynMAEuS7+3CfQP1fX2IZ4fj/gG64QGWRKEcsKxoschOkw0KiWNNP3Tw65uVxPJU3fJUBryDCSFC2h1wIB4+AV79/DoYmdpX0Lmo12Y8= \
    -o ./0x1c2000.bin

$MAIN_PATH/lelinkTool.py \
    -t ./dingdingMT7687.lua \
    -o ./0x1c3000.bin
# $MAIN_PATH/lelinkTool.py \
#     -t $MAIN_PATH/testLua/basic.lua \
#     -o ./0x1c3000.bin

$MAIN_PATH/lelinkTool.py \
    --genpriv \
    --ssid Xiaomi_Lelink \
    --passwd 987654321 \
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

