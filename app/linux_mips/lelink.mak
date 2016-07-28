#
# For lelink lib
#

SYS_MACROS := LINUX PF_VAL=3
# [TOBESET] means u have to set the vars, E.g. ./build.sh MYXPATH=/home/lf/dev/compiler/OpenWrt-Toolchain-ar71xx-for-mips_r2-gcc-4.6-linaro_uClibc-0.9.33.2/toolchain-mips_r2_gcc-4.6-linaro_uClibc-0.9.33.2/bin/ MYXPREFIX=mips-openwrt-linux-
MYXPATH := 
MYXPREFIX := mipsel-linux-

# common
# MYXPREFIX := mipsel-linux-
# qca9531
# MYXPREFIX := mips-openwrt-linux-

COM_CFLAGS := -ffunction-sections -O0 -gstabs+

IS_SUPPORT_AIRCONFIG_CTRL := 1
IS_LINUX_PROJECT := 1

