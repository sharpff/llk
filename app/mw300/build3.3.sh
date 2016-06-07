#!/bin/bash

#
# Create on: 2016-05-27
#
#    Author: feiguoyou@hotmail.com
#

MAIN_PATH="`pwd`/../../"

if [ ! -n "$WMSDK33" ]; then
    echo "Please set SDK path first. \$WMSDK33"
    exit -1
fi

touch "$MAIN_PATH/sw/data.c"
$MAIN_PATH/tool/gitVersion $MAIN_PATH/tool/version.template.h $MAIN_PATH/sw/version.h

pushd $WMSDK33 > /dev/null 2>&1
make APP=$MAIN_PATH/app/mw300/wmsdk/external/lelink/ $*
make APP=$MAIN_PATH/app/mw300/wmsdk/external/sengine/ $*
make APP=$MAIN_PATH/app/mw300/sample_apps/le_demo/ $*
popd > /dev/null 2>&1

cp $WMSDK33/bin/mw300_defconfig/board/*.bin ../../tool