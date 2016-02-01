LOCAL_PATH := $(call my-dir)

SW_FILES := airconfig.c airconfig_ctrl.c cache.c convertor.c data.c io.c misc.c \
network.c pack.c protocol.c state.c utility.c
SW_FILES := $(addprefix ../../sw/, $(SW_FILES))
SW_CRYPTO_FILES := $(wildcard $(LOCAL_PATH)/../../sw/crypto/*.c)
SW_CRYPTO_FILES := $(SW_CRYPTO_FILES:$(LOCAL_PATH)/%=%)
SW_JSMN_FILES := $(wildcard $(LOCAL_PATH)/../../sw/jsmn/*.c)
SW_JSMN_FILES := $(SW_JSMN_FILES:$(LOCAL_PATH)/%=%)
SW_MBEDTLS_FILES := $(wildcard $(LOCAL_PATH)/../../sw/mbedtls-2.2.0_crypto/library/*.c)
SW_MBEDTLS_FILES := $(SW_MBEDTLS_FILES:$(LOCAL_PATH)/%=%)
JSON_FILES := $(wildcard $(LOCAL_PATH)/jsoncpp/*.cpp)
JSON_FILES := $(JSON_FILES:$(LOCAL_PATH)/%=%)
JNI_FILES := $(wildcard $(LOCAL_PATH)/*.c) $(wildcard $(LOCAL_PATH)/*.cpp)
JNI_FILES := $(JNI_FILES:$(LOCAL_PATH)/%=%)

ALL_SRC_FILES := $(SW_FILES) $(SW_CRYPTO_FILES) $(SW_JSMN_FILES) $(SW_MBEDTLS_FILES) $(JSON_FILES) $(JNI_FILES)

ALL_SRC_INCLUDES := \
	$(LOCAL_PATH)/ \
	$(LOCAL_PATH)/jsoncpp/json/ \
	$(LOCAL_PATH)/../../sw/ \
	$(LOCAL_PATH)/../../sw/crypto \
	$(LOCAL_PATH)/../../sw/jsmn \
	$(LOCAL_PATH)/../../sw/mbedtls-2.2.0_crypto/include
	
include $(CLEAR_VARS)
LOCAL_SRC_FILES := $(ALL_SRC_FILES)
LOCAL_C_INCLUDES :=$(ALL_SRC_INCLUDES) 
LOCAL_CFLAGS := -O2 -fexceptions -D__STDC_CONSTANT_MACROS=1 -D__LE_SDK__
LOCAL_LDLIBS := -llog
LOCAL_MODULE := liblelink
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)