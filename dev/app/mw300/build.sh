#!/bin/bash

#WMSDK="/home/fei/wmsdk_bundle-3.2.12"
RM="rm -f"
COPY="cp -prf"
MKDIR="mkdir -p"
MAIN_PATH="`pwd`/../../"

if [ ! -n "$WMSDK" ]; then
    echo "Please set SDK path first. \$WMSDK"
    exit -1
fi

function do_dir()
{
    local IN_DIR=$1
    echo PWD:`pwd`
    for file in `ls`;do
        if [ -d $file ]; then
            $MKDIR "$DSTDIR/$IN_DIR/$file"
            pushd $file > /dev/null 2>&1
            do_dir $IN_DIR/$file
            popd > /dev/null 2>&1
        elif [ -f $file ]; then
            $RM "$DSTDIR/$IN_DIR/$file"
            $COPY $file "$DSTDIR/$IN_DIR/$file"
        fi
    done
}

function do_copy()
{
    SRCDIR=$1
    DSTDIR=$2
    echo "Copy: $SRCDIR --> $DSTDIR"
    pushd $SRCDIR > /dev/null 2>&1
    echo SRC:`pwd`
    $MKDIR $DSTDIR
    do_dir ""
    popd > /dev/null 2>&1
}


touch "$MAIN_PATH/sw/data.c"
$MAIN_PATH/tool/SubWCRev $MAIN_PATH $MAIN_PATH/tool/version.template.h $MAIN_PATH/sw/version.h

do_copy "$MAIN_PATH/app/mw300"  $WMSDK
do_copy "$MAIN_PATH/hal/Marvell/" "$WMSDK/sample_apps/le_demo/src/hal/Marvell/"
do_copy "$MAIN_PATH/sw/" "$WMSDK/wmsdk/external/lelink/sw/"
do_copy "$MAIN_PATH/sw/sengine/" "$WMSDK/wmsdk/external/sengine"

$MKDIR $WMSDK/sample_apps/le_demo/obj/app/mw300
$MKDIR $WMSDK/sample_apps/le_demo/obj/hal/Marvell

make -C $WMSDK XIP=1 APPS=le_demo $*
