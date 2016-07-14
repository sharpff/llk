# Copyright (C) 2008-2015 Marvell International Ltd.
# All Rights Reserved.

exec-y += le_demo

RELATIVE_PATH=../../../..
MAIN_PATH=$(d)/$(RELATIVE_PATH)

le_demo-cflags-y := \
	-D WM_IOT_PLATFORM=TRUE \
	-D HAVE_CONFIG_H \
	-D __MRVL_MW300__ \
	-D__MRVL_SDK3_3__

le_demo-cflags-y += -Wno-implicit-function-declaration -Wno-comment -Wno-pointer-sign -Wno-format

le_demo-cflags-y += \
	-I$(MAIN_PATH)/../ \
	-I$(MAIN_PATH)/sw/ \
	-I$(MAIN_PATH)/sw/sengine \
	-I$(MAIN_PATH)/hal/Marvell

le_demo-cflags-y += -llelink -Lbin/mw300_defconfig/libs/

le_demo-objs-y := \
	$(RELATIVE_PATH)/app/mw300/sample_apps/le_demo/src/app/mw300/main.c \
	$(RELATIVE_PATH)/hal/Marvell/halAES.c \
	$(RELATIVE_PATH)/hal/Marvell/halAirConfig.c \
	$(RELATIVE_PATH)/hal/Marvell/halCallback.c \
	$(RELATIVE_PATH)/hal/Marvell/halHelper.c \
	$(RELATIVE_PATH)/hal/Marvell/halMD5.c \
	$(RELATIVE_PATH)/hal/Marvell/halNetwork.c \
	$(RELATIVE_PATH)/hal/Marvell/halOS.c \
	$(RELATIVE_PATH)/hal/Marvell/halRSA.c \
	$(RELATIVE_PATH)/hal/Marvell/halConvertor.c \
	$(RELATIVE_PATH)/hal/Marvell/halIO.c \
	$(RELATIVE_PATH)/hal/Marvell/halOTA.c

le_demo-linkerscript-y := $(MAIN_PATH)/app/mw300/mw300-xip.ld
le_demo-board-y := $(MAIN_PATH)/app/mw300/sample_apps/le_demo/src/board.c

