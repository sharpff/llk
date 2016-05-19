RM="rm -f"
COPY="cp -prf"
MKDIR="mkdir -p"
MAIN_PATH="`pwd`/../../"
PATH_SENGINE=../../lib/sengine
PATH_LELINK=../../lib/lelink

if [ "$1" = "gdb" ]; then

	if [ ! -n "$LinuxGDB" ]; then
	    echo "Please set SDK path first. \$LinuxGDB"
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

	do_copy "$MAIN_PATH/hal/linux/" "$LinuxGDB/dev/hal/linux/"
	do_copy "$MAIN_PATH/sw/" "$LinuxGDB/dev/sw/"
	cp "$MAIN_PATH/app/linux/main.c"  "$LinuxGDB/dev/app/linux"
	cp "$MAIN_PATH/app/linux/0x1c2000.bin"  "$LinuxGDB/dev/app/linux/Debug"
	cp "$MAIN_PATH/app/linux/0x1c3000.bin"  "$LinuxGDB/dev/app/linux/Debug"
	pushd $LinuxGDB/dev/app/linux/Debug > /dev/null 2>&1
	chmod 777 * -R
	popd > /dev/null 2>&1

fi

touch "$MAIN_PATH/sw/data.c"
# $MAIN_PATH/tool/SubWCRev $MAIN_PATH $MAIN_PATH/tool/version.template.h $MAIN_PATH/sw/version.h
$MAIN_PATH/tool/gitVersion $MAIN_PATH/tool/version.template.h $MAIN_PATH/sw/version.h

pushd $PATH_SENGINE > /dev/null 2>&1
make PLATFORM="linux" $*
popd > /dev/null 2>&1

pushd $PATH_LELINK > /dev/null 2>&1
make PLATFORM="linux" $*
popd > /dev/null 2>&1

make PLATFORM="linux" $*
echo done

