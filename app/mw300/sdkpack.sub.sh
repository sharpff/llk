#!/bin/bash

#
# Create on: 2016-06-23
#
#    Author: feiguoyou@hotmail.com
#

echo $(pwd)
PACK_FILES=$PACK_FILES:"*.c"
PACK_FILES=$PACK_FILES:"*.h"
PACK_FILES=$PACK_FILES:"/lib/Debug-$PLATFORM/*.a@lib/Debug-$PLATFORM"

./build3.3.sh MYXPATH=/home/lf/dev/compiler/gcc-arm-none-eabi-4_9-2015q1/bin/ MYXPREFIX=arm-none-eabi-

