#
# For lelink lib
#

SYS_MACROS := MT7687
MYXPATH := /home/fei/Desktop/slimV3.2.0/tools/gcc/gcc-arm-none-eabi/bin/
MYXPREFIX := arm-none-eabi-

COM_CFLAGS := -mlittle-endian -mthumb -mcpu=cortex-m4
COM_CFLAGS := -fsingle-precision-constant -Wdouble-promotion -mfpu=fpv4-sp-d16 -mfloat-abi=hard
COM_CFLAGS += -ffunction-sections -fdata-sections -fno-builtin -Wno-implicit-function-declaration
COM_CFLAGS += -gdwarf-2 -Os -Wall -fno-strict-aliasing -fno-common
COM_CFLAGS += -Wall -Werror=uninitialized -Wno-error=maybe-uninitialized -Werror=return-type
COM_CFLAGS += -DPCFG_OS=2 -D_REENT_SMALL

IS_LINUX_PROJECT := 0

