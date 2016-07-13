#Generated by VisualGDB (http://visualgdb.com)
#DO NOT EDIT THIS FILE MANUALLY UNLESS YOU ABSOLUTELY NEED TO
#USE VISUALGDB PROJECT PROPERTIES DIALOG INSTEAD

BINARYDIR := Debug-$(PLATFORM)
MAIN_PATH=$(CURDIR)/..

ADDITIONAL_MAKE_FILES := $(MAIN_PATH)/app/$(PLATFORM)/lelink.mak
include $(ADDITIONAL_MAKE_FILES)

#Toolchain
CC := $(MYXPATH)$(MYXPREFIX)gcc
CXX := $(MYXPATH)$(MYXPREFIX)g++
LD := $(CXX)
AR := $(MYXPATH)$(MYXPREFIX)ar
OBJCOPY := $(MYXPATH)$(MYXPREFIX)objcopy

#Additional flags
PREPROCESSOR_MACROS := DEBUG $(SYS_MACROS) __LE_SDK__
INCLUDE_DIRS := $(MAIN_PATH)/sw $(MAIN_PATH)/hal/$(PLATFORM) $(MAIN_PATH)/sw/mbedtls-2.2.0_crypto/include $(MAIN_PATH)/sw/jsmn $(MAIN_PATH)/sw/crypto $(MAIN_PATH)/sw/sengine
LIBRARY_DIRS := 
LIBRARY_NAMES := 
ADDITIONAL_LINKER_INPUTS := 
MACOS_FRAMEWORKS := 
LINUX_PACKAGES := 

CFLAGS := $(COM_CFLAGS) 
CXXFLAGS := $(COM_CFLAGS)
ASFLAGS := 
LDFLAGS := -Wl,-gc-sections
COMMONFLAGS := 

START_GROUP := -Wl,--start-group
END_GROUP := -Wl,--end-group

