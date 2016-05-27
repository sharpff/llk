# Copyright (C) 2015 Marvell International Ltd.

libs-y += liblelink

RELATIVE_PATH=../../../../..
MAIN_PATH=$(d)/$(RELATIVE_PATH)

liblelink-cflags-y := \
	-DHAVE_CONFIG_H \
	-DVANILLA_NACL \
	-DCONFIG_WLAN_KNOWN_NETWORKS=5 \
	-D__LE_SDK__ \
	-D__MRVL_MW300__ \
	-DAPPCONFIG_DEBUG_ENABLE=1 \
	-DWMSDK3_3

liblelink-cflags-y += -Wno-implicit-function-declaration -Wno-comment -Wno-pointer-sign -Wno-format

liblelink-cflags-y += \
	-I$(MAIN_PATH)/sw/ \
	-I$(MAIN_PATH)/sw/crypto \
	-I$(MAIN_PATH)/sw/jsmn \
	-I$(MAIN_PATH)/sw/mbedtls-2.2.0_crypto \
	-I$(MAIN_PATH)/sw/mbedtls-2.2.0_crypto/include \
	-I$(MAIN_PATH)/sw/mbedtls-2.2.0_crypto/include/mbedtls \
	-I$(MAIN_PATH)/sw/mbedtls-2.2.0_crypto/library \
	-I$(MAIN_PATH)/sw/sengine

liblelink-objs-y := \
	$(RELATIVE_PATH)/sw/airconfig.c \
	$(RELATIVE_PATH)/sw/airconfig_ctrl.c \
	$(RELATIVE_PATH)/sw/cache.c \
	$(RELATIVE_PATH)/sw/data.c \
	$(RELATIVE_PATH)/sw/misc.c \
	$(RELATIVE_PATH)/sw/network.c \
	$(RELATIVE_PATH)/sw/pack.c \
	$(RELATIVE_PATH)/sw/protocol.c \
	$(RELATIVE_PATH)/sw/state.c \
	$(RELATIVE_PATH)/sw/utility.c \
	$(RELATIVE_PATH)/sw/io.c \
	$(RELATIVE_PATH)/sw/convertor.c \
	$(RELATIVE_PATH)/sw/sengine.c \
	$(RELATIVE_PATH)/sw/ota.c \
	$(RELATIVE_PATH)/sw/jsmn/jswrap.c \
	$(RELATIVE_PATH)/sw/jsmn/jsmn.c \
	$(RELATIVE_PATH)/sw/jsmn/jsgen.c \
	$(RELATIVE_PATH)/sw/crypto/aesWrapper.c \
	$(RELATIVE_PATH)/sw/crypto/md5Wrapper.c \
	$(RELATIVE_PATH)/sw/crypto/rsaWrapper.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/ecjpake.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/error.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/aes.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/rsa.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/asn1write.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/version_features.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/sha1.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/memory_buffer_alloc.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/entropy_poll.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/pem.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/ripemd160.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/padlock.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/md2.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/ecp_curves.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/pkwrite.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/pk_wrap.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/havege.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/timing.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/pkparse.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/md5.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/xtea.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/asn1parse.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/blowfish.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/bignum.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/dhm.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/camellia.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/ecdsa.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/des.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/platform.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/cipher.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/md4.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/sha512.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/pkcs12.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/entropy.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/base64.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/md.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/aesni.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/ctr_drbg.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/arc4.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/md_wrap.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/oid.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/cipher_wrap.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/gcm.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/pk.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/threading.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/ecp.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/hmac_drbg.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/pkcs5.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/version.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/ecdh.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/sha256.c \
	$(RELATIVE_PATH)/sw/mbedtls-2.2.0_crypto/library/ccm.c

