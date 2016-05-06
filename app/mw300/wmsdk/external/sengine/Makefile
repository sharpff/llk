# Copyright (C) 2008-2014 Marvell International Ltd.
# All Rights Reserved.
BUILD_DIR=build
SOURCE_DIR=$(CURDIR)/.
SOURCE_PORTING_DIR=$(CURDIR)/.
JOYLINK_DIR=$(SDK_PATH)/external/joylink
LWIP_INC_DIR=$(SDK_PATH)/external/lwip/src/include/
LWIP_SYSARCH_INC_DIR=$(SDK_PATH)/external/lwip/contrib/port/FreeRTOS/STR91x/

# PORT_DIR=${SOURCE_DIR}/portable/GCC/ARM_CM3
# MEMMANG_DIR=${SOURCE_DIR}/portable/MemMang

OUTDIR = Debug

ifeq ($(SDK_PATH), )
$(error You need to give SDK_PATH)
endif

ifeq ($(NOISY),1)
AT=
else
AT=@
endif


CROSS_COMPILE := arm-none-eabi


CC=$(CROSS_COMPILE)-gcc
AR=$(CROSS_COMPILE)-ar

CFLAGS=-mthumb -g -Os -ffunction-sections -ffreestanding -MMD -Wall \
	-fno-strict-aliasing -mcpu=$(ARCH-y) -fdata-sections -fno-common -Wno-implicit-function-declaration

CFLAGS += -I$(CURDIR) -I$(CURDIR)/incl

CFLAGS += -I$(SDK_PATH)/src/incl/libc \
 	-I$(SDK_PATH)/src/incl/platform/os/freertos \
 	-I$(SDK_PATH)/src/incl/platform/net/lwip \
	-I$(SDK_PATH)/src/incl \
	-I$(SDK_PATH)/incl/sdk \
	-I$(SDK_PATH)/src/incl/sdk \
	-I$(SDK_PATH)/external/cyassl \
	-I$(SDK_PATH)/external/cyassl/cyassl \
	-I$(SDK_PATH)/external/cyassl/cyassl/ctaocrypt \
	-I$(SDK_PATH)/src/incl/sdk/drivers \
	-I$(SDK_PATH)/src/incl/sdk/drivers/wlan \
	-I$(SDK_PATH)/src/incl/sdk/drivers/mw300 \
	-I$(SDK_PATH)/src/incl/sdk/drivers/mw300/regs \
	-I$(LWIP_SYSARCH_INC_DIR)/ \
	-I$(JOYLINK_DIR) \
	-I$(LWIP_INC_DIR) \
	-I$(LWIP_INC_DIR)/ipv4/ \
	-I$(LWIP_INC_DIR)/ipv6/ \
	-I$(FREERTOS_INC_DIR) \
	-I$(SOURCE_PORTING_DIR) 

CFLAGS += -DHAVE_CONFIG_H
CFLAGS += -DVANILLA_NACL
CFLAGS += -DCONFIG_WLAN_KNOWN_NETWORKS=5
CFLAGS += -D__MRVL_MW300__
CFLAGS += -D__LE_SDK__
# CFLAGS += -D__BOARDLINK__


#VPATH = . : joylink_protocol

SOURCE := ${SOURCE_DIR}/lapi.c \
		${SOURCE_DIR}/lauxlib.c \
		${SOURCE_DIR}/lbaselib.c \
		${SOURCE_DIR}/lbitlib.c \
		${SOURCE_DIR}/lcode.c \
		${SOURCE_DIR}/lcorolib.c \
		${SOURCE_DIR}/lctype.c \
		${SOURCE_DIR}/ldblib.c \
		${SOURCE_DIR}/ldebug.c \
		${SOURCE_DIR}/ldo.c \
		${SOURCE_DIR}/ldump.c \
		${SOURCE_DIR}/lfunc.c \
		${SOURCE_DIR}/lgc.c \
		${SOURCE_DIR}/linit.c \
		${SOURCE_DIR}/llex.c \
		${SOURCE_DIR}/lmem.c \
		${SOURCE_DIR}/lobject.c \
		${SOURCE_DIR}/lopcodes.c \
		${SOURCE_DIR}/lparser.c \
		${SOURCE_DIR}/lstate.c \
		${SOURCE_DIR}/lstring.c \
		${SOURCE_DIR}/lstrlib.c \
		${SOURCE_DIR}/ltable.c \
		${SOURCE_DIR}/ltablib.c \
		${SOURCE_DIR}/ltm.c \
		${SOURCE_DIR}/lua_cjson.c \
		${SOURCE_DIR}/lua_clib.c \
		${SOURCE_DIR}/lundump.c \
		${SOURCE_DIR}/lvm.c \
		${SOURCE_DIR}/lzio.c \
		${SOURCE_DIR}/strbuf.c



OBJS=$(SOURCE:.c=.o)

libsengine.a: ${OBJS}
	@echo " [ar] $@"
	@echo ""
	-@rm -f ${@}
	${AT}${AR} -cr ${@} ${^}

.c.o:
	@echo " [cc] $@"
	${AT}${CC} ${CFLAGS} -o ${@} -c ${<}




clean:
	-@rm -rf ${OBJS}
	-@rm -rf ./${BUILD_DIR}
	-@rm -f libsengine.a
	-@rm -rf $(OUTDIR)
