#!/bin/bash
#--ssid FAST_D908EC \
#--passwd 1234Abcd@126.com \
#    --ssid TP-LINK_lelink2 \
#    --passwd 987654321 \

MAIN_PATH="../.."
$MAIN_PATH/lelinkTool.py \
    -a iot.leinlife.com \
    -p 9011 \
    -u 10000100201001310023 \
    -k MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQC/JNNhXICy5Nmq2OsPUet7jtu8ZbH0vDCG2qT/Jd3mz2+xGMu/jpjT9C2y8j73k+GiW4gPbCzjU/uKXb5gr6ax2k4KLvFRhptW/PqWVLYAz/5063jlgwJt7M5hem6qfLEq57+YRfENUBuH1Etv++2DpUy1uzr1TJCES0ubAhKp8QIDAQAB \
    -s ssE7AaVorhtJ991NGJuYdHOKTW8FhonoLkPMTw2AjRA6jAQUpHg5rq17FlkL75JmrjDNl0XTDHbS9yLLPQLATOoCByWGVvtvqs+lAWN3+Z+TxQjy1+0dJ/gUbsTLgYF+DoDGekjqO0q9nxKLW+H8B17JPHoICBMi6BcwLJysVR0= \
    -o ./0x1c2000.bin

$MAIN_PATH/lelinkTool.py \
    -t ./9188L.lua \
    -o ./0x1c3000.bin

$MAIN_PATH/lelinkTool.py \
    --genpriv \
    --ssid TP-LINK_lelink2 \
    --passwd 987654321 \
    -o ./0x1c8000.bin

$MAIN_PATH/lelinkTool.py \
    ./0x1c2000.bin 0x1000 \
    ./0x1c3000.bin 0x5000 \
    ./0x1c8000.bin 0x1000 \
    -o ./cust.bin

$MAIN_PATH/lelinkTool.py \
    $MAIN_PATH/boardEMW3081/emw3081_ram_1.p.bin  0xB000 \
    $MAIN_PATH/boardEMW3081/emw3081_bootloader.bin 0x8000 \
    $MAIN_PATH/boardEMW3081/mico_app.bin 0xba800 \
    $MAIN_PATH/boardEMW3081/emw3081_ate.bin 0x112800 \
    ./cust.bin 0x20000 \
    -o ./lelink_3081_full.bin
