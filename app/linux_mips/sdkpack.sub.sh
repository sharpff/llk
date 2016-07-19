#!/bin/bash

#
# Create on: 2016-06-23
#
#    Author: feiguoyou@hotmail.com
#

echo $(pwd)
PACK_FILES=$PACK_FILES:"debug.mak"
PACK_FILES=$PACK_FILES:"Makefile"
PACK_FILES=$PACK_FILES:"*.c"
# PACK_FILES=$PACK_FILES:"*.h"
PACK_FILES=$PACK_FILES:"/lib/Debug-$PLATFORM/*.a@lib/Debug-$PLATFORM"

./build.sh MYXPATH=/home/lf/dev/compiler/OpenWrt-Toolchain-ar71xx-for-mips_r2-gcc-4.6-linaro_uClibc-0.9.33.2/toolchain-mips_r2_gcc-4.6-linaro_uClibc-0.9.33.2/bin/ MYXPREFIX=mips-openwrt-linux-
