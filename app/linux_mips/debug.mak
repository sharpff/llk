#Generated by VisualGDB (http://visualgdb.com)
#DO NOT EDIT THIS FILE MANUALLY UNLESS YOU ABSOLUTELY NEED TO
#USE VISUALGDB PROJECT PROPERTIES DIALOG INSTEAD

BINARYDIR := Debug
PLATFORM := linux_mips
MYXPATH := 
# MYXPREFIX := mipsel-linux-
# qca9531
MYXPREFIX := mips-openwrt-linux-


MAIN_PATH=$(CURDIR)/../..
#Toolchain
CC := $(MYXPATH)$(MYXPREFIX)gcc
CXX := $(MYXPATH)$(MYXPREFIX)g++
LD := $(CXX)
AR := $(MYXPATH)$(MYXPREFIX)ar
OBJCOPY := $(MYXPATH)$(MYXPREFIX)objcopy

#Additional flags
PREPROCESSOR_MACROS := DEBUG LINUX
INCLUDE_DIRS := $(MAIN_PATH)/sw $(MAIN_PATH)/hal/$(PLATFORM) $(MAIN_PATH)/sw/mbedtls-2.2.0_crypto/include $(MAIN_PATH)/sw/sengine include .
LIBRARY_DIRS := $(MAIN_PATH)/lib/$(BINARYDIR)-$(PLATFORM)
LIBRARY_NAMES := pthread lelink
ADDITIONAL_LINKER_INPUTS := 
MACOS_FRAMEWORKS := 
LINUX_PACKAGES := 

CFLAGS := -ggdb -ffunction-sections -O0
CXXFLAGS := -ggdb -ffunction-sections -O0
ASFLAGS := 
LDFLAGS := -Wl,-gc-sections -ldl
COMMONFLAGS := 

START_GROUP := -Wl,--start-group
END_GROUP := -Wl,--end-group

IS_SUPPORT_AIRCONFIG_CTRL := 1
#Additional options detected from testing the toolchain
IS_LINUX_PROJECT := 1
