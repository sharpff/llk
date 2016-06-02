#!/bin/bash
MAIN_PATH="../../.."
$MAIN_PATH/tool/lelinkTool.py \
    -a 115.182.63.167 \
    -p 9002 \
    -u f1b312fd6f7b427f8306 \
    -k MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDipXUAYnbyus2/Ujk3LzMFyip18Gis/yPcoSelUYdefLONS5Bik7SPyV4s4oarUw+cKFM/rHOUORoueodJXHGcV+GtT4VedayJ+Lwc/HgAn7RLlHgFDHJOFajcpAhbeHeayHijGmkrwE1YkaeOMDciDZcPbWDf16W6J9RX7uDo5QIDAQAB \
    -s IYr2fYx+6ixg/vZ+2MtoVFY/zV44r78GimU7vjAq1Q63k2JGFIAysithT5zmkJxGotlefadpoHFMD9ak082jEOZPGLyggITvbpcNv7QqyxgenP7RnCgAemMdXIXq3pehE1ZRN0WnoxN/IaZP8fWzEUWSCZ99rMmMW3P2DzWwsuI= \
    -o $MAIN_PATH/app/android/assets/lelink/auth.cfg

    # 测试
    #-a 10.154.252.130 \
    #-p 5546 \

    # 外网, 内网ip
    #-a 10.182.192.19 \
    #-p 9002 \

    # 外网
    #-a 115.182.63.167 \
    #-p 9002 \
