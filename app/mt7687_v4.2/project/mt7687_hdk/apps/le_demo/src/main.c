/* Copyright Statement:
 *
 * (C) 2005-2016  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
 * if you have agreed to and been bound by the applicable license agreement with
 * MediaTek ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "flash_map.h"
#include "halHeader.h"
#include "airconfig.h"
#include "protocol.h"
#include "state.h"
#include "io.h"
#include "ota.h"
#include "task.h"
#include "sys_init.h"
#include "wifi_nvdm_config.h"
#include "wifi_lwip_helper.h"
#if defined(MTK_MINICLI_ENABLE)
#include "cli_def.h"
#endif
#ifdef MTK_HOMEKIT_ENABLE
#include "homekit_init.h"
#endif

#include "bsp_gpio_ept_config.h"
#include "hal_sleep_manager.h"

#if configUSE_TICKLESS_IDLE == 2
extern void tickless_init(void);
#endif
#include "connsys_profile.h"
#include "wifi_api.h"

#ifdef MTK_WIFI_CONFIGURE_FREE_ENABLE
#include "wifi_profile.h"
#include "smt_conn.h"
#endif

#include "smt_api.h"

// #include "network_init.h"
#include "cli_def.h"
#include "CoOTA.h"

int gin_airconfig_channel_cared[MAX_CHANNEL_CARE];
static TimerHandle_t lelink_airconfig_timer = NULL;
extern void smtcn_evt_handler(wifi_smart_connection_event_t event, void *data);
extern void aes_task_init();
extern void lelinkPltCtrlProcess(void);

int airconfig_start(void *pc, uint8_t *prov_key, int prov_key_len);
int airconfig_stop();

int8_t gin_airconfig_ap_connected;
int8_t gin_airconfig_sniffer_got;

#define AIRCONFIG_TIMEOUT (60*60*1000/portTICK_PERIOD_MS)

int salResetConfigData(void) {
    leLedReset();
    return 0;
}

int aalUserRead(userHandler_t* handler, uint8_t *buf, uint32_t len) {
    return leLedRead(buf, len);
}

int aalUserWrite(userHandler_t* handler, const uint8_t *buf, uint32_t len) {
    return leLedWrite(buf, len);
}

void aalSetLedStatus(RLED_STATE_t st) {
    if (st == RLED_STATE_WIFI) {
        leLedBlueSlowBlink(); // TODO: discuss (modified)
    } else if (st == RLED_STATE_CONNECTING) {
        leLedBlueFastBlink();
    } else if (st == RLED_STATE_RUNNING) {
        leLedSetDefault();
    }
}

static void lelink_airconfig_timeout_timer_callback( TimerHandle_t tmr ) {
    if (tmr) {
        xTimerStop(tmr, 0);
        xTimerDelete(tmr, portMAX_DELAY);
        lelink_airconfig_timer = NULL;
    }
    airconfig_stop();
    APPLOG("lelink_airconfig_timeout_timer_callback(). \r\n");
}

static void mtk_thread_airconfig_proc(void *args) { 
    if(wifi_smart_connection_init(NULL, 0, smtcn_evt_handler) < 0){
        return;
    }
    wifi_smart_connection_start(0);
    vTaskDelete(NULL);
}

int airconfig_start(void *ptr, uint8_t *prov_key, int prov_key_len) {
    lelink_airconfig_timer = xTimerCreate( "lelink_airconfig_timer",
                                       AIRCONFIG_TIMEOUT,
                                       pdFALSE,
                                       NULL,
                                       lelink_airconfig_timeout_timer_callback);
    if (lelink_airconfig_timer != NULL) {
        xTimerStart(lelink_airconfig_timer, 0);
    }
    
    if (pdPASS != xTaskCreate(mtk_thread_airconfig_proc,
                              "thread_lelink_proc",
                              1024*2,  //2K stack size;
                              NULL,
                              1,
                              NULL)) {
        LOG_E(common, "create user task fail");
        return 0;
    }
    
    gin_airconfig_sniffer_got = 0;
    return 1;
}

int airconfig_stop() {
    wifi_smart_connection_stop();
    //wifi_smart_connection_deinit();
    airconfig_reset();
    airhug_reset();
    return 0;
}

void printForFac(void) {
    uint8_t mac[6] = {0};
    halGetMac(mac, 6);
    APPLOG("mac: %02x:%02x:%02x:%02x:%02x:%02x\r\n", 
        mac[0], 
        mac[1], 
        mac[2], 
        mac[3], 
        mac[4], 
        mac[5]);
}

static void mtk_thread_lelink_proc(void *args) {
    int ret = 0; 
    void *ctxR2R;
    void *ctxQ2A;
    leLedInit();
    printForFac();
    CoOTAReset(0);
    haalCoOTAProcessing();
    printf("Build Time: " __DATE__ " " __TIME__ "\r\n");
    ret = lelinkStorageInit(CUST_BASE, CUST_LENGTH - GW_CONF_LENGTH, 0x1000);//CM4 buff slim:128KB + fota buff slim:128KB;->totalSize:0x40000
    if (0 > ret) {
        APPLOGE("lelinkStorageInit ret[%d]\r\n", ret);
        vTaskDelete(NULL);
        return;
    }
    // protocol
    ret = lelinkInit(NULL);
    if (0 > ret) {
        APPLOGE("lelinkInit ret[%d]\r\n", ret);
        vTaskDelete(NULL);
        return;
    }

    ctxR2R = (void *)lelinkNwNew(REMOTE_BAK_IP, REMOTE_BAK_PORT, 0, NULL);
    ctxQ2A = (void *)lelinkNwNew(NULL, 0, NW_SELF_PORT, NULL);

    while (1) {
        lelinkPltCtrlProcess();
        lelinkPollingState(1, ctxR2R, ctxQ2A);
    }

    lelinkNwDelete(ctxR2R);
    lelinkNwDelete(ctxQ2A);
    vTaskDelete(NULL);
}

static int platform_launch_lelink_start(void) {

    if (pdPASS != xTaskCreate(mtk_thread_lelink_proc,
							  "thread_lelink_proc",
							  1024*6,  //16K stack size;
							  NULL,
							  1,
							  NULL)) {
		LOG_E(common, "create user task fail");
		return -1;
	}
#ifdef HW_AES
    aes_task_init();
#endif
    return 0;
}

uint8_t g_ipv4_addr[4] = {0};

extern void tickless_init(void);

void wifi_socket_ip_ready_callback(const struct netif *netif) { 
    APPLOG("[wifibox]begin to create wifi_audio_box_socket_task. \n");
    //xTaskHandle xHandle;

#if BYTE_ORDER == BIG_ENDIAN
     g_ipv4_addr[0] = (((netif->ip_addr).addr) >> 24) & 0xff ;
     g_ipv4_addr[1] = (((netif->ip_addr).addr) >> 16) & 0xff ;
     g_ipv4_addr[2] = (((netif->ip_addr).addr) >> 8) & 0xff ;
     g_ipv4_addr[3] = ((netif->ip_addr).addr) & 0xff ;
     APPLOG("[BIG_ENDIAN]:wifi_box_socket_test(), ip_addr = %d.%d.%d.%d \n",g_ipv4_addr[0], g_ipv4_addr[1], g_ipv4_addr[2], g_ipv4_addr[3]);
#else
     g_ipv4_addr[0] = ((netif->ip_addr).addr) & 0xff ;
     g_ipv4_addr[1] = (((netif->ip_addr).addr) >> 8) & 0xff ;
     g_ipv4_addr[2] = (((netif->ip_addr).addr) >> 16) & 0xff ;
     g_ipv4_addr[3] = (((netif->ip_addr).addr) >> 24) & 0xff ;
     APPLOG("[LITTLE_ENDIAN]:wifi_box_socket_test(), ip_addr = %d.%d.%d.%d \n",g_ipv4_addr[0], g_ipv4_addr[1], g_ipv4_addr[2], g_ipv4_addr[3]);
#endif
     gin_airconfig_ap_connected = 1;
}

int main(void)
{
    /* Do system initialization, eg: hardware, nvdm, logging and random seed. */
    system_init();

    /* bsp_ept_gpio_setting_init() under driver/board/mt76x7_hdk/ept will initialize the GPIO settings
     * generated by easy pinmux tool (ept). ept_*.c and ept*.h are the ept files and will be used by
     * bsp_ept_gpio_setting_init() for GPIO pinumux setup.
     */
    bsp_ept_gpio_setting_init();

#if configUSE_TICKLESS_IDLE == 2
    if (hal_sleep_manager_init() == HAL_SLEEP_MANAGER_OK) {
        tickless_init();
    }
#endif

    /* User initial the parameters for wifi initial process,  system will determin which wifi operation mode
     * will be started , and adopt which settings for the specific mode while wifi initial process is running*/
    wifi_cfg_t wifi_config = {0};
    if (0 != wifi_config_init(&wifi_config)) {
        LOG_E(common, "wifi config init fail");
        return -1;
    }

    wifi_config_t config = {0};
    wifi_config_ext_t config_ext = {0};

    config.opmode = wifi_config.opmode;

    memcpy(config.sta_config.ssid, wifi_config.sta_ssid, 32);
    config.sta_config.ssid_length = wifi_config.sta_ssid_len;
    config.sta_config.bssid_present = 0;
    memcpy(config.sta_config.password, wifi_config.sta_wpa_psk, 64);
    config.sta_config.password_length = wifi_config.sta_wpa_psk_len;
    config_ext.sta_wep_key_index_present = 1;
    config_ext.sta_wep_key_index = wifi_config.sta_default_key_id;
    config_ext.sta_auto_connect_present = 1;
    config_ext.sta_auto_connect = 1;

    memcpy(config.ap_config.ssid, wifi_config.ap_ssid, 32);
    config.ap_config.ssid_length = wifi_config.ap_ssid_len;
    memcpy(config.ap_config.password, wifi_config.ap_wpa_psk, 64);
    config.ap_config.password_length = wifi_config.ap_wpa_psk_len;
    config.ap_config.auth_mode = (wifi_auth_mode_t)wifi_config.ap_auth_mode;
    config.ap_config.encrypt_type = (wifi_encrypt_type_t)wifi_config.ap_encryp_type;
    config.ap_config.channel = wifi_config.ap_channel;
    config.ap_config.bandwidth = wifi_config.ap_bw;
    config.ap_config.bandwidth_ext = WIFI_BANDWIDTH_EXT_40MHZ_UP;
    config_ext.ap_wep_key_index_present = 1;
    config_ext.ap_wep_key_index = wifi_config.ap_default_key_id;
    config_ext.ap_hidden_ssid_enable_present = 1;
    config_ext.ap_hidden_ssid_enable = wifi_config.ap_hide_ssid;

    /* Initialize wifi stack and register wifi init complete event handler,
     * notes:  the wifi initial process will be implemented and finished while system task scheduler is running,
     *            when it is done , the WIFI_EVENT_IOT_INIT_COMPLETE event will be triggered */
    wifi_init(&config, &config_ext);

    wifi_connection_register_event_handler(WIFI_EVENT_IOT_INIT_COMPLETE, wifi_init_done_handler);


    /* Tcpip stack and net interface initialization,  dhcp client, dhcp server process initialization*/
    lwip_network_init(config.opmode);
    lwip_net_start(config.opmode);

#ifdef MTK_WIFI_CONFIGURE_FREE_ENABLE
        uint8_t configured = 0;
        register_configure_free_callback(save_cf_credential_to_nvdm,  save_cf_ready_to_nvdm);
        get_cf_ready_to_nvdm(&configured);
        if (!configured) { // not configured
#ifdef MTK_SMTCN_ENABLE
            /* Config-Free Demo */
            if (wifi_config.opmode == 1) {
                mtk_smart_connect();
            }
#endif
        }
#endif /* MTK_WIFI_CONFIGURE_FREE_ENABLE */

#if defined(MTK_MINICLI_ENABLE)
    /* Initialize cli task to enable user input cli command from uart port.*/
    cli_def_create();
    cli_task_create();
#endif

#ifdef MTK_HOMEKIT_ENABLE
    homekit_init();
#endif

    platform_launch_lelink_start();
	
    vTaskStartScheduler();

    /* If all is well, the scheduler will now be running, and the following line
    will never be reached.  If the following line does execute, then there was
    insufficient FreeRTOS heap memory available for the idle and/or timer tasks
    to be created.  See the memory management section on the FreeRTOS web site
    for more details. */
    for ( ;; );
}

