#!/bin/bash

#
# Create on: 2016-06-23
#
#    Author: feiguoyou@hotmail.com
#

# must set it
PLATFORM="mw300"

echo $(pwd)
PACK_FILES=$PACK_FILES:"*.bin@app/$PLATFORM"
PACK_FILES=$PACK_FILES:"genProfile.sh@app/$PLATFORM"
PACK_FILES=$PACK_FILES:"/tool/lelinkTool.py@tool"
PACK_FILES=$PACK_FILES:"/tool/product/fwScript.md@"
PACK_FILES=$PACK_FILES:"/tool/product/test.lua@app/$PLATFORM"
PACK_FILES=$PACK_FILES:"kfr.lua@app/$PLATFORM"

./genProfile.sh

