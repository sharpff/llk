#!/bin/bash
MAIN_PATH="../../tool"
$MAIN_PATH/lelinkTool.py \
    -a 115.182.63.167 \
    -p 9002 \
    -u 10000100021000410015 \
    -k MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCPnsgdmZAPFCUU1PgJTTzetkU/dcKY7bryvvTxouo3vHtYWyr5k332E6Px7dgdiapxdbFc4oVIYc7dRV+wxdDKp7+TrWcFCPdc2jVEhiwM3QII81TtT+rWwXHtQ1G56I8ES89tlwXapgdgQhpWSDyYqzyUErJ5mkv5zD4L6yBmxQIDAQAB \
    -s OFiOU0IKcEST2+QMiH+pxEupixHegRw+QwUAiM2Pvu+/gXZ8pU3xJhKZ0DuWJvTyulN1yEpezjyE5PqidZxH292gVC/Y4z7GobeYao1ljDV2WWT932fBH4MCTuvTrMMlxSEUSDRfmzgnXZdPwaTy0dUO7xLvd+zOIh5Joodjhe4= \
    -o ./0x1c2000.bin

$MAIN_PATH/lelinkTool.py \
    -t ezviz.lua \
    -o ./0x1c3000.bin

$MAIN_PATH/lelinkTool.py \
    --genpriv \
    --ssid Letv_lelink \
    --passwd 987654321 \
    -o ./0x1c8000.bin

