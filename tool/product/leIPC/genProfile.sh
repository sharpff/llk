#!/bin/bash
MAIN_PATH="../.."
$MAIN_PATH/lelinkTool.py \
    -a iot.fineat.com \
    -p 9011 \
    -u 10000100021000010027 \
    -k MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCINbAF7K/MHs+3RsHZKXVx4clmqzTeiDCyCRMfLMIptQKVUj0dPP835kDaCkDW8WrdG0KvlU56IC5FiopH5Q4HUGKtg7gu5l0G4SMz4ggotqPDGYodloaSufEOnBkNKdaZxO1YDbnvGNyb7zDU4obJrl7Xa6wewNIRwnf5aqI5oQIDAQAB \
    -s FYFvTzINnl4aqK50DKG0Sfav8XQRCpp4G0/6NwPcbn1QGAELjQma6bfW6FoEBRMa2/oX01qRpnuS9qO9MWd5nycpiqufd7yDiitzxmkAEmmFoiDiLm2zB29xiHByVMhgPuJ+YNjfEt5+hzQt5u3It6zc5ePywtwMaod5Vh4UaTA= \
    -o ./0x1c2000.bin

$MAIN_PATH/lelinkTool.py \
    --genpriv \
    --ssid ff \
    --passwd fengfeng2qiqi \
    -o ./0x1c8000.bin



