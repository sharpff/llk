#!/bin/bash
#--ssid FAST_D908EC \
#--passwd 1234Abcd@126.com \
#    --ssid TP-LINK_lelink2 \
#    --passwd 987654321 \

MAIN_PATH="../.."
$MAIN_PATH/lelinkTool.py \
    -a 115.182.63.167 \
    -p 9002 \
    -u 10000100011000510005 \
    -k MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDCyQMwvQauI1/PbtQ2FwVTZwDcPcDXI1nEUIvqsz+tlmQzwpCWGUOwHbZF3AVw8b1zvF5nW/UU0aF8z2KKCqtm6gB4jSblbJZDUlvMhASiGnCUGg2lHf3MDtiMFOeGy2XmvBLDLZVP3uU8gDLfTfCqW+JWzTqoEBZrEK5IPQbi+wIDAQAB \
    -s F7nZeAaAoEky6vIITX5Lts3Fz2iThJH6a7iQzKGejWFTCE9nQTlsmlM8r6gQk2dEtmCBBerXK9SL17LuB6llnJjH59XebuV4nxA+1xV8XBZV5KXwLeKjTgpuTIoKpytRiJaMhkuLc1zxqt+jo+7ymAqd2fpAKRdouiy4Aoq/1n0= \
    -o ./0x1c2000.bin

$MAIN_PATH/lelinkTool.py \
    -t ./mt7687.lua \
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


