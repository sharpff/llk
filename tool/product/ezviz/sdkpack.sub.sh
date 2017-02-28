#!/bin/bash

#
# Create on: 2016-06-23
#
#    Author: feiguoyou@hotmail.com
#

# must set it
PLATFORM="linux_arm"

echo $(pwd)
PACK_FILES=$PACK_FILES:"*.bin@app/$PLATFORM"
PACK_FILES=$PACK_FILES:"genProfile.sh@app/$PLATFORM"
PACK_FILES=$PACK_FILES:"ezviz.lua@app/$PLATFORM"
PACK_FILES=$PACK_FILES:"/tool/lelinkTool.py@tool"
PACK_FILES=$PACK_FILES:"/tool/product/fwScript.md@"

./genProfile.sh

