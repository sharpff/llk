# ./build.sh MYXPATH=/home/lf/dev/mtk/slimV3.3.1/tools/gcc/gcc-arm-none-eabi/bin/ MYXPREFIX=arm-none-eabi- && cp ~/dev/mtk/slimV3.3.1/out/mt7687_hdk/le_demo/mt7687_le_demo.bin ~/dev/mtk/slimV3.3.1/tools/PC_tool_Win/FOTA/_Load/mt7687/ && cp ../../tool/product/dingding/cust.bin ~/dev/mtk/slimV3.3.1/out/mt7687_hdk/le_demo/
RM="rm -f"
COPY="cp -prf"
MKDIR="mkdir -p"
MAIN_PATH="`pwd`/../../"
PATH_LELINK=../../lib
PF=mt7687


if [ ! -n "$MTSDK7687" ]; then
    echo "Please set SDK path first. \$MTSDK7687"
    exit -1
fi

if [ "$1" = "clean" ]; then
	MYXPATH= 
	MYXPREFIX=
fi

# if [ "$1" = "gdb" ]; then

# 	if [ ! -n "$LinuxGDB" ]; then
# 	    echo "Please set SDK path first. \$LinuxGDB"
# 	    exit -1
# 	fi

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

# 	do_copy "$MAIN_PATH/hal/linux/" "$LinuxGDB/dev/hal/linux/"
# 	do_copy "$MAIN_PATH/sw/" "$LinuxGDB/dev/sw/"
# 	cp "$MAIN_PATH/app/linux/main.c"  "$LinuxGDB/dev/app/linux"
# 	cp "$MAIN_PATH/app/linux/0x1c2000.bin"  "$LinuxGDB/dev/app/linux/Debug"
# 	cp "$MAIN_PATH/app/linux/0x1c3000.bin"  "$LinuxGDB/dev/app/linux/Debug"
# 	pushd $LinuxGDB/dev/app/linux/Debug > /dev/null 2>&1
# 	chmod 777 * -R
# 	popd > /dev/null 2>&1

# fi




touch "$MAIN_PATH/sw/data.c"
# $MAIN_PATH/tool/SubWCRev $MAIN_PATH $MAIN_PATH/tool/version.template.h $MAIN_PATH/sw/version.h
$MAIN_PATH/tool/gitVersion $MAIN_PATH/tool/version.template.h $MAIN_PATH/sw/version.h

rm ../../lib/Debug-$PF/*.a
rm $MTSDK7687/middleware/third_party/cloud/lelink/lib/*.a
pushd $PATH_LELINK > /dev/null 2>&1
make PLATFORM="$PF" $*
popd > /dev/null 2>&1

if [ "$1" != "clean" ]; then
	# for mt7687 sdk
	echo "*****************"
	$COPY ./config/chip/mt7687/chip.mk $MTSDK7687/config/chip/mt7687
	do_copy "./middleware/third_party/cloud/lelink" "$MTSDK7687/middleware/third_party/cloud/lelink"
	#$COPY main.c $MTSDK7687/project/mt7687_hdk/apps/le_demo/src
        $COPY ./project/mt7687_hdk/apps/le_demo/src/*.c $MTSDK7687/project/mt7687_hdk/apps/le_demo/src
        cp ./project/mt7687_hdk/apps/le_demo/src/main.c $MTSDK7687/project/mt7687_hdk/apps/le_demo/src
        cp ./driver/board/mt76x7_hdk/lib/libbsp.a $MTSDK7687/driver/board/mt76x7_hdk/lib
        cp ./driver/board/mt76x7_hdk/lib/libhal.a $MTSDK7687/driver/board/mt76x7_hdk/lib
        $COPY ./project/mt7687_hdk/template/download/flash_download.ini $MTSDK7687/project/mt7687_hdk/template/download
        $COPY ./project/mt7687_hdk/apps/le_demo/inc/*.h $MTSDK7687/project/mt7687_hdk/apps/le_demo/inc
        $COPY ./project/mt7687_hdk/apps/le_demo/inc/flash_map.h $MTSDK7687/project/mt7687_hdk/apps/bootloader/inc
        $COPY ./project/mt7687_hdk/apps/le_demo/GCC/feature.mk $MTSDK7687/project/mt7687_hdk/apps/le_demo/GCC
        $COPY ./project/mt7687_hdk/apps/le_demo/GCC/Makefile $MTSDK7687/project/mt7687_hdk/apps/le_demo/GCC
        $COPY ./project/mt7687_hdk/apps/le_demo/GCC/mt7687_flash.ld $MTSDK7687/project/mt7687_hdk/apps/le_demo/GCC
        $COPY ./middleware/MTK/smtcn/src/*.c $MTSDK7687/middleware/MTK/smtcn/src
        $COPY ./middleware/MTK/smtcn/inc/*.h $MTSDK7687/middleware/MTK/smtcn/inc
        $COPY ./middleware/third_party/httpclient/src/*.c   $MTSDK7687/middleware/third_party/httpclient/src/
	$COPY $MAIN_PATH/hal/$PF/*.h $MTSDK7687/project/mt7687_hdk/apps/le_demo/inc
	$COPY $MAIN_PATH/hal/$PF/*.c $MTSDK7687/project/mt7687_hdk/apps/le_demo/hal
	# for lelink
	$COPY ../../lib/Debug-$PF/*.a $MTSDK7687/middleware/third_party/cloud/lelink/lib/
	rm $MTSDK7687/out/mt7687_hdk/le_demo/*.bin
	$COPY $MAIN_PATH/sw/airconfig.h $MTSDK7687/project/mt7687_hdk/apps/le_demo/inc/sw
	$COPY $MAIN_PATH/sw/io.h $MTSDK7687/project/mt7687_hdk/apps/le_demo/inc/sw
	$COPY $MAIN_PATH/sw/leconfig.h $MTSDK7687/project/mt7687_hdk/apps/le_demo/inc/sw
	$COPY $MAIN_PATH/sw/ota.h $MTSDK7687/project/mt7687_hdk/apps/le_demo/inc/sw
	$COPY $MAIN_PATH/sw/protocol.h $MTSDK7687/project/mt7687_hdk/apps/le_demo/inc/sw
	$COPY $MAIN_PATH/sw/state.h $MTSDK7687/project/mt7687_hdk/apps/le_demo/inc/sw
	do_copy "$MAIN_PATH/sw/mbedtls-2.2.0_crypto/include/" "$MTSDK7687/project/mt7687_hdk/apps/le_demo/inc/sw/mbedtls-2.2.0_crypto/include/"
	echo "!!!!!!!!!!!!!!!!!"
	pushd $MTSDK7687 > /dev/null 2>&1
	./build.sh mt7687_hdk le_demo
	popd > /dev/null 2>&1
	$COPY $MTSDK7687/out/mt7687_hdk/le_demo/*.bin ../../tool
	$COPY $MTSDK7687/out/mt7687_hdk/le_demo/mt7687_le_demo.bin $MTSDK7687/tools/PC_tool_Win/FOTA/_Load/mt7687
else
	pushd $MTSDK7687 > /dev/null 2>&1
	./build.sh mt7687_hdk le_demo clean
	popd > /dev/null 2>&1
	rm ../../lib/Debug-$PF -Rfr
fi
echo done

