#!/bin/bash

#WMSDK="/home/fei/wmsdk_bundle-3.2.12"
RM="rm -f"
COPY="cp -prf"
MKDIR="mkdir -p"
RENAME="mv -f"
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

# version
touch "$MAIN_PATH/sw/data.c"
# $MAIN_PATH/tool/SubWCRev $MAIN_PATH $MAIN_PATH/tool/version.template.h $MAIN_PATH/sw/version.h
$MAIN_PATH/tool/gitVersion $MAIN_PATH/tool/version.template.h $MAIN_PATH/sw/version.h

# ld
LDFILE="mw300-xip.ld"
LDFILE_BK="mw300-xip.ld.bk"
if [ ! -f "$WMSDK/sample_apps/toolchains/gnu/$LDFILE_BK" ]; then
    $RENAME "$WMSDK/sample_apps/toolchains/gnu/$LDFILE" "$WMSDK/sample_apps/toolchains/gnu/$LDFILE_BK"
fi
$COPY "$MAIN_PATH/app/mw300/$LDFILE" "$WMSDK/sample_apps/toolchains/gnu/$LDFILE"

# source
do_copy "$MAIN_PATH/app/mw300"  $WMSDK
do_copy "$MAIN_PATH/hal/mw300/" "$WMSDK/sample_apps/le_demo/src/hal/mw300/"
do_copy "$MAIN_PATH/sw/" "$WMSDK/wmsdk/external/lelink/sw/"
do_copy "$MAIN_PATH/sw/sengine/" "$WMSDK/wmsdk/external/sengine"

# obj dir
$MKDIR $WMSDK/sample_apps/le_demo/obj/app/mw300
$MKDIR $WMSDK/sample_apps/le_demo/obj/hal/mw300

# make -C $WMSDK clean
make -C $WMSDK XIP=1 APPS=le_demo $*
$RM $WMSDK/build.sh
$COPY $WMSDK/bin/mw300_rd/le_demo.bin ../../tool
