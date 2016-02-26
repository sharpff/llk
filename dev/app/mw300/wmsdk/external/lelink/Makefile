# Copyright (C) 2008-2014 Marvell International Ltd.
# All Rights Reserved.
BUILD_DIR=build
# SOURCE_DIR=sw
SOURCE_PORTING_DIR=$(CURDIR)/.
PROTOCOL_DIR=$(CURDIR)/sw
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
	-I$(SDK_PATH)/src/incl/libc/sys \
 	-I$(SDK_PATH)/src/incl/platform/os/freertos \
 	-I$(SDK_PATH)/src/incl/platform/net/lwip \
	-I$(SDK_PATH)/src/incl \
	-I$(SDK_PATH)/incl/sdk \
	-I$(SDK_PATH)/src/incl/sdk \
	-I$(SDK_PATH)/incl/lelink/sw \
	-I$(PROTOCOL_DIR) \
	-I$(PROTOCOL_DIR)/crypto \
	-I$(PROTOCOL_DIR)/jsmn \
	-I$(PROTOCOL_DIR)/mbedtls-2.2.0_crypto/include \
	-I$(SDK_PATH)/external/sengine \
	-I$(SDK_PATH)/external/cyassl \
	-I$(SDK_PATH)/external/cyassl/cyassl \
	-I$(SDK_PATH)/external/cyassl/cyassl/ctaocrypt \
	-I$(SDK_PATH)/src/incl/sdk/drivers \
	-I$(SDK_PATH)/src/incl/sdk/drivers/wlan \
	-I$(SDK_PATH)/src/incl/sdk/drivers/mw300 \
	-I$(SDK_PATH)/src/incl/sdk/drivers/mw300/regs \
	-I$(LWIP_SYSARCH_INC_DIR)/ \
	-I$(LWIP_INC_DIR) \
	-I$(LWIP_INC_DIR)/ipv4/ \
	-I$(LWIP_INC_DIR)/ipv6/ \
	-I$(FREERTOS_INC_DIR) \
	-I$(PROTOCOL_DIR)/3rd \
	-I$(PROTOCOL_DIR)/3rd/rsa \
	-I$(PROTOCOL_DIR)/inc \
	-I$(SOURCE_PORTING_DIR) 

CFLAGS += -DHAVE_CONFIG_H
CFLAGS += -DVANILLA_NACL
CFLAGS += -DCONFIG_WLAN_KNOWN_NETWORKS=5
CFLAGS += -D__LE_SDK__
CFLAGS += -D__MRVL_MW300__
CFLAGS += -DAPPCONFIG_DEBUG_ENABLE=1


SOURCE := ${PROTOCOL_DIR}/airconfig.c \
		${PROTOCOL_DIR}/airconfig_ctrl.c \
		${PROTOCOL_DIR}/cache.c \
		${PROTOCOL_DIR}/data.c \
		${PROTOCOL_DIR}/misc.c \
		${PROTOCOL_DIR}/network.c \
		${PROTOCOL_DIR}/ota.c \
		${PROTOCOL_DIR}/pack.c \
		${PROTOCOL_DIR}/protocol.c \
		${PROTOCOL_DIR}/state.c \
		${PROTOCOL_DIR}/utility.c \
		${PROTOCOL_DIR}/io.c \
		${PROTOCOL_DIR}/convertor.c \
		${PROTOCOL_DIR}/sengine.c \
		${PROTOCOL_DIR}/jsmn/jswrap.c \
		${PROTOCOL_DIR}/jsmn/jsmn.c \
		${PROTOCOL_DIR}/jsmn/jsgen.c \
		${PROTOCOL_DIR}/crypto/aesWrapper.c \
		${PROTOCOL_DIR}/crypto/md5Wrapper.c \
		${PROTOCOL_DIR}/crypto/rsaWrapper.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/ecjpake.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/error.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/aes.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/rsa.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/asn1write.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/version_features.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/sha1.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/memory_buffer_alloc.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/entropy_poll.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/pem.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/ripemd160.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/padlock.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/md2.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/ecp_curves.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/pkwrite.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/pk_wrap.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/havege.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/timing.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/pkparse.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/md5.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/xtea.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/asn1parse.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/blowfish.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/bignum.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/dhm.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/camellia.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/ecdsa.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/des.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/platform.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/cipher.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/md4.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/sha512.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/pkcs12.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/entropy.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/base64.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/md.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/aesni.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/ctr_drbg.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/arc4.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/md_wrap.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/oid.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/cipher_wrap.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/gcm.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/pk.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/threading.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/ecp.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/hmac_drbg.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/pkcs5.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/version.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/ecdh.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/sha256.c \
		${PROTOCOL_DIR}/mbedtls-2.2.0_crypto/library/ccm.c



OBJS=$(SOURCE:.c=.o)

liblelink.a: ${OBJS}
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
	-@rm -f liblelink.a
	-@rm -rf $(OUTDIR)
