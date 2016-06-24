#!/bin/bash

#
# Create on: 2016-06-23
#
#    Author: feiguoyou@hotmail.com
#

# 使用注意事项
# 
# 1、要求在tool/目录下运行, 参数为产品相对路径, 示例: ./sdkPackTool product/ezviz
#
# 2、tool/product/$PRODUCT_NAME 下要求设置平台类型$PLATFORM, 它决定了之后查找app,hal的目录.
#
# 3、以下目录要求有sdkpack.sub.sh可执行脚本文件: tool/product/$PRODUCT_NAME/, app/$PLATFORM, hal/$PLATFORM, sw/.
#
# 4、sdkpack.sub.sh的规则示例格式如下:
#
#    PACK_FILES=$PACK_FILES:"/tool/product/test.lua@app"
#
#    4.1、$PACK_FILES 这个变量是用来存储所有需要Copy的文件, 由于它是一个变量所以要求以叠加的方式写.
#    4.2、一个文件的格式如"/tool/product/test.lua@app".
#
#         4.2.1 要是首字符是'/'表示文件的路径是工作主目录下, 否则表示文件与sdkpack.sub.sh同目录.
#         4.2.2 如果格式中有'@'字符, 表示该文件指定Copy到'@'之后的路径下, 否则表示Copy到与该文件相对应的目录.
#
# 5、如果有其它处理事务, 可以在各sdkpack.sub.sh中去完成.
#
# 6、$PRODUCT_NAME 示例: ezviz; $PLATFORM 示例: linux_arm

RM="rm -rf"
COPY="cp -Rf"
MKDIR="mkdir -p"
MAIN_PATH="`pwd`/.."
SDKPACK_SUB_SHELL="sdkpack.sub.sh"

if [ -z $1 ] || [ ! -d $1 ]; then
    echo "Command Error!"
    echo "USEAGE: $0 product/PRODUCT_NAME"
    echo "example: $0 product/ezviz"
    exit
fi

PRODUCT_NAME=$(basename $1)
echo "PRODUCT: $PRODUCT_NAME"

GLOBIGNORE="*"
declare -A PACK_FILE_DIC=()
WALK_PATH=(tool/product/$PRODUCT_NAME sw app@ hal@)
for d in ${WALK_PATH[@]} 
do 
PACK_FILES=
let idx=`expr index $d "@"`
if [ $idx -gt 0 ]; then
    NOW_PATH="${d:0:($idx - 1)}/$PLATFORM"
else
    NOW_PATH="$d"
fi
pushd $MAIN_PATH/$NOW_PATH > /dev/null 2>&1
source $SDKPACK_SUB_SHELL
popd > /dev/null 2>&1
PACK_FILE_DIC[$NOW_PATH]=$(echo $PACK_FILES | tr ':' ' ')
done

PACK_SDK_PATH=.
PACK_SDK_NAME="lelink.$PLATFORM.sdk"
echo $RM "$PACK_SDK_PATH/$PACK_SDK_NAME"
$RM "$PACK_SDK_PATH/$PACK_SDK_NAME"
for key in ${!PACK_FILE_DIC[*]}
do
    GLOBIGNORE="*"
    array=${PACK_FILE_DIC[$key]} 
    for f in ${array[@]} 
    do 
        if [ -z $f ]; then
            continue
        fi
        dst=$PACK_SDK_NAME
        let idx=`expr index $f "@"`
        if [ $idx -gt 0 ]; then
            dst=$dst/${f:$idx}
            f=${f:0:($idx - 1)}
        else
            dst=$dst/$key
        fi
        if [ "${f:0:1}" = "/" ]; then
            src=$f
        else
            src=/$key/$f
        fi
        echo Copy $src ">>" $dst
        src=$MAIN_PATH$src
        dst=$PACK_SDK_PATH/$dst
        #echo Copy $src ">>" $dst
        GLOBIGNORE=
        $MKDIR $dst
        $COPY $src $dst
    done
done

sed -e "s/PLATFORM/$PLATFORM/g" $MAIN_PATH/tool/product/README.md > $PACK_SDK_PATH/$PACK_SDK_NAME/README.md

echo done

