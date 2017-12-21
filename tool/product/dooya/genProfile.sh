#!/bin/bash
MAIN_PATH="../.."
$MAIN_PATH/lelinkTool.py \
    -a iot.test.leinlife.com \
    -p 9003 \
    -u 10000100091000610006 \
    -k MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCWgzz4zfQCCUCEDAtBMX0TxFBwTAO51LBMoLcC86Y1SLATa+neotTsJn/kjm8i9RIMKJL52gEGzvSPJ6YNcWM0a6jDTqeOT3HFigbWHVTa09q3f64vlGbAJ5wFDJ3Mf6q2PNztJ05mtsBNC6PcvMAIIQ8YRN1bEbcCb5CyCGWVIwIDAQAB \
    -s ZxlFMdaFuGgJw3Zi3XmX6obl8j/HHN/0xMrOztaq6Nl++v9u4Sj/JR1pK6fSJCehgHeyhkkH4k0MkdjJBkspsgDS8gcZuEM/kXW5+/gFEt5Iay2QjPY4m0EemOlFHt1C0yfYpDbdAeqYlSdYtEOmMJ3tO0MFnL19SXwjtTcmZ9g= \
    -o ./0x1c2000.bin

$MAIN_PATH/lelinkTool.py \
    -t $MAIN_PATH/product/dooya/dooya.lua \
    -o ./0x1c3000.bin

$MAIN_PATH/lelinkTool.py \
    --genpriv \
    --wmode sap \
    --ssid LLGD \
    --passwd 12345678 \
    -o ./0x1c8000.bin

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
     ./cust.bin 0x7000 \
     -o ./image.bin

