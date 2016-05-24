#!/bin/bash
MAIN_PATH="../../tool"
$MAIN_PATH/lelinkTool.py \
    -a 115.182.63.167 \
    -p 9002 \
    -u 10000100161000010016 \
    -k MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCT49Dn+4WR61k1D8p2taR0yoypNcRYZR9VlL8It3W4qGXN2Ez5PzkXBzE5s4sRdjBcofM/kKYqE23J5CCMahnYd7SpiLCjsWybB9u6q79rZH9SiFnKP6Ov1y4n1gQ0MmgSRjQqI8yVJTi+OB+rvmgGIX2n5zSvKaMMP7D79R0glwIDAQAB \
    -s Kn647OdS45pwLOLbcvQGbgLTiBIZXEYF+gguWZNOYOpwj4uDTaSZ9J0Z4XT7hEfq1Jt+9b/08WMwxO6P6A4qHtJ1QdF5Ytt4yba2hVb8CoRVFeZprRMQmWOt+Mn/mevUXkpYBUh9ZTmbKGVMYSYHxExnqHIFCLCczgQS1M3WVJ0= \
    -o ./0x1c2000.bin

$MAIN_PATH/lelinkTool.py \
    -t test.lua \
    -o ./0x1c3000.bin

$MAIN_PATH/lelinkTool.py \
    --genpriv \
    --ssid lelink \
    --passwd 987654321 \
    -o ./0x1c8000.bin

