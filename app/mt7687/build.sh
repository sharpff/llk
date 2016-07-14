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

# 	function do_dir()
# 	{
# 	    local IN_DIR=$1
# 	    echo PWD:`pwd`
# 	    for file in `ls`;do
# 	        if [ -d $file ]; then
# 	            $MKDIR "$DSTDIR/$IN_DIR/$file"
# 	            pushd $file > /dev/null 2>&1
# 	            do_dir $IN_DIR/$file
# 	            popd > /dev/null 2>&1
# 	        elif [ -f $file ]; then
# 	            $RM "$DSTDIR/$IN_DIR/$file"
# 	            $COPY $file "$DSTDIR/$IN_DIR/$file"
# 	        fi
# 	    done
# 	}

# 	function do_copy()
# 	{
# 	    SRCDIR=$1
# 	    DSTDIR=$2
# 	    echo "Copy: $SRCDIR --> $DSTDIR"
# 	    pushd $SRCDIR > /dev/null 2>&1
# 	    echo SRC:`pwd`
# 	    $MKDIR $DSTDIR
# 	    do_dir ""
# 	    popd > /dev/null 2>&1
# 	}

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
	cp ../../lib/Debug-$PF/*.a $MTSDK7687/middleware/third_party/cloud/lelink/lib/
fi

rm $MTSDK7687/out/mt7687_hdk/le_demo/*.bin
pushd $MTSDK7687 > /dev/null 2>&1
./build.sh mt7687_hdk le_demo
popd > /dev/null 2>&1
if [ "$1" != "clean" ]; then
	cp $MTSDK7687/out/mt7687_hdk/le_demo/*.bin ../../tool
fi
echo done

