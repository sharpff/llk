# Copyright (C) 2012-2013 Marvell International Ltd.
# All Rights Reserved.

# This file is placed in the external directory that helps
# in building FreeRTOS and lwIP

export CONFIG_DEBUG_BUILD
export CONFIG_IPV6
export CONFIG_AUTOIP
export CONFIG_BONJ_CONFORMANCE
export CONFIG_ENABLE_LTO
export CONFIG_ENABLE_STACK_OVERFLOW_CHECK

STRIP=arm-none-eabi-strip
EXTERN_DIR=$(CURDIR)

include $(SDK_PATH)/.config
include $(SDK_PATH)/Makefile.vars

export ARCH-$(CONFIG_CPU_MC200) := cortex-m3
export ARCH-$(CONFIG_CPU_MW300) := cortex-m4


.PHONY: pre-sdk post-sdk clean install cmsis freertos usb usb_host btstack lwip lelink sengine cyassl evrythng libxively fw_supplicant blestack

pre-sdk: cmsis freertos usb lwip lelink sengine cyassl btstack evrythng libxively fw_supplicant blestack
	@echo "Pre-SDK done"
usb lwip lelink sengine cyassl btstack libxively evrythng fw_supplicant blestack: freertos

post-sdk: usb_host
	@echo "Post-SDK done"

clean: cmsis_clean freertos_clean usb_clean lwip_clean lelink_clean sengine_clean cyassl_clean btstack_clean evrythng_clean libxively_clean usb_host_clean fw_supplicant_clean blestack_clean

pre-sdk-install: cmsis_install freertos_install usb_install lwip_install cyassl_install evrythng_install btstack_install libxively_install fw_supplicant_install arrayent_install blestack_install lelink_install sengine_install

post-sdk-install: post_fw_supplicant_install post_blestack_install
	
lelink:
	@echo "external/lelink"
	$(MAKE) -C lelink/

sengine:
	@echo "external/sengine"
	$(MAKE) -C sengine/

cmsis:
ifeq (y, $(CONFIG_CMSIS_DSPLIB))
	@echo "external/cmsis"
	$(MAKE) $(WMSDK_PARALLEL_MAKE) -C cmsis/
endif

freertos:
	@echo "external/freertos"
	$(MAKE) $(WMSDK_PARALLEL_MAKE) -C freertos/

usb:
ifeq (y, $(CONFIG_USB_DRIVER))
	@echo "external/usb"
	$(MAKE) -C usb/
endif

usb_host:
ifeq (y, $(CONFIG_USB_DRIVER_HOST))
	@if [ -d "$(SDK_PATH)/../usb-host" ] ; then \
	echo "usb-host" ; \
	echo "$(SDK_PATH)/../usb-host" ; \
	$(MAKE) -C $(SDK_PATH)/../usb-host SDK_PATH=$(SDK_BIN_PATH); \
	fi
endif

blestack:
ifeq (y, $(CONFIG_CPU_MC200))
ifeq (y, $(CONFIG_BT_SUPPORT))
	@if [ -d "$(BLESTACK_PATH)" ] ; then \
	echo "blestack" ; \
	echo "$(BLESTACK_PATH)" ; \
	$(MAKE) -C $(BLESTACK_PATH); \
	fi
endif
endif

btstack:
	@if [ -d "$(BTSTACK_PATH)" ] ; then \
	echo "btstack" ; \
	echo "$(BTSTACK_PATH)" ; \
	$(MAKE) -C $(BTSTACK_PATH); \
	fi

lwip:
	@echo "external/lwip"
	$(MAKE) $(WMSDK_PARALLEL_MAKE) -C lwip/ sdk_artifacts

cyassl:
	$(AT)if [ -d "$(CYASSL_PATH)" ] ; then \
	echo "cyassl" ; \
	echo "$(CYASSL_PATH)" ; \
	$(MAKE) -C $(CYASSL_PATH) DEBUG_CYASSL=$(CONFIG_ENABLE_CYASSL_DEBUG); \
	$(STRIP) --strip-debug $(CYASSL_PATH)/lib/*.a; \
	fi

evrythng:
	echo "external/evrythng"; \
        $(MAKE) $(WMSDK_PARALLEL_MAKE) -C $(EVRYTHING_PATH); \


libxively: lwip
	@echo "external/libxively"
	$(MAKE) $(WMSDK_PARALLEL_MAKE) -C libxively/ -f Makefile.wmsdk all
	@echo ""

fw_supplicant:
ifeq (y, $(CONFIG_HOST_SUPP))
	@if [ -d "$(FW_SUPPLICANT_PATH)" ] ; then \
	echo "fw_supplicant" ; \
	echo "$(FW_SUPPLICANT_PATH)" ; \
	$(MAKE) -C $(FW_SUPPLICANT_PATH); \
	fi
	$(AT)mkdir -p $(EXT_FW_SUPPLICANT_PATH)
endif

lelink_clean:
	$(MAKE) -C lelink/ clean

sengine_clean:
	$(MAKE) -C sengine/ clean
	
cmsis_clean:
ifeq (y, $(CONFIG_CMSIS_DSPLIB))
	$(MAKE) -C cmsis/ clean
endif

freertos_clean:
	$(MAKE) -C freertos/ clean

usb_clean:
ifeq (y, $(CONFIG_USB_DRIVER))
	$(MAKE) -C usb/ clean
endif

usb_host_clean:
ifeq (y, $(CONFIG_USB_DRIVER_HOST))
	@if [ -d "$(SDK_PATH)/../usb-host" ] ; then \
	$(MAKE) -C $(SDK_PATH)/../usb-host clean; \
	fi
endif

blestack_clean:
ifeq (y, $(CONFIG_CPU_MC200))
ifeq (y, $(CONFIG_BT_SUPPORT))
	@if [ -d "$(BLESTACK_PATH)" ] ; then \
	$(MAKE) -C $(BLESTACK_PATH) clean; \
	fi
endif
endif

btstack_clean:
	@if [ -d "$(BTSTACK_PATH)" ] ; then \
	$(MAKE) -C $(BTSTACK_PATH) clean; \
	fi

lwip_clean:
	$(MAKE) -C lwip/ clean

evrythng_clean:
	$(MAKE) -C $(EVRYTHING_PATH) clean;


cyassl_clean:
	@if [ -d "$(CYASSL_PATH)" ] ; then \
	$(MAKE) -C $(CYASSL_PATH) clean; \
	fi

libxively_clean:
	$(MAKE) -C libxively/ -f Makefile.wmsdk clean

fw_supplicant_clean:
ifeq (y, $(CONFIG_HOST_SUPP))
	@if [ -d "$(FW_SUPPLICANT_PATH)" ] ; then \
	$(MAKE) -C $(FW_SUPPLICANT_PATH) clean; \
	rm -fr $(FW_SUPPLICANT_PATH)/include; \
	fi
endif

cmsis_install:
ifeq (y, $(CONFIG_CMSIS_DSPLIB))
	rm -fr $(INSTALL_DIR)/incl/cmsis
	rm -f $(INSTALL_DIR)/libs/libcmsis_dsp.a
	mkdir -p $(INSTALL_DIR)/incl/cmsis
	$(AT)$(COPY_CMD) -a cmsis/Include/* $(INSTALL_DIR)/incl/cmsis
	$(AT)$(COPY_CMD) -a cmsis/libcmsis_dsp.a $(INSTALL_DIR)/libs
endif

freertos_install:
# freertos
	rm -fr $(INSTALL_DIR)/incl/freertos
	$(AT)$(COPY_CMD) -a freertos/build/include $(INSTALL_DIR)/incl/freertos
	$(AT)$(COPY_CMD) -a freertos/build/libfreertos.a $(INSTALL_DIR)/libs/


usb_install:
# usb
ifeq (y, $(CONFIG_USB_DRIVER))
	rm -fr $(INSTALL_DIR)/incl/usb
	$(AT)$(COPY_CMD) -a usb/incl/ $(INSTALL_DIR)/incl/usb
	$(AT)$(COPY_CMD) -a usb/lib/libusb.a $(INSTALL_DIR)/libs/
endif

usb_host_install:
# usb_host
	@if [ -d "$(USB_HOST_PATH)" ] ; then \
	rm -fr $(USB_INSTALL_DIR)/incl; \
	$(COPY_CMD) -a $(USB_HOST_PATH)/incl/ $(USB_INSTALL_DIR)/incl; \
	$(COPY_CMD) -a $(USB_HOST_PATH)/libs/*.a $(USB_INSTALL_DIR)/libs; \
	$(STRIP) --strip-debug $(USB_INSTALL_DIR)/libs/*.a; \
	$(COPY_CMD) -a $(USB_HOST_PATH)/driver/marvell/*.c $(USB_INSTALL_DIR)/driver/marvell/; \
	$(COPY_CMD) -a $(USB_HOST_PATH)/driver/marvell/makefile.drivers $(USB_INSTALL_DIR)/Makefile; \
	fi

btstack_install:
# btstack
	@if [ -d "$(BTSTACK_PATH)" ] ; then \
	rm -fr $(INSTALL_DIR)/incl/btstack; \
	$(COPY_CMD) -a $(BTSTACK_PATH)/build/include/ $(INSTALL_DIR)/incl/btstack; \
	$(COPY_CMD) -a $(BTSTACK_PATH)/build/libbtstack.a $(INSTALL_DIR)/libs; \
	fi

evrythng_install:
# evrythng
	rm -fr $(INSTALL_DIR)/incl/evrythng; \
	mkdir $(INSTALL_DIR)/incl/evrythng; \
	$(COPY_CMD) -a $(EVRYTHING_PATH)/build/evrythng/*.h $(INSTALL_DIR)/incl/evrythng; \
	$(COPY_CMD) -a $(EVRYTHING_PATH)/build/libevrythng.a $(INSTALL_DIR)/libs/ ;

cyassl_install:
# cyassl
	$(AT)if [ -d "$(EXT_CYASSL_PATH)" ] ; then \
	rm -fr $(INSTALL_DIR)/incl/cyassl; \
	$(COPY_CMD) -a $(EXT_CYASSL_PATH)/cyassl/ $(INSTALL_DIR)/incl/cyassl; \
	$(COPY_CMD) -a $(EXT_CYASSL_PATH)/lib/libctaocrypt.a $(INSTALL_DIR)/libs; \
	$(COPY_CMD) -a $(EXT_CYASSL_PATH)/lib/libcyassl.a $(INSTALL_DIR)/libs; \
	fi

lwip_install:
# lwip
	rm -fr $(INSTALL_DIR)/incl/lwip
	$(AT)$(COPY_CMD) -a lwip/Debug/lwip/ $(INSTALL_DIR)/incl/lwip
	$(AT)$(COPY_CMD) -a lwip/Debug/liblwip.a $(INSTALL_DIR)/libs/

blestack_install:
ifeq (y, $(CONFIG_CPU_MC200))
ifeq (y, $(CONFIG_BT_SUPPORT))
	$(AT)if [ -d "$(BLESTACK_PATH)" ] ; then \
	rm -fr $(EXT_BLESTACK_PATH); \
	mkdir -p $(EXT_BLESTACK_PATH); \
	$(COPY_CMD) -a $(BLESTACK_PATH)/include/ $(EXT_BLESTACK_PATH)/incl; \
	$(COPY_CMD) -a $(BLESTACK_PATH)/libmrvllestack.a $(EXT_BLESTACK_PATH); \
	fi
endif
endif

post_blestack_install:
ifeq (y, $(CONFIG_CPU_MC200))
ifeq (y, $(CONFIG_BT_SUPPORT))
	$(AT)if [ -d "$(EXT_BLESTACK_PATH)" ] ; then \
	rm -fr $(INSTALL_DIR)/incl/mrvl_lestack; \
	$(COPY_CMD) -a $(EXT_BLESTACK_PATH)/incl/ $(INSTALL_DIR)/incl/mrvl_lestack; \
	$(COPY_CMD) -a $(EXT_BLESTACK_PATH)/libmrvllestack.a $(INSTALL_DIR)/libs; \
	fi
endif
endif

libxively_install:
	rm -fr $(INSTALL_DIR)/incl/libxively
	$(AT)mkdir $(INSTALL_DIR)/incl/libxively
	$(AT)$(COPY_CMD) -a libxively/src/libxively/*.h $(INSTALL_DIR)/incl/libxively
	$(AT)$(COPY_CMD) -a libxively/obj/libxively.a $(INSTALL_DIR)/libs/

arrayent_install:
	rm -fr $(INSTALL_DIR)/incl/arrayent
	$(AT)mkdir $(INSTALL_DIR)/incl/arrayent
	$(AT)$(COPY_CMD) -a arrayent/source/include/*.h  $(INSTALL_DIR)/incl/arrayent
	$(AT)$(COPY_CMD) -a arrayent/source/libaca.a $(INSTALL_DIR)/libs/


fw_supplicant_install:
# fw_supplicant
ifeq (y, $(CONFIG_HOST_SUPP))
	@if [ -d "$(FW_SUPPLICANT_PATH)" ] ; then \
	mkdir -p $(EXT_FW_SUPPLICANT_PATH)/incl; \
	$(COPY_CMD) -a $(FW_SUPPLICANT_PATH)/include/* $(EXT_FW_SUPPLICANT_PATH)/incl; \
	$(COPY_CMD) -a $(FW_SUPPLICANT_PATH)/lib/libfw_supplicant.a $(EXT_FW_SUPPLICANT_PATH)/; \
	fi
endif

post_fw_supplicant_install:
# post_fw_supplicant
ifeq (y, $(CONFIG_HOST_SUPP))
	@if [ -d "$(EXT_FW_SUPPLICANT_PATH)" ] ; then \
	rm -fr $(INSTALL_DIR)/incl/fw_supp; \
	$(COPY_CMD) -a $(EXT_FW_SUPPLICANT_PATH)/incl/ $(INSTALL_DIR)/incl/fw_supp; \
	$(COPY_CMD) -a $(EXT_FW_SUPPLICANT_PATH)/libfw_supplicant.a $(INSTALL_DIR)/libs; \
	fi
endif

lelink_install:
	$(AT)$(COPY_CMD) -a lelink/ $(INSTALL_DIR)/incl/lelink
	$(AT)$(COPY_CMD) -a lelink/liblelink.a $(INSTALL_DIR)/libs/

sengine_install:
	$(AT)$(COPY_CMD) -a sengine/ $(INSTALL_DIR)/incl/sengine
	$(AT)$(COPY_CMD) -a sengine/libsengine.a $(INSTALL_DIR)/libs/
