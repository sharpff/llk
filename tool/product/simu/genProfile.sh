#!/bin/bash
MAIN_PATH="../.."
$MAIN_PATH/lelinkTool.py \
    -a 10.154.252.130 \
    -p 5546 \
    -u d05bca44feb34aeca2dd \
    -k MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQC2KZCgOOr9hefZxOWJ8Uk2LfcsLJ5OdYFcLbUSdrho704MSfsd/75LfqwWSazHbDjoUfbNCB5SywOMpZv/ab8G8LhlgoiZf5Eh1dSdpq609jd7fgsbelPtVeag1twsrZYc3H+++fJ+l56vnE7WcMFKryiuIfnKKMQIbGTzwRf9lQIDAQAB \
    -s QtuBVU7gq27dtbwrSKuy/2IMPkCnTISfyDq+QgarLbKRra4h6IE+USdua0yfBOuvg2Hka/NhzvPIADAElhuIKLlRmxmlJm0+Ji2OSgcNrF8yJGtpi+REWhlrRcTlIRQ/Tm3okTs6Bug53o7cvwfPiNfVvKtkwIHv7XHP2d4ntEk= \
    -o ./0x1c2000.bin

$MAIN_PATH/lelinkTool.py \
    -t $MAIN_PATH/product/simu/simu.lua \
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

    # 测试
    #-a 10.154.252.130 \
    #-p 5546 \

    # 外网, 内网ip
    #-a 10.182.192.19 \
    #-p 9002 \

    # 外网
    #-a 115.182.63.167 \
    #-p 9002 \