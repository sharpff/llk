#Generated by VisualGDB (http://visualgdb.com)
#DO NOT EDIT THIS FILE MANUALLY UNLESS YOU ABSOLUTELY NEED TO
#USE VISUALGDB PROJECT PROPERTIES DIALOG INSTEAD

BINARYDIR := Debug-$(PLATFORM)


#Toolchain
#MYXPATH:=/root/feng/qca9531/OpenWrt-Toolchain-ar71xx-for-mips_r2-gcc-4.6-linaro_uClibc-0.9.33.2/toolchain-mips_r2_gcc-4.6-linaro_uClibc-0.9.33.2/bin/
#MYXPREFIX:=mips-openwrt-linux-
CC := $(MYXPATH)$(MYXPREFIX)gcc
CXX := $(MYXPATH)$(MYXPREFIX)g++
LD := $(CXX)
AR := $(MYXPATH)$(MYXPREFIX)ar
OBJCOPY := $(MYXPATH)$(MYXPREFIX)objcopy


#Additional flags
PREPROCESSOR_MACROS := DEBUG LINUX __LE_SDK__
INCLUDE_DIRS := 
LIBRARY_DIRS := 
LIBRARY_NAMES := 
ADDITIONAL_LINKER_INPUTS := 
MACOS_FRAMEWORKS := 
LINUX_PACKAGES := 

CFLAGS := -ggdb -ffunction-sections -O0
CXXFLAGS := -ggdb -ffunction-sections -O0
ASFLAGS := 
LDFLAGS := -Wl,-gc-sections
COMMONFLAGS := 

START_GROUP := -Wl,--start-group
END_GROUP := -Wl,--end-group

#Additional options detected from testing the toolchain
IS_LINUX_PROJECT := 1
