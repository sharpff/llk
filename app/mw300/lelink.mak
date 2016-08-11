#
# For lelink lib
#

SYS_MACROS := mw300 PF_VAL=1
# [TOBESET] means u have to set the vars, E.g. ./build3.3.sh MYXPATH=/home/lf/dev/compiler/gcc-arm-none-eabi-4_9-2015q1/bin/ MYXPREFIX=arm-none-eabi-
MYXPATH := 
MYXPREFIX := 



COM_CFLAGS := -mthumb -g -Os -ffunction-sections -ffreestanding -MMD -Wall -fno-strict-aliasing -mcpu=cortex-m4 -fdata-sections -fno-common -Wno-implicit-function-declaration -Wno-comment

IS_LINUX_PROJECT := 0

