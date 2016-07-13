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
PACK_FILES=$PACK_FILES:"*.h"
PACK_FILES=$PACK_FILES:"/lib/lelink/Debug-$PLATFORM/*.a@lib/lelink/Debug-$PLATFORM"
PACK_FILES=$PACK_FILES:"/lib/sengine/Debug-$PLATFORM/*.a@lib/sengine/Debug-$PLATFORM/"

./build.sh

