#!/bin/bash

#
# Create on: 2016-05-27
#
#    Author: feiguoyou@hotmail.com
#


RM="rm -f"
COPY="cp -prf"
MKDIR="mkdir -p"
MAIN_PATH="`pwd`/../../"
PATH_LELINK=../../lib
PF=mw300

if [ ! -n "$WMSDK33" ]; then
    echo "Please set SDK path first. \$WMSDK33"
    exit -1
fi

if [ "$1" = "clean" ]; then
	MYXPATH= 
	MYXPREFIX=
fi
# # rm $WMSDK33/bin/mw300_defconfig/board/*.bin
# touch "$MAIN_PATH/sw/data.c"
# touch "$MAIN_PATH/app/mw300/sample_apps/le_demo/src/app/mw300/main.c"
# $MAIN_PATH/tool/gitVersion $MAIN_PATH/tool/version.template.h $MAIN_PATH/sw/version.h

# pushd $WMSDK33 > /dev/null 2>&1
# make APP=$MAIN_PATH/app/mw300/wmsdk/external/lelink/ $*
# make APP=$MAIN_PATH/app/mw300/wmsdk/external/sengine/ $*
# make APP=$MAIN_PATH/app/mw300/sample_apps/le_demo/ $*
# popd > /dev/null 2>&1
# cp $WMSDK33/bin/mw300_defconfig/board/*.bin ../../tool



touch "$MAIN_PATH/sw/data.c"
# $MAIN_PATH/tool/SubWCRev $MAIN_PATH $MAIN_PATH/tool/version.template.h $MAIN_PATH/sw/version.h
$MAIN_PATH/tool/gitVersion $MAIN_PATH/tool/version.template.h $MAIN_PATH/sw/version.h

rm ../../lib/Debug-$PF/*.a
rm $WMSDK33/bin/mw300_defconfig/libs/liblelink.a
pushd $PATH_LELINK > /dev/null 2>&1
make PLATFORM="$PF" $*
popd > /dev/null 2>&1
if [ "$1" != "clean" ]; then
	$COPY ../../lib/Debug-$PF/*.a $WMSDK33/bin/mw300_defconfig/libs/
fi

touch "$MAIN_PATH/app/mw300/sample_apps/le_demo/src/app/mw300/main.c"
pushd $WMSDK33 > /dev/null 2>&1
VAR=$*
VAR=${VAR% MYXPREFIX*}
VAR=${VAR##*MYXPATH=}
PATH=$PATH:$VAR
make APP=$MAIN_PATH/app/mw300/sample_apps/le_demo/ $*
popd > /dev/null 2>&1
if [ "$1" != "clean" ]; then
	$COPY $WMSDK33/bin/mw300_defconfig/board/*.bin ../../tool
fi
echo done

