# Copyright (C) 2008-2014 Marvell International Ltd.
# All Rights Reserved.
#
# Application Makefile
#
#    Builds and Installs Application ELF Executable Image (.axf)
#    and image suitable for flashing (.bin).
#
#    The Makefile is built using Makefile.targets (and Makefile.include)
#    available in $(TOOLCHAIN_DIR).
#
# Usage:
#
#     Targets:
#
#	  all: Builds the firmware binary image in both ELF (axf) format
#          (suitable for loading into RAM) and bin format (suitable
#          for flashing)
#
#          Should be called with SDK_PATH set to root of a pre-built
#          SDK against which the application should be built.
#
# 		   If BOARD_FILE is set (to the absolute path to the file),
#		   then the file is copied locally (to $(SRC_DIR)/board.c) and
#          then used for build. If its not present, then the application
#          sources should have the needed board-specific functions.
#
#     clean: Cleans all the build artifacts
#
#     install: Installs ELF image, Bin Image, and MAP file to
#              $(INSTALL_DIR). By default INSTALL_DIR = ./bin.
#
# Description:
#
#   Minimally, in this file, only the following need to be specified.
#
#   SRCS     = list of source files
#   DST_NAME = prefix to be used for generated build artifcats.
#
#   Default Variables:
#
#   A number of variables are used in Makefile.targets that can be
#   overridden here.
#
#   SRC_DIR:  directory for source files (default ./src)
#   OBJ_DIR:  directory for intermediate object files (default ./obj)
#   BIN_DIR:  directory for final build artifacts (default ./bin)
#
#   LDSCRIPT: Linker script (default $(TOOLCHAIN_DIR)/mc200.ld)
#   EXTRACFLAGS: pass any additional CFLAGS to be passed to the C Compiler.
#
#




DST_NAME = le_demo

EXTRACFLAGS += -DWM_IOT_PLATFORM=TRUE
EXTRACFLAGS += -DHAVE_CONFIG_H
EXTRACFLAGS += -D__MRVL_MW300__
EXTRACFLAGS += -I./src/app/mw300
EXTRACFLAGS += -I./src/hal/mw300


SRCS = ./app/mw300/main.c \
	./hal/mw300/halAES.c \
	./hal/mw300/halAirConfig.c \
	./hal/mw300/halCallback.c \
	./hal/mw300/halHelper.c \
	./hal/mw300/halMD5.c \
	./hal/mw300/halNetwork.c \
	./hal/mw300/halOS.c \
	./hal/mw300/halRSA.c \
	./hal/mw300/halConvertor.c \
	./hal/mw300/halIO.c \
	./hal/mw300/halOTA.c \
	board.c 
# SRCS = main.c \
# 	halAirConfig.c \
# 	board.c 


CFLAGS += -I$(SDK_PATH)/incl/lelink
# CFLAGS += -I$(SDK_PATH)/incl/sengine

EXTRALIBS += $(SDK_PATH)/libs/liblelink.a
# EXTRALIBS += $(SDK_PATH)/libs/libsengine.a


LDSCRIPT = $(TOOLCHAIN_DIR)/$(ARCH_NAME-y).ld
# LDSCRIPT = mw300.ld # !!! problem here !!!

include $(TOOLCHAIN_DIR)/targets.mk
include $(TOOLCHAIN_DIR)/rules.mk
