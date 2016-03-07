#!/bin/bash

function CreateJniHeaderFileForBuildLib()
{
    command -v "javac" > /dev/null && JAVAC=1 || JAVAC=0
    command -v "javah" > /dev/null && JAVAH=1 || JAVAH=0
    if [ $# -ne 3 ]; then
        echo "CreateJniHeaderFileForBuildLib Must have 3 args!"
        return 0
    fi
    BASE_PATH=$1
    JAVA_FILE_DIR=$2
    JAVA_FILE_NAME=$3
    cd $BASE_PATH
    JAVA_FILE_PATH=$JAVA_FILE_DIR/$JAVA_FILE_NAME.java
    CLASS_FILE_PATH=$JAVA_FILE_DIR/$JAVA_FILE_NAME.class
    CLASS_FILE_NAME=$(echo $JAVA_FILE_DIR/$JAVA_FILE_NAME | tr -t '/' '.')
    CPPH_FILE_NAME=$(echo $CLASS_FILE_NAME | tr -t '.' '_').h
    JNIH_FILE_NAME=jni$JAVA_FILE_NAME.h
    if [ $JAVAC = 1 -a $JAVAH = 1 ]; then
        if [ -f  ]; then
            javac $JAVA_FILE_PATH
            if [ -f $CLASS_FILE_PATH ]; then
                javah $CLASS_FILE_NAME
                rm -rf $JAVA_FILE_DIR/*.class
                if [ -f $CPPH_FILE_NAME ]; then
                    mv $CPPH_FILE_NAME $JNIH_FILE_NAME
                    rm -f *_*_*.h
                fi
            fi
        fi
    fi

    IRET=1
    if [ ! -f $JNIH_FILE_NAME ]; then
        echo "Faild to create Jni file: $JNIH_FILE_NAME"
        IRET=0
    fi
    cd -
    return $IRET
}

CreateJniHeaderFileForBuildLib . com/letv/lelink LeLink
ndk-build NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=./Android.mk NDK_APPLICATION_MK=./Application.mk $*
