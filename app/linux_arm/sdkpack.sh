#!/bin/bash

RM="rm -f"
COPY="cp -f"
MKDIR="mkdir -p"
MAIN_PATH="`pwd`/../.."
PLATFORM="linux_arm"
PACK_SDK_NAME="lelink.$PLATFORM.sdk"
PACK_SDK_PATH="`pwd`/$PACK_SDK_NAME"

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

$MKDIR $PACK_SDK_PATH/app/$PLATFORM/
$COPY $MAIN_PATH/app/$PLATFORM/*.c $PACK_SDK_PATH/app/$PLATFORM/
$COPY $MAIN_PATH/app/$PLATFORM/*.bin $PACK_SDK_PATH/app/$PLATFORM/
$COPY $MAIN_PATH/app/$PLATFORM/*.lua $PACK_SDK_PATH/app/$PLATFORM/
$COPY $MAIN_PATH/app/$PLATFORM/debug.mak $PACK_SDK_PATH/app/$PLATFORM/
$COPY $MAIN_PATH/app/$PLATFORM/Makefile $PACK_SDK_PATH/app/$PLATFORM/
$COPY $MAIN_PATH/app/$PLATFORM/genProfile.sh $PACK_SDK_PATH/app/$PLATFORM/
do_copy $MAIN_PATH/hal/$PLATFORM/ $PACK_SDK_PATH/hal/$PLATFORM/
$MKDIR $PACK_SDK_PATH/lib/lelink/Debug-$PLATFORM/
$COPY $MAIN_PATH/lib/lelink/Debug-$PLATFORM/lib*.a $PACK_SDK_PATH/lib/lelink/Debug-$PLATFORM/
$COPY $MAIN_PATH/lib/sengine/Debug-$PLATFORM/lib*.a $PACK_SDK_PATH/lib/lelink/Debug-$PLATFORM/
$MKDIR $PACK_SDK_PATH/sw/
$COPY $MAIN_PATH/sw/airconfig.h $PACK_SDK_PATH/sw/
$COPY $MAIN_PATH/sw/io.h $PACK_SDK_PATH/sw/
$COPY $MAIN_PATH/sw/leconfig.h $PACK_SDK_PATH/sw/
$COPY $MAIN_PATH/sw/ota.h $PACK_SDK_PATH/sw/
$COPY $MAIN_PATH/sw/protocol.h $PACK_SDK_PATH/sw/
$COPY $MAIN_PATH/sw/state.h $PACK_SDK_PATH/sw/
do_copy $MAIN_PATH/sw/mbedtls-2.2.0_crypto/include/mbedtls/ $PACK_SDK_PATH/sw/mbedtls-2.2.0_crypto/include/mbedtls/
$MKDIR $PACK_SDK_PATH/tool/
$COPY $MAIN_PATH/tool/lelinkTool.py $PACK_SDK_PATH/tool/

#tar cvjf $PACK_SDK_NAME\(`date +"%Y%m%d"`\).tar.bz2 $PACK_SDK_NAME

echo done

