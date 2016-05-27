# Copyright (C) 2015 Marvell International Ltd.

libs-y += libsengine

RELATIVE_PATH=../../../../..
MAIN_PATH=$(d)/$(RELATIVE_PATH)

libsengine-cflags-y := \
	-DHAVE_CONFIG_H \
	-DVANILLA_NACL \
	-DCONFIG_WLAN_KNOWN_NETWORKS=5 \
	-D__LE_SDK__ \
	-D__MRVL_MW300__ \
	-DWMSDK3_3

libsengine-cflags-y += -Wno-implicit-function-declaration -Wno-comment -Wno-pointer-sign -Wno-format

libsengine-cflags-y += \
	-I$(MAIN_PATH)/sw/sengine

libsengine-objs-y := \
	${RELATIVE_PATH}/sw/sengine/lapi.c \
	${RELATIVE_PATH}/sw/sengine/lauxlib.c \
	${RELATIVE_PATH}/sw/sengine/lbaselib.c \
	${RELATIVE_PATH}/sw/sengine/lbitlib.c \
	${RELATIVE_PATH}/sw/sengine/lcode.c \
	${RELATIVE_PATH}/sw/sengine/lcorolib.c \
	${RELATIVE_PATH}/sw/sengine/lctype.c \
	${RELATIVE_PATH}/sw/sengine/ldblib.c \
	${RELATIVE_PATH}/sw/sengine/ldebug.c \
	${RELATIVE_PATH}/sw/sengine/ldo.c \
	${RELATIVE_PATH}/sw/sengine/ldump.c \
	${RELATIVE_PATH}/sw/sengine/lfunc.c \
	${RELATIVE_PATH}/sw/sengine/lgc.c \
	${RELATIVE_PATH}/sw/sengine/linit.c \
	${RELATIVE_PATH}/sw/sengine/llex.c \
	${RELATIVE_PATH}/sw/sengine/lmem.c \
	${RELATIVE_PATH}/sw/sengine/lobject.c \
	${RELATIVE_PATH}/sw/sengine/lopcodes.c \
	${RELATIVE_PATH}/sw/sengine/lparser.c \
	${RELATIVE_PATH}/sw/sengine/lstate.c \
	${RELATIVE_PATH}/sw/sengine/lstring.c \
	${RELATIVE_PATH}/sw/sengine/lstrlib.c \
	${RELATIVE_PATH}/sw/sengine/ltable.c \
	${RELATIVE_PATH}/sw/sengine/ltablib.c \
	${RELATIVE_PATH}/sw/sengine/ltm.c \
	${RELATIVE_PATH}/sw/sengine/lua_cjson.c \
	${RELATIVE_PATH}/sw/sengine/lua_clib.c \
	${RELATIVE_PATH}/sw/sengine/lundump.c \
	${RELATIVE_PATH}/sw/sengine/lvm.c \
	${RELATIVE_PATH}/sw/sengine/lzio.c \
	${RELATIVE_PATH}/sw/sengine/strbuf.c

