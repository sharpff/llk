LELINK_SRC = middleware/third_party/cloud/lelink

CFLAGS += \
	-D__LE_SDK__ \
	-DMT7687  \
	-Wno-implicit-function-declaration -Wno-comment -Wno-pointer-sign -Wno-format

# include path
CFLAGS += -I$(SOURCE_DIR)/middleware/third_party/cloud/lelink/sw
CFLAGS += -I$(SOURCE_DIR)/middleware/third_party/cloud/lelink/sw/sengine
CFLAGS += -I$(SOURCE_DIR)/middleware/third_party/cloud/lelink/sw/jsmn
CFLAGS += -I$(SOURCE_DIR)/middleware/third_party/cloud/lelink/sw/crypto
# sys path
CFLAGS += -I$(SOURCE_DIR)/kernel/rtos/FreeRTOS/Source/include
CFLAGS += -I$(SOURCE_DIR)/kernel/rtos/FreeRTOS/Source/portable/GCC/ARM_CM4F
CFLAGS += -I$(SOURCE_DIR)/kernel/service/src_core/MDK-ARM/inc
CFLAGS += -I$(SOURCE_DIR)/middleware/third_party/lwip/src/include/lwip
CFLAGS += -I$(SOURCE_DIR)/project/mt7687_hdk/apps/le_demo/inc
