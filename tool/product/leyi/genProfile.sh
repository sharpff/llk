#!/bin/bash
MAIN_PATH="../.."
$MAIN_PATH/lelinkTool.py \
    -a 115.182.63.167 \
    -p 9002 \
    -u 10000100061000010013 \
    -k MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCBipSIN10kUetuRuJ4lXWWYXCFTk1qG+0FWyaW1Z49ZaAtK/SZn44TXmGs++YFHdBX0edUn08yZZdtVxMNKhHdLkmrguzroXt5/VNeNikucd9kmJQ/rfvqrK6Fh7M5Er/pTBmyKavWS4jqG3pd56lIryF6qUOmSHP9t8jU0SdMwwIDAQAB \
    -s F4L+uhM0bvmSPTv4U4oF0u+f6w9lJElQBMmQn0ixDINyf66FTRX1+Ap5Py8I37eHMuw50/BU32BKOpbJkTW0dkocw6G8amNqEP8a9hMzaPPuNAOaaIgaNmMV8SXOiq1KjjCI79j7rIPxyqfaD5ayo63wAU2scgycC1QvAb/kqRo= \
    -o ./0x1c2000.bin

$MAIN_PATH/lelinkTool.py \
    -t $MAIN_PATH/product/lexiaobao/lexiaobao.lua \
    -o ./0x1c3000.bin

$MAIN_PATH/lelinkTool.py \
    --genpriv \
    -o ./0x1c8000.bin

