/*
 *  Copyright (C) 2008-2015, Marvell International Ltd.
 *  All Rights Reserved.
 */

/*
 * Simple sniffer application
 *
 * Summary:
 *
 * This application configures the Wi-Fi firmware to act as a
 * sniffer. Important information about all the frames received is printed on
 * the console.
 *
 * Description:
 *
 * The application is written using Application Framework that
 * simplifies development of WLAN networking applications.
 *
 * WLAN Initialization:
 *
 * When the application framework is started, it starts up the WLAN
 * sub-system and initializes the network stack. The app receives the event
 * when the WLAN subsystem has been started and initialized.
 *
 * The application starts the sniffer mode of the Wi-Fi module. A sniffer
 * callback is registered that gets called whenever a frame is received from the
 * firmware.
 *
 * The sniffer callback prints important information about the beacon or data
 * packets that are received.
 *
 */
#if 0
#include <wm_os.h>
#include <app_framework.h>
#include <wmtime.h>
#include <partition.h>
#include <cli.h>
#include <cli_utils.h>
#include <wlan.h>
#include <wmstdio.h>
#include <wmsysinfo.h>
#include <wm_net.h>
#include <httpd.h>
#include <wifidirectutl.h>
#include <nw_utils.h>
#endif//0

#if 1//defined(__MRVL_SDK3_3__)
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "timers.h"

#include "halHeader.h"
#include "airconfig.h"
#include "protocol.h"
#include "state.h"
//#include "data.h"
#include "io.h"
#include "ota.h"
#include "hal_sleep_manager.h"
#else
#include <lelink/sw/leconfig.h>
#include <lelink/sw/airconfig.h>
#include <lelink/sw/protocol.h>
#include <lelink/sw/state.h>
#include <lelink/sw/data.h>
#include <lelink/sw/io.h>
#include <lelink/sw/ota.h>
#endif

#if defined(MTK_SMTCN_ENABLE)
#include "smt_conn.h"
#include "smt_api.h"
#endif
#include "netif.h"
#include "flash_map.h"

#include "sys_init.h"
#include "bsp_gpio_ept_config.h"
#include "network_init.h"
#include "cli_def.h"


//static uint8_t gin_airconfig_running;
//static uint8_t gin_airconfig_channel_locked;
uint8_t gin_airconfig_current_channel = 0;

int gin_airconfig_channel_cared[MAX_CHANNEL_CARE];
//struct wlan_network gin_sta_net;

static TimerHandle_t lelink_airconfig_timer = NULL;


int airconfig_start(void *pc, uint8_t *prov_key, int prov_key_len);
int airconfig_stop();

//void inner_set_ap_info(const ap_passport_t *passport);
//int inner_start_to_connect_ap(const ap_passport_t *passport);

// static int lelink_start();

/*-----------------------Global declarations----------------------*/
int8_t gin_airconfig_ap_connected;
int8_t gin_airconfig_sniffer_got;
char ginSsid[33];

/* This function is defined for handling critical error.
 * For this application, we just stall and do nothing when
 * a critical error occurs.
 *
 */
#if 0
void appln_critical_error_handler(void *data)
{
	while (1)
		;
	/* do nothing -- stall */
}

void print_frame_info(const wlan_frame_t *frame)
{
	if (frame->frame_type == BEACON) {
		wmprintf("********* Beacon Info *********");
		wmprintf("\r\nType: 0x%x", frame->frame_type);
		wmprintf("\r\nFrame Control flags: 0x%x",
				frame->frame_data.beacon_info.frame_ctrl_flags);
		wmprintf("\r\nSequence Number: %d",
				wlan_get_seq_num(frame->frame_data.
				beacon_info.seq_frag_num));
		wmprintf("\r\nFragmentation Number: %d",
				wlan_get_frag_num(frame->frame_data.
				beacon_info.seq_frag_num));
		snprintf(ginSsid, frame->frame_data.beacon_info.ssid_len + 1,
				"%s", frame->frame_data.beacon_info.ssid);
		wmprintf("\r\nSSID: %s", ginSsid);
		wmprintf("\r\n*******************************\r\n");
	} else if (frame->frame_type == DATA) {
		if (frame->frame_data.data_info.frame_ctrl_flags & 0x01) {
			wmprintf("********* Data Packet Info *********");
			if (frame->frame_data.data_info.frame_ctrl_flags & 0x08)
				wmprintf("\r\nThis is a retransmission\r\n");
			wmprintf("\r\nType: 0x%x", frame->frame_type);
			wmprintf("\r\nFrame Control flags: 0x%x",
				frame->frame_data.data_info.frame_ctrl_flags);
			wmprintf("\r\nBSSID: ");
			print_mac(frame->frame_data.data_info.bssid);
			wmprintf("\r\nSource: ");
			print_mac(frame->frame_data.data_info.src);
			wmprintf("\r\nDestination: ");
			print_mac(frame->frame_data.data_info.dest);
			wmprintf("\r\nSequence Number: %d",
				wlan_get_seq_num(frame->frame_data.
				data_info.seq_frag_num));
			wmprintf("\r\nFragmentation Number: %d",
				wlan_get_frag_num(frame->frame_data.
				data_info.seq_frag_num));
			wmprintf("\r\nQoS Control Flags: 0x%x",
				frame->frame_data.data_info.qos_ctrl);
			wmprintf("\r\n*******************************\r\n");
		}
	}
}
#endif//0

//#define AIRCONFIG_INFINITE_INTERVAL os_msec_to_ticks(0xffffffff)


//#define AIRCONFIG_TIMEOUT os_msec_to_ticks(60*60*1000)
#define AIRCONFIG_TIMEOUT (60*60*1000/portTICK_PERIOD_MS)

#if 0
#define LOCK_AIRCONFIG "lock_airconfig"
static os_thread_t thread_airconfig;
static os_thread_t thread_lelink;
static os_timer_t timer_airconfig;
static os_timer_t timer_timeout;
static os_thread_stack_define(thread_stack_airconfig, 1024);
static os_thread_stack_define(thread_stack_lelink, 1024 * 16);
static os_semaphore_t sem_airconfig;
#endif

// static uint16_t g_airconfig_base = 0x0000;
// extern uint8_t gin_ssid_crc;
#if 0
void native_sniffer_airconfig_processing(const wlan_frame_t *frame, const uint16_t len)
{   
    // uint8_t latest_ssid_crc8 = 0;
    if (frame->frame_type == BEACON) {
        #if 1
        snprintf(ginSsid, frame->frame_data.beacon_info.ssid_len + 1,
            "%s", frame->frame_data.beacon_info.ssid);
        // latest_ssid_crc8 = crc8(ginSsid, strlen(ginSsid));
        // wmprintf("SSID: %s crc[0x%02x-0x%02x]\r\n", ginSsid, latest_ssid_crc8);
        #endif      
    } else  if (frame->frame_type == DATA){
        const uint8_t *tmp_dest;
        const uint8_t *tmp_src;
        const uint8_t *tmp_bssid;
        static uint16_t base = 0;
        static target_item_t item;
        int i = 0;
        tmp_dest = (uint8_t*)frame->frame_data.data_info.dest;
        tmp_bssid = (uint8_t*)frame->frame_data.data_info.bssid;
        tmp_src  = (uint8_t*)frame->frame_data.data_info.src; 
            //wmprintf("frame_type is [0x%x]\r\n", frame->frame_type);
        if (!gin_airconfig_running)
            return;

        if (!gin_airconfig_channel_locked)
        {
#if 0
            wmprintf("********* Data Packet Info *********");
            if (frame->frame_data.data_info.frame_ctrl_flags & 0x08)
                wmprintf("\r\nThis is a retransmission\r\n");
            wmprintf("\r\nType: 0x%x", frame->frame_type);
            wmprintf("\r\nFrame Control flags: 0x%x",
                frame->frame_data.data_info.frame_ctrl_flags);
            wmprintf("\r\nBSSID: ");
            print_mac(frame->frame_data.data_info.bssid);
            wmprintf("\r\nSource: ");
            print_mac(frame->frame_data.data_info.src);
            wmprintf("\r\nDestination: ");
            print_mac(frame->frame_data.data_info.dest);
            wmprintf("\r\nSequence Number: %d",
            wlan_get_seq_num(frame->frame_data.data_info.seq_frag_num));
            wmprintf("\r\nFragmentation Number: %d",
            wlan_get_frag_num(frame->frame_data.data_info.seq_frag_num));
            wmprintf("\r\nQoS Control Flags: 0x%x",
                frame->frame_data.data_info.qos_ctrl);
            wmprintf("\r\n*******************************\r\n");

            /**address#3 is is the destination address*/
            tmp_dest = frame->frame_data.data_info.src;
            /*address #2 is the source address*/
            tmp_src  = frame->frame_data.data_info.bssid;

#endif


            // only multicast
            if (!(0x01 == tmp_dest[0] && 0x00 == tmp_dest[1] && 0x5E == tmp_dest[2])) {
                // only broadcast
                for (i = 0; i < ETH_ALEN; i++) {
                    if (tmp_dest[i] != 0xFF) {
                        return;
                    }
                }
            }

            // if (((tmp_src[0] == 0x58) &&
            //    (tmp_src[1] == 0x20) &&
            //     (tmp_src[2] == 0xb1) &&
            //     (tmp_src[3] == 0x79) &&
            //     (tmp_src[4] == 0xe8) && 
            //     (tmp_src[5] == 0x5f))) {
            //     wmprintf("ch[%d], ieee80211_hdr->TRU:%02X:%02X:%02X:%02X:%02X:%02X--SRC:%02X:%02X:%02X:%02X:%02X:%02X--DST:%02X:%02X:%02X:%02X:%02X:%02X---%d\r\n",
            //         gin_airconfig_current_channel, tmp_bssid[0], tmp_bssid[1], tmp_bssid[2], tmp_bssid[3], tmp_bssid[4], tmp_bssid[5],
            //         tmp_src[0], tmp_src[1], tmp_src[2], tmp_src[3], tmp_src[4], tmp_src[5],
            //         tmp_dest[0], tmp_dest[1], tmp_dest[2], tmp_dest[3], tmp_dest[4], tmp_dest[5],
            //         len
            //     );
            // }

            item.data = len;
            memcpy(item.mac_src, tmp_src, sizeof(item.mac_src));
            memcpy(item.mac_dst, tmp_dest, sizeof(item.mac_dst));
            memcpy(item.mac_bssid, tmp_bssid, sizeof(item.mac_bssid));
            airconfig_nw_state_t state = (airconfig_nw_state_t)airconfig_do_sync(&item, gin_airconfig_current_channel, gin_airconfig_channel_cared, &base);
            switch (state) {
                case AIRCONFIG_NW_STATE_NONE:
                break;
                case AIRCONFIG_NW_STATE_ERROR_PARAM:
                break; 
                case AIRCONFIG_NW_STATE_CHANNEL_LOCKED: {
                    wmprintf("[Prov] state => gin_airconfig_channel_locked \r\n");
                    gin_airconfig_channel_locked = 1;
                    // os_thread_sleep(AIRCONFIG_INFINITE_INTERVAL);
                }break; 
            }
        } else if (0 == memcmp(item.mac_src, tmp_src, sizeof(item.mac_src))) {
            ap_passport_t passport;
            // wmprintf("[Prov] sniffer Channel# [%d] \r\n", gin_airconfig_current_channel);
            int ret = airconfig_get_info(len, base, &passport, ginSsid, strlen(ginSsid));
            // int ret = airconfig_get_info(len, base, &passport, NULL, 0);
            if (ret) {
                APPLOG("ssid[%s] psk[%s] base[%d]\r\n", passport.ssid, passport.psk, base);
                airconfig_stop();
                inner_set_ap_info(&passport);
                gin_airconfig_sniffer_got = 1;
            }
        }
    }
}


static void timer_airconfig_cb(os_timer_arg_t handle)
{
	os_semaphore_put(&sem_airconfig);
}

static void timer_timeout_cb(os_timer_arg_t handle)
{
    airconfig_stop();
    os_timer_deactivate(&timer_timeout);
    os_timer_delete(&timer_timeout);
    wmprintf("timer_timeout_cb \r\n");
}
#endif//0
static void lelink_airconfig_timeout_timer_callback( TimerHandle_t tmr )
{
    //int ret = HTTPCLIENT_ERROR_CONN;

    if (tmr) 
	{
        xTimerStop(tmr, 0);
        xTimerDelete(tmr, portMAX_DELAY);
        lelink_airconfig_timer = NULL;
    }

    airconfig_stop();

	printf("lelink_airconfig_timeout_timer_callback(). \r\n");

	#if 0
	//timer loop;
   	{
        lelink_airconfig_timer = xTimerCreate( "lelink_airconfig_timer",
                                       AIRCONFIG_TIMEOUT,
                                       pdFALSE,
                                       NULL,
                                       lelink_airconfig_timeout_timer_callback);
        if (lelink_airconfig_timer != NULL) {
            xTimerStart(lelink_airconfig_timer, 0);
        }
    }
	#endif//0
}

#if 0
static void thread_airconfig_proc(void *args)//marvell;
{

    int ret;
    int channel = 1;
    int do_care = 0;

// struct wlan_network *net;

	ret = os_semaphore_create(&sem_airconfig, "sem_airconfig");
	os_semaphore_get(&sem_airconfig, OS_WAIT_FOREVER);

	ret = os_timer_create(&timer_airconfig, "timer_airconfig",
			      AIRCONFIG_CHANNEL_SWITCH_INTERVAL,
			      timer_airconfig_cb, NULL, OS_TIMER_PERIODIC,
			      OS_TIMER_AUTO_ACTIVATE);

#if 0
	ret = aes_drv_init();
	if (ret != WM_SUCCESS) {
		wmlog_e("Unable to initialize AES engine.\r\n");
		return;
	}
#endif

	wakelock_get(LOCK_AIRCONFIG);

	gin_airconfig_running = 1;
	while (gin_airconfig_running) {
        int i = 0;
        if (gin_airconfig_channel_locked) {
            //os_thread_sleep(AIRCONFIG_CHANNEL_TIMEOUT);
            do_care = 0;
            for (i = 0; i < MAX_CHANNEL_CARE; i++) {
                if (gin_airconfig_channel_cared[i] && channel == gin_airconfig_channel_cared[i]) {
                    do_care = 1;
                    break;
                }
            }
        } else {
            do_care = 1;
        }

        // test only
        // do_care = 1;


        if (do_care) {
            wmprintf("[Prov] sniffer Channel# [%d] \r\n", channel);
            gin_airconfig_current_channel = channel;
            ret = wlan_sniffer_start(0x07, 0x00, channel, native_sniffer_airconfig_processing);
            //ret = wlan_sniffer_start(0x07, 0x00, channel, ezconn_recv);
            if (ret != WM_SUCCESS)
                wmprintf("Error: wlan_sniffer_start failed.");
            //wmprintf("os_semaphore_get s [%d]\r\n", ret);
            os_semaphore_get(&sem_airconfig, OS_WAIT_FOREVER);
            //wmprintf("os_semaphore_get e [%d]\r\n", ret);
            if (gin_airconfig_channel_locked) 
			{
                os_thread_sleep(AIRCONFIG_CHANNEL_TIMEOUT);
                wmprintf("AIRCONFIG_CHANNEL_TIMEOUT for channel[%d] [%d] \r\n", gin_airconfig_channel_cared[i], channel);
                gin_airconfig_channel_cared[i] = 0;
                if (0 == gin_airconfig_sniffer_got) {
                    // state go back to sniffer
                    gin_airconfig_sniffer_got = -1;
                    airconfig_stop();
                }
            }
            wlan_sniffer_stop();
        }

        if (!gin_airconfig_running) {
           break;
        }

        /*              
        if (es.state == I_LINK_CONNECT_PROVISION_DONE) {
        wmprintf("[prov] I_LINK_CONNECT provisioning done.\r\n");
        break;
        }
        */
        channel++;

        if (wlan_get_11d_enable_status()) {
            if (channel == 15)
                channel = 1;
        }
        else if (channel == 12)
            channel = 1;    
    }

    os_timer_deactivate(&timer_airconfig);
    os_timer_delete(&timer_airconfig);

    wakelock_put(LOCK_AIRCONFIG);
    wmprintf("[Prov] sniffer Channel# [%d] finished\r\n", channel);

    os_thread_self_complete((os_thread_t *)thandle);
    
    return;
}
#endif//0

extern void smtcn_evt_handler(wifi_smart_connection_event_t event, void *data);

static void mtk_thread_airconfig_proc(void *args)
{
    // wifi_smart_connection_status_t ret = WIFI_SMART_CONNECTION_OK;
		
    if(wifi_smart_connection_init(NULL, 0, smtcn_evt_handler) < 0){
        return;
    }

    /*ret = */wifi_smart_connection_start(0);

  #if 0
	if (gin_airconfig_channel_locked) 
	{
        //os_thread_sleep(AIRCONFIG_CHANNEL_TIMEOUT);
        vTaskDelay();
        
        wmprintf("AIRCONFIG_CHANNEL_TIMEOUT for channel[%d] [%d] \r\n", gin_airconfig_channel_cared[i], channel);
        gin_airconfig_channel_cared[i] = 0;
        if (0 == gin_airconfig_sniffer_got) {
            // state go back to sniffer
            gin_airconfig_sniffer_got = -1;
            airconfig_stop();
        }
    }
  #endif
}

int airconfig_start(void *ptr, uint8_t *prov_key, int prov_key_len)
{
    // int ret;
    /*
    if (prov_g.pc != NULL) {
    prov_w("already configured!");
    return -WM_E_PROV_INVALID_CONF;
    }
    

    if (pc == NULL) {
        prov_w("configuration not provided!");
        return -WM_E_PROV_INVALID_CONF;
    }


    if (prov_key_len > 0 && prov_key != NULL) {
        prov_ezconn_set_device_key(prov_key, prov_key_len);
    }


    if (!pc->provisioning_event_handler) {
        prov_w("No event handler specified");
        return -WM_E_PROV_INVALID_CONF;
    }
    */


    //prov_g.pc = pc;

	//air config timeout handler;
    /*ret = os_timer_create(&timer_timeout, "timer_timeout",
                  AIRCONFIG_TIMEOUT,
                  timer_timeout_cb, NULL, OS_TIMER_PERIODIC,
                  OS_TIMER_AUTO_ACTIVATE);
	
    if (ret) 
	{
        APPLOGE("os_timer_create timeout: %d", ret);
        goto fail; 
    }*/
	lelink_airconfig_timer = xTimerCreate( "lelink_airconfig_timer",
                                       AIRCONFIG_TIMEOUT,
                                       pdFALSE,
                                       NULL,
                                       lelink_airconfig_timeout_timer_callback);
    if (lelink_airconfig_timer != NULL) 
	{
        xTimerStart(lelink_airconfig_timer, 0);
    }
	

	//launch air config; 
    /*ret = os_thread_create(&thread_airconfig,
        "airconfig",
        thread_airconfig_proc,
        (void *)&thread_airconfig,
        &thread_stack_airconfig,
        OS_PRIO_3);
    if (ret) {
        APPLOGE("Failed to launch thread: %d", ret);
        goto fail;
    }*/
    if (pdPASS != xTaskCreate(mtk_thread_airconfig_proc,
							  "thread_lelink_proc",
							  1024*2,  //2K stack size;
							  NULL,
							  1,
							  NULL)) 
	{
		LOG_E(common, "create user task fail");
		//return -1;
		return 0;
	}
	
    gin_airconfig_sniffer_got = 0;
    return 1;

/*fail:
    if (thread_airconfig)
        os_thread_delete(&thread_airconfig);

    return 0;*/
}

int airconfig_stop()
{
    //gin_airconfig_channel_locked = 0;
    gin_airconfig_current_channel = 0;
    //gin_airconfig_running = 0;
    airconfig_reset();
    return 0;
}

#if 0
void inner_set_ap_info(const ap_passport_t *passport) {
    gin_airconfig_ap_connected = 0;
    bzero(&gin_sta_net, sizeof(gin_sta_net));
    /* Set SSID as passed by the user */
    strncpy(gin_sta_net.ssid, passport->ssid, strlen(passport->ssid));
    if (strlen(passport->psk))
        gin_sta_net.security.type = WLAN_SECURITY_WPA2;
    else
        gin_sta_net.security.type = WLAN_SECURITY_NONE;
    /* Set the passphrase */
    strncpy(gin_sta_net.security.psk, passport->psk, strlen(passport->psk));
    gin_sta_net.security.psk_len = strlen(gin_sta_net.security.psk);
    /* Set profile name */
    strcpy(gin_sta_net.name, "sta-network");
    /* Set channel selection to auto (0) */
    gin_sta_net.channel = 0;
    /* Set network type to STA */
    gin_sta_net.type = WLAN_BSS_TYPE_STA;
    /* Set network role to STA */
    gin_sta_net.role = WLAN_BSS_ROLE_STA;
    /* Specify address type as dynamic assignment */
    gin_sta_net.ip.ipv4.addr_type = ADDR_TYPE_DHCP;
}
#endif//0

static void mtk_thread_lelink_proc(void *args) 
{
    void *ctxR2R = (void *)lelinkNwNew(REMOTE_BAK_IP, REMOTE_BAK_PORT, 0, NULL);
    void *ctxQ2A = (void *)lelinkNwNew(NULL, 0, NW_SELF_PORT, NULL);
    // int i, ret = 0;


    while (1) 
	{
        lelinkPollingState(100, ctxR2R, ctxQ2A);
        // LELOG("thread_airconfig_proc pollingState ret [%d]", ret);
    }

    lelinkNwDelete(ctxR2R);
    lelinkNwDelete(ctxQ2A);
    //os_thread_self_complete((os_thread_t *)thandle);
}

static int platform_launch_lelink_start(void) 
{
    // int ret;
    /*ret = os_thread_create(&thread_lelink,
        "lelink",
        thread_lelink_proc,
        (void *)&thread_lelink,
        &thread_stack_lelink,
        OS_PRIO_3);*/
    if (pdPASS != xTaskCreate(mtk_thread_lelink_proc,
							  "thread_lelink_proc",
							  1024*16,  //16K stack size;
							  NULL,
							  1,
							  NULL)) 
	{
		LOG_E(common, "create user task fail");
		return -1;
	}
	
    /*if (ret) 
	{
        APPLOGE("Failed to launch thread: %d", ret);
        goto fail;
    }
    return 1;
fail:
    if (thread_lelink)
        os_thread_delete(&thread_lelink);*/

    return 0;
}

// static int lelink_stop(void) {
//     os_thread_delete(&thread_lelink);
//     lelinkDeinit();
// }

#if 0
/** This Sniffer callback is called from a thread with small stack size,
 * So do minimal memory allocations for correct behaviour.
 */
void sniffer_cb(const wlan_frame_t *frame, const uint16_t len)
{
    if (frame) {
        print_frame_info(frame);
    }
}


void printOutBytes(const uint8_t buf[], int len) {
    int i = 0;
    APPPRINTF("print out: [%d]\r\n", len);
    for (i = 0; i < len; i++) {
        APPPRINTF("%02x ", buf[i]);
        if ((i + 1) % 16 == 0) {
            APPPRINTF("\r\n");
        }
    }
    APPPRINTF("\n"); 
}
#endif//0

void printForFac(void) {
    // char fwVer[64] = {0};
    uint8_t mac[6] = {0};
    //getVer(fwVer, sizeof(fwVer));
    //wmprintf("firmware: %s\r\n", fwVer);
    //wlan_get_mac_address(mac);
    halGetMac(mac, 6);
    printf("mac: %02x:%02x:%02x:%02x:%02x:%02x\r\n", 
        mac[0], 
        mac[1], 
        mac[2], 
        mac[3], 
        mac[4], 
        mac[5]);
}

/*
 * Handler invoked when WLAN subsystem is ready.
 *
 * The app-framework tells the handler whether there is
 * valid network information stored in persistent memory.
 *
 * The handler can then chose to connect to the network.
 *
 * We ignore the data and just start a Micro-AP network
 * with DHCP service. This will allow a client device
 * to connect to us and receive a valid IP address via
 * DHCP.
 */
void platform_and_wlan_init_done(void)
{
	int ret;

	/*ret = wlan_cli_init();
    if (ret != WM_SUCCESS) {
		APPLOGE("Error: wlan_cli_init failed");
    }
	ret = wlan_iw_cli_init();
    if (ret != WM_SUCCESS) {
		APPLOGE("Error: wlan_iw_init failed");
    }*/
    
    printForFac();
	
    //os_dump_mem_stats();
    // sector 0x1000(512pcs), block 0x10000(32pcs)
    ret = lelinkStorageInit(CM4_FLASH_LELINK_CUST_ADDR, FLASH_LELINK_CUST_SIZE, 0x1000);//CM4 buff slim:128KB + fota buff slim:128KB;->totalSize:0x40000
    if (0 > ret) 
	{
        //APPLOGE("lelinkStorageInit ret[%d]\r\n", ret);
        printf("lelinkStorageInit ret[%d]\r\n", ret);
        return;
    }
    // protocol
    ret = lelinkInit(NULL);
    if (0 > ret) 
	{
        //APPLOGE("lelinkInit ret[%d]\r\n", ret);
        printf("lelinkInit ret[%d]\r\n", ret);
        return;
    }
    platform_launch_lelink_start();
}

/* This is the main event handler for this project. The application framework
 * calls this function in response to the various events in the system.
 */
#if 0
int common_event_handler(int event, void *data)
{
	switch (event) {
	case AF_EVT_WLAN_INIT_DONE:
        event_wlan_init_done(data);
        break;
    case AF_EVT_UAP_STARTED:
        APPLOG("uap interface started");
        break;
    case AF_EVT_UAP_STOPPED:
        APPLOG("uap interface stopped");
        break;
    case AF_EVT_NORMAL_CONNECTING:
        // if (gin_airconfig_ap_connected == 1) {
        //     LELOGW("need to reconnect");
        //     gin_airconfig_ap_connected = -1;
        // }
        break;
    case AF_EVT_NORMAL_CONNECTED:
        gin_airconfig_ap_connected = 1;
        break;
	default:
		break;
	}

	return 0;
}
#endif//0

extern int getVer(char fwVer[64], int size);
extern int halUpdateImage(int type, const char *url, const char *sig);

#if 0//CLI CMD test OTA func;
void le_ota(int argc, char **argv)
{
    int c;
    char fwVer[64] = {0};
    bool optflag = false, startflag = false;
    static int type = 2;
    // static const char *sig = NULL;
    static char url[64] = "http://115.182.63.167/fei/le_demo.bin";

    cli_optind = 1;
    while ((c = cli_getopt(argc, argv, "t:u:sp")) != -1) {
        switch (c) {
            case 's':
                startflag = true;
                optflag = true;
                break;
            case 't':
                type = atoi(cli_optarg);
                optflag = true;
                break;
            case 'u':
                strncpy(url, cli_optarg, sizeof(url));
                optflag = true;
                break;
            case 'h':
            default:
                goto end;
        }
    }
    if (!optflag) {
        goto end;  
    }
    wmprintf("OTA INFO: Type %d, URL %s\r\n", type, url);
    if(startflag) {
        leOTA(type, url, NULL, 0);
    }
    return;
end:
    getVer(fwVer, sizeof(fwVer));
    wmprintf("%s Usage(%s, %s %s):\r\n", argv[0], fwVer, __DATE__, __TIME__);
    wmprintf("-s start update\r\n");
    wmprintf("-t set update image type. %d (%d-fw, %d-fw-script, %d-ia-script)\r\n", 
            type, OTA_TYPE_FW, OTA_TYPE_FW_SCRIPT, OTA_TYPE_IA_SCRIPT);
    wmprintf("-u set update image url, max 64 bytes. (%s)\r\n", url);
    wmprintf("-h display this message\r\n");
}

void le_reboot(int argc, char **argv) {
    app_reboot(REASON_USER_REBOOT);
}

static struct cli_command le_utils[] = { 
    {"lo", "letv ota", le_ota},
    {"reboot", "letv reboot", le_reboot}
};

int le_utils_cli_init(void)                                                                                                                                            
{
    int i;

    for (i = 0; i < sizeof(le_utils) / sizeof(struct cli_command); i++)
        if (cli_register_command(&le_utils[i]))
            return -WM_FAIL;
    return WM_SUCCESS;
}
#endif//0

uint8_t g_ipv4_addr[4] = {0};

extern void tickless_init(void);

void socket_test(const struct netif *netif)
{	
	printf("[wifibox]begin to create wifi_audio_box_socket_task. \n");
	//xTaskHandle xHandle;

#if BYTE_ORDER == BIG_ENDIAN
	 g_ipv4_addr[0] = (((netif->ip_addr).addr) >> 24) & 0xff ;
	 g_ipv4_addr[1] = (((netif->ip_addr).addr) >> 16) & 0xff ;
	 g_ipv4_addr[2] = (((netif->ip_addr).addr) >> 8) & 0xff ;
	 g_ipv4_addr[3] = ((netif->ip_addr).addr) & 0xff ;
	 printf("[BIG_ENDIAN]:wifi_box_socket_test(), ip_addr = %d.%d.%d.%d \n",g_ipv4_addr[0], g_ipv4_addr[1], g_ipv4_addr[2], g_ipv4_addr[3]);
#else
	 g_ipv4_addr[0] = ((netif->ip_addr).addr) & 0xff ;
	 g_ipv4_addr[1] = (((netif->ip_addr).addr) >> 8) & 0xff ;
	 g_ipv4_addr[2] = (((netif->ip_addr).addr) >> 16) & 0xff ;
	 g_ipv4_addr[3] = (((netif->ip_addr).addr) >> 24) & 0xff ;
	 printf("[LITTLE_ENDIAN]:wifi_box_socket_test(), ip_addr = %d.%d.%d.%d \n",g_ipv4_addr[0], g_ipv4_addr[1], g_ipv4_addr[2], g_ipv4_addr[3]);
#endif

}

void mtk_platform_init()
{
	/* Do system initialization, eg: hardware, nvdm, logging and random seed. */
    system_init();

    /* bsp_ept_gpio_setting_init() under driver/board/mt76x7_hdk/ept will initialize the GPIO settings
     * generated by easy pinmux tool (ept). ept_*.c and ept*.h are the ept files and will be used by
     * bsp_ept_gpio_setting_init() for GPIO pinumux setup.
     */
    bsp_ept_gpio_setting_init();

    /* Initialize network process,  includes: load the wifi profile setting from NVDM,  
     * wifi stack initaization,  and some wifi event handler register,   
     * tcpip stack and net interface initialization,  dhcp client, dhcp server process initialization
     * notes:  the wifi initial process will be implemented and finished while system task scheduler is running,
     *            when it is done , the WIFI_EVENT_IOT_INIT_COMPLETE event will be triggered */
    network_full_init();
    
#if configUSE_TICKLESS_IDLE == 2
    if (hal_sleep_manager_init() == HAL_SLEEP_MANAGER_OK) {
        tickless_init();
    }
#endif

#if defined(MTK_MINICLI_ENABLE)
    /* Initialize cli task to enable user input cli command from uart port.*/
    cli_def_create();
    cli_task_create();
#endif

#ifdef MTK_HOMEKIT_ENABLE
    homekit_init();
#endif
	
	//sta_network_init();
	wifi_register_ip_ready_callback(socket_test);


	/*if (pdPASS != xTaskCreate(user_entry,
							  "user entry",
							  USER_ENTRY_STACK_SIZE,
							  NULL,
							  USER_ENTRY_TASK_PRIO,
							  NULL)) {
		LOG_E(common, "create user task fail");
		return -1;
	}*/
}


int main()
{
    //modules_init();
    mtk_platform_init();

    platform_and_wlan_init_done();
	
    vTaskStartScheduler();

    /* If all is well, the scheduler will now be running, and the following line
    will never be reached.  If the following line does execute, then there was
    insufficient FreeRTOS heap memory available for the idle and/or timer tasks
    to be created.  See the memory management section on the FreeRTOS web site
    for more details. */
    for ( ;; );
}
