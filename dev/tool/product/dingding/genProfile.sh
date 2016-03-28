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
    -a 10.154.252.130 \
    -p 5546 \
    -u 10000100011000510005 \
    -k MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDCyQMwvQauI1/PbtQ2FwVTZwDcPcDXI1nEUIvqsz+tlmQzwpCWGUOwHbZF3AVw8b1zvF5nW/UU0aF8z2KKCqtm6gB4jSblbJZDUlvMhASiGnCUGg2lHf3MDtiMFOeGy2XmvBLDLZVP3uU8gDLfTfCqW+JWzTqoEBZrEK5IPQbi+wIDAQAB \
    -s F7nZeAaAoEky6vIITX5Lts3Fz2iThJH6a7iQzKGejWFTCE9nQTlsmlM8r6gQk2dEtmCBBerXK9SL17LuB6llnJjH59XebuV4nxA+1xV8XBZV5KXwLeKjTgpuTIoKpytRiJaMhkuLc1zxqt+jo+7ymAqd2fpAKRdouiy4Aoq/1n0= \
    -o ./0x1c2000.bin

$MAIN_PATH/lelinkTool.py \
    -t ./dingding.lua \
    -o ./0x1c3000.bin
# $MAIN_PATH/lelinkTool.py \
#     -t $MAIN_PATH/testLua/basic.lua \
#     -o ./0x1c3000.bin

# $MAIN_PATH/lelinkTool.py \
#     --genpriv \
#     --ssid ff \
#     --passwd fengfeng2qiqi \
#     -o ./0x1c8000.bin
$MAIN_PATH/lelinkTool.py \
    --genpriv \
    -o ./0x1c8000.bin

$MAIN_PATH/lelinkTool.py \
    -t $MAIN_PATH/product/AI/genIt/helloworld.lua \
    -o ./0x1c9000.bin

$MAIN_PATH/lelinkTool.py \
    ./0x1c2000.bin 0x1000 \
    ./0x1c3000.bin 0x5000 \
    ./0x1c8000.bin 0x1000 \
    ./0x1c9000.bin 0x5000 \
    -o ./cust.bin

$MAIN_PATH/lelinkTool.py \
    $MAIN_PATH/boardMW300/boot2.bin 0x4000 \
    $MAIN_PATH/boardMW300/layout.bin 0x2000 \
    $MAIN_PATH/boardMW300/psm.bin 0x4000 \
    $MAIN_PATH/le_demo.bin 0x154000 \
    $MAIN_PATH/boardMW300/300_WIFI.bin 0x64000 \
    ./cust.bin 0x3E000 \
    -o ./image.bin

