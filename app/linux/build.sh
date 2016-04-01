MAIN_PATH="`pwd`/../../"
PATH_SENGINE=../../lib/sengine
PATH_LELINK=../../lib/lelink

touch "$MAIN_PATH/sw/data.c"
$MAIN_PATH/tool/SubWCRev $MAIN_PATH $MAIN_PATH/tool/version.template.h $MAIN_PATH/sw/version.h

pushd $PATH_SENGINE > /dev/null 2>&1
make
popd > /dev/null 2>&1

pushd $PATH_LELINK > /dev/null 2>&1
make
popd > /dev/null 2>&1

make clean && make
echo done

