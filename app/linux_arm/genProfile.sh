#!/bin/bash
MAIN_PATH="../../tool"
$MAIN_PATH/lelinkTool.py \
    -a 115.182.63.167 \
    -p 9002 \
    -u 10000100091000610006 \
    -k MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCWgzz4zfQCCUCEDAtBMX0TxFBwTAO51LBMoLcC86Y1SLATa+neotTsJn/kjm8i9RIMKJL52gEGzvSPJ6YNcWM0a6jDTqeOT3HFigbWHVTa09q3f64vlGbAJ5wFDJ3Mf6q2PNztJ05mtsBNC6PcvMAIIQ8YRN1bEbcCb5CyCGWVIwIDAQAB \
    -s ZxlFMdaFuGgJw3Zi3XmX6obl8j/HHN/0xMrOztaq6Nl++v9u4Sj/JR1pK6fSJCehgHeyhkkH4k0MkdjJBkspsgDS8gcZuEM/kXW5+/gFEt5Iay2QjPY4m0EemOlFHt1C0yfYpDbdAeqYlSdYtEOmMJ3tO0MFnL19SXwjtTcmZ9g= \
    -o ./0x1c2000.bin

$MAIN_PATH/lelinkTool.py \
    -t test.lua \
    -o ./0x1c3000.bin

$MAIN_PATH/lelinkTool.py \
    --genpriv \
    --ssid lelink \
    --passwd 987654321 \
    -o ./0x1c8000.bin

