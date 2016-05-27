#!/bin/bash

#
# Create on: 2016-05-27
#
#    Author: feiguoyou@hotmail.com
#

WMSDK="/home/fei/workspace/marvell-sdk/wmsdk_bundle-3.3.30"
MAIN_PATH="`pwd`/../../"

if [ ! -n "$WMSDK" ]; then
    echo "Please set SDK path first. \$WMSDK"
    exit -1
fi

pushd $WMSDK > /dev/null 2>&1
make APP=$MAIN_PATH/app/mw300/wmsdk/external/lelink/ $*
make APP=$MAIN_PATH/app/mw300/wmsdk/external/sengine/ $*
make APP=$MAIN_PATH/app/mw300/sample_apps/le_demo/ $*
popd > /dev/null 2>&1

