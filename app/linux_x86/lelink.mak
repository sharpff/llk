#
# For lelink lib
#

SYS_MACROS := LINUX PF_VAL=3
MYXPATH := 
MYXPREFIX :=
SOURCEFILES := $(MAIN_PATH)/sw/airconfig_ctrl.c
COM_CFLAGS := -ggdb -ffunction-sections -O0 -gstabs+

IS_SUPPORT_AIRCONFIG_CTRL := 1
IS_LINUX_PROJECT := 1

