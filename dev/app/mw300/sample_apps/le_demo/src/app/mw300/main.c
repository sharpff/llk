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

#include <wm_os.h>
#include <app_framework.h>
#include <wmtime.h>
#include <partition.h>
// #include <appln_cb.h>
// #include <appln_dbg.h>
#include <cli.h>
#include <cli_utils.h>
#include <wlan.h>
#include <psm.h>
#include <wmstdio.h>
#include <wmsysinfo.h>
#include <wm_net.h>
#include <httpd.h>
#include <wifidirectutl.h>
#include <nw_utils.h>

#include <lelink/sw/leconfig.h>
#include <lelink/sw/airconfig.h>
#include <lelink/sw/protocol.h>
#include <lelink/sw/state.h>
#include <lelink/sw/io.h>
// test only
#include <lelink/sw/misc.h>


// #define DEBUG_AIRCONFIG

#include <wmlog.h>
#if APPCONFIG_DEBUG_ENABLE
#define dbg(_fmt_, ...)             \
    wmprintf("[appln] "_fmt_"\r\n", ##__VA_ARGS__)
#else
#define dbg(...)
#endif /* APPCONFIG_DEBUG_ENABLE */



static uint8_t gin_airconfig_running;
static uint8_t gin_airconfig_channel_locked;
static uint8_t gin_airconfig_current_channel;
static int gin_airconfig_channel_cared[MAX_CHANNEL_CARE];
struct wlan_network gin_sta_net;


int airconfig_start(void *pc, uint8_t *prov_key, int prov_key_len);
int airconfig_stop();

void inner_set_ap_info(const ap_passport_t *passport);
int inner_start_to_connect_ap(const ap_passport_t *passport);

static int lelink_start();

/*-----------------------Global declarations----------------------*/
int8_t gin_airconfig_ap_connected;
int8_t gin_airconfig_sniffer_got;
char ginSsid[33];

/* This function is defined for handling critical error.
 * For this application, we just stall and do nothing when
 * a critical error occurs.
 *
 */
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


#define AIRCONFIG_INFINITE_INTERVAL os_msec_to_ticks(0xffffffff)
// #define AIRCONFIG_CHANNEL_SWITCH_INTERVAL os_msec_to_ticks(800)
#define AIRCONFIG_CHANNEL_SWITCH_INTERVAL os_msec_to_ticks(300)
#define AIRCONFIG_CHANNEL_TIMEOUT os_msec_to_ticks(12*1000)
#define AIRCONFIG_TIMEOUT os_msec_to_ticks(60*60*1000)
#define LOCK_AIRCONFIG "lock_airconfig"
static os_thread_t thread_airconfig;
static os_thread_t thread_lelink;
static os_timer_t timer_airconfig;
static os_timer_t timer_timeout;
static os_thread_stack_define(thread_stack_airconfig, 1024);
static os_thread_stack_define(thread_stack_lelink, 2048*4);
static os_semaphore_t sem_airconfig;

// static uint16_t g_airconfig_base = 0x0000;
// extern uint8_t gin_ssid_crc;
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

static void thread_airconfig_proc(os_thread_arg_t thandle)
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
            if (gin_airconfig_channel_locked) {
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

int airconfig_start(void *ptr, uint8_t *prov_key, int prov_key_len)
{
    int ret;
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
    ret = os_timer_create(&timer_timeout, "timer_timeout",
                  AIRCONFIG_TIMEOUT,
                  timer_timeout_cb, NULL, OS_TIMER_PERIODIC,
                  OS_TIMER_AUTO_ACTIVATE);
    if (ret) {
        APPLOGE("os_timer_create timeout: %d", ret);
        goto fail; 
    }
    ret = os_thread_create(&thread_airconfig,
        "airconfig",
        thread_airconfig_proc,
        (void *)&thread_airconfig,
        &thread_stack_airconfig,
        OS_PRIO_3);
    if (ret) {
        APPLOGE("Failed to launch thread: %d", ret);
        goto fail;
    }
    gin_airconfig_sniffer_got = 0;
    return 1;

fail:
    if (thread_airconfig)
        os_thread_delete(&thread_airconfig);

    return 0;
}

int airconfig_stop()
{
    gin_airconfig_channel_locked = 0;
    gin_airconfig_current_channel = 0;
    gin_airconfig_running = 0;
    airconfig_reset();
    return 0;
}

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

static void thread_lelink_proc(os_thread_arg_t thandle) {
    void *ctx_r2r = (void *)lelinkNwNew(REMOTE_IP, REMOTE_PORT, 0, 0);
    void *ctx_q2a = (void *)lelinkNwNew(NULL, 0, NW_SELF_PORT, ctx_r2r);
    // int i, ret = 0;


    while (1) {
        lelinkPollingState(1000, ctx_r2r, ctx_q2a);
        // LELOG("thread_airconfig_proc pollingState ret [%d]", ret);
    }

    lelinkNwDelete(ctx_r2r);
    lelinkNwDelete(ctx_q2a);
    os_thread_self_complete((os_thread_t *)thandle);
}

static int lelink_start(void) {
    int ret;
    ret = os_thread_create(&thread_lelink,
        "lelink",
        thread_lelink_proc,
        (void *)&thread_lelink,
        &thread_stack_lelink,
        OS_PRIO_3);
    if (ret) {
        APPLOGE("Failed to launch thread: %d", ret);
        goto fail;
    }
    return 1;
fail:
    if (thread_lelink)
        os_thread_delete(&thread_lelink);

    return 0;
}

// static int lelink_stop(void) {
//     os_thread_delete(&thread_lelink);
//     lelinkDeinit();
// }

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
void event_wlan_init_done(void *data)
{
	int ret;
    // char utc[] = "{\"redirect\":0,\"utc\":123412341234}";
    // char tmp2[] = "{\"dir\":\"54321\"}";
    // char utc[64] = "{\"utc\":1234}";
    // syncUTC(utc, sizeof(utc));

    // memset(utc, 0, sizeof(utc));
    // memcpy(utc, tmp1, sizeof(tmp1));
    // syncUTC(utc, sizeof(utc));

    // memset(utc, 0, sizeof(utc));
    // memcpy(utc, tmp1, sizeof(tmp1));
    // syncUTC(utc, sizeof(utc));

	/*
	 * Initialize CLI Commands for some of the modules:
	 *
	 * -- wlan: allows user to explore basic wlan functions
	 */

	ret = wlan_cli_init();
	if (ret != WM_SUCCESS)
		dbg("Error: wlan_cli_init failed");

	ret = wlan_iw_cli_init();
	if (ret != WM_SUCCESS)
		dbg("Error: wlan_iw_init failed");

	/** Filter flags:
	 *		Bit[0]: Enable/disable management frame receive,
	 *		Bit[1]: Enable/disable control frame receive,
	 *		Bit[2]: Enable/disable data frame receive.
	 *
	 * radio_type:
	 *		Band configuration type
	 *		Bits[7:6]: Reserved (set to 0)
	 *		Bits[5:4]: Secondary channel offset
	 *		     00 = no secondary channel
	 *		     01 = secondary channel is above primary channel
	 *		     10 = reserved
	 *		     11 = secondary channel is below primary channel
	 *		Bits[3:2]: Channel width
	 *		     00 = 20 MHz channel width
	 *		     01, 10 and 11 = reserved
	 *		Bits[1:0]: Band information
	 *		     00 = 2.4 GHz band
	 *		     01 = 5 GHz band
	 *		     10 and 11 = reserved
	 *
	 * channel:
	 * 		Channel number in respective band.
	 *
	 * To sniff packets in 40 MHz channel bandwidth, specify secondary
	 * channel offset above/below in radio type.
	 *
	 * wlan_sniffer_start(4, 17, 40, sniffer_cb);
	 *
	 * The above call with start sniffer for data frames on channel
	 * number 40 and 44 in 5 GHz band, as secondary channel specified
	 * is above primary channel.
	 *
	 * Following call starts WLAN sniffer capturing control, management
	 * and data frames on channel 8 of 2.4 GHz band.
	 *
	 * All captured frames are delivered to registered sniffer callback
	 * sniffer_cb.
	 *
	 */

	 
    /*
    ret = wlan_sniffer_start(0x07, 0x00, 0x08, sniffer_cb);
    if (ret != WM_SUCCESS)
    dbg("Error: wlan_sniffer_start failed");
    */
    #ifdef DEBUG_AIRCONFIG
    if (0 != airconfig_start(NULL, NULL, 0))
    {
        dbg("Error: wlan_sniffer_start failed");
    }
    return;
    #endif

#if 0
    {
#include <lelink/sw/sengine.h>

        uint32_t parity = UART_PARITY_NONE;
        uint32_t stop = UART_STOPBITS_1;
        uint32_t baud = 9600;
        mdev_t  *uartPort;
        uint8_t buf[] = {0x12, 0x34, 0xab, 0xcd};
        int count = sizeof(buf) + 1;

        // extern int preGenStableInfo2Flash(void);
        // ret = preGenStableInfo2Flash();
        // APPLOG("preGenStableInfo2Flash [%d] \r\n", ret);

        #if 0
        ret = uart_drv_init(UART1_ID, UART_8BIT);
        APPLOG("uart_drv_init [%d] \r\n", ret);
        ret = uart_drv_set_opts(UART1_ID, parity, stop, UART_PARITY_NONE);
        APPLOG("uart_drv_set_opts [%d] \r\n", ret);
        uartPort = uart_drv_open(UART1_ID, baud);
        APPLOG("uart_drv_open [%d] \r\n", uartPort);
        while (count--) { // 4, 3, 2, 1
            ret = uart_drv_write(uartPort, &buf[sizeof(buf) - count], 1);
            APPLOG("uart_drv_write [%d] \r\n", ret);
            os_thread_sleep(os_msec_to_ticks(1000));
        }
        while (1) {
            int i = 0;
            ret = uart_drv_read(uartPort, buf, sizeof(buf));
            APPLOG("uart_drv_read [%d]\r\n", ret);
            if (ret > 0) {
                for (i = 0; i < ret; i++) {
                    APPPRINTF("%02x ", buf[i]);
                }
                APPPRINTF("\r\n");
            }
            os_thread_sleep(os_msec_to_ticks(1000));
        }
        #endif

        #if 1
        {
            // #define PAGE_SIZE 0x100
            #define PAGE_SIZE (0x1000)
            #define WRITE_SIZE 8*1024
            #define PIECES (WRITE_SIZE/PAGE_SIZE)
            #define START_ADDR (0x1E0000)

            mdev_t * dev = NULL;
            int ret, counts = 4;
            int pieces = PIECES, i, j;
            static uint8_t data[PAGE_SIZE] = {0};
            static uint8_t out[WRITE_SIZE] = {0};
            int tmpAddr = 0;

            while (counts--) {
                os_thread_sleep(os_msec_to_ticks(1000));
                APPLOG("waiting... [%d]\r\n", counts);
            }
            for (i = 0; i < pieces; i++) {
                int m = 0;
                tmpAddr = START_ADDR + i*PAGE_SIZE;
                for (j = 0; j < PAGE_SIZE; j++) {
                    data[m++] = j;
                }
                data[sizeof(data) - 1] = i;
                dev = (mdev_t *)flash_drv_open(FL_INT);
                ret = flash_drv_erase((mdev_t *)dev, tmpAddr, PAGE_SIZE);
                APPLOG("flash_drv_erase idx[%d][0x%x] ret[%d]\r\n", i, tmpAddr, ret);
                ret = flash_drv_write((mdev_t *)dev, data, PAGE_SIZE, tmpAddr);
                APPLOG("flash_drv_write idx[%d][0x%x] ret[%d]\r\n", i, tmpAddr, ret);
                memset(out, 0, sizeof(out));
                flash_drv_close((mdev_t *)dev);

                // os_thread_sleep(os_msec_to_ticks(100));

                dev = (mdev_t *)flash_drv_open(FL_INT);
                ret = flash_drv_read(dev, out, PAGE_SIZE, tmpAddr);
                APPLOG("flash_drv_read [0x%x] ret[%d]\r\n", tmpAddr, ret);
                if (0 == ret) {
                    ret = PAGE_SIZE;
                }
                printOutBytes(out, PAGE_SIZE); 
                flash_drv_close((mdev_t *)dev);
                APPLOG("flash_drv_close\n");
            }

            dev = (mdev_t *)flash_drv_open(FL_INT);
            memset(out, 0, sizeof(out));
            ret = flash_drv_read(dev, out, sizeof(out), START_ADDR);
            APPLOG("flash_drv_read [0x%x] ret[%d]\r\n", START_ADDR, ret);
            if (0 == ret) {
                ret = sizeof(out);
            }
            flash_drv_close((mdev_t *)dev);

            printOutBytes(out, ret); 

            APPLOG("flash pieces reading...\r\n");
            for (i = 0; i < pieces; i++) {
                tmpAddr = START_ADDR + i*PAGE_SIZE;
                dev = (mdev_t *)flash_drv_open(FL_INT);
                memset(out, 0, PAGE_SIZE);
                ret = flash_drv_read(dev, out, PAGE_SIZE, tmpAddr);
                APPLOG("flash_drv_read [0x%x] ret[%d]\r\n", tmpAddr, ret);
                if (0 == ret) {
                    ret = PAGE_SIZE;
                }
                flash_drv_close((mdev_t *)dev);

                printOutBytes(out, ret); 
            }
        }
        #endif

        return;
    }
#else
    os_dump_mem_stats();

    // sector 0x1000(512pcs), block 0x10000(32pcs)
    ret = lelinkStorageInit(0x1C2000, 0x3E000, 0x1000);
    if (0 > ret) {
        APPLOGE("lelinkStorageInit ret[%d]\r\n", ret);
        return;
    }

    // protocol
    ret = lelinkInit(NULL);
    if (0 > ret) {
        return;
    }
    lelink_start();
    
    // flash
    // testFlash();

#endif
}

/* This is the main event handler for this project. The application framework
 * calls this function in response to the various events in the system.
 */
int common_event_handler(int event, void *data)
{
	switch (event) {
	case AF_EVT_WLAN_INIT_DONE:
		event_wlan_init_done(data);
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

extern int halUpdateImage(int type, const char *url, const char *sig);
void le_ota(int argc, char **argv)
{
    int c;
    bool start_update = false;
    static int type = 0;
    static const char *sig = NULL;
    static char url[64] = "http://115.182.63.167/fei/le_demo.bin";

    cli_optind = 1;
    while ((c = cli_getopt(argc, argv, "t:u:sp")) != -1) {
        switch (c) {
            case 's':
                start_update = true;
                break;
            case 't':
                 type = atoi(cli_optarg);
                break;
            case 'u':
                strncpy(url, cli_optarg, sizeof(url));
                break;
            case 'p':
                break;
            default:
                goto end;
        }

    }
    wmprintf("Now update info, type = %d, url = %s\r\n", type, url);
    if(start_update) {
        halUpdateImage(type, url, sig);
    }
    return;
end:
    wmprintf("%s Usage:\r\n", argv[0]);
    wmprintf("-s start update\r\n");
    wmprintf("-t set update image type. (0-fw, 1-fw-script, 2-lk-script)\r\n");
    wmprintf("-u set update image url, max 64 bytes\r\n");
    wmprintf("-p print now update information\r\n");
}

static struct cli_command le_utils[] = { 
    {"lo", "letv ota", le_ota},
};

int le_utils_cli_init(void)                                                                                                                                            
{
    int i;

    for (i = 0; i < sizeof(le_utils) / sizeof(struct cli_command); i++)
        if (cli_register_command(&le_utils[i]))
            return -WM_FAIL;
    return WM_SUCCESS;
}

static void modules_init()
{
	int ret;

	/*
	 * Initialize wmstdio prints
	 */
	ret = wmstdio_init(UART0_ID, 0);
	if (ret != WM_SUCCESS) {
		dbg("Error: wmstdio_init failed");
		appln_critical_error_handler((void *) -WM_FAIL);
	}

	/*
	 * Initialize CLI Commands
	 */
	ret = cli_init();
	if (ret != WM_SUCCESS) {
		dbg("Error: cli_init failed");
		appln_critical_error_handler((void *) -WM_FAIL);
	}
	/* Initialize time subsystem.
	 *
	 * Initializes time to 1/1/1970 epoch 0.
	*/
    ret = wmtime_init();
    if (ret != WM_SUCCESS) {
        dbg("Error: wmtime_init failed");
        appln_critical_error_handler((void *) - WM_FAIL);
    }

    /*
    	* Register Power Management CLI Commands
    	*/
    ret = pm_cli_init();
    if (ret != WM_SUCCESS) {
        dbg("Error: pm_cli_init failed");
        appln_critical_error_handler((void *) - WM_FAIL);
    }

    /*
	 * Register Time CLI Commands
	 */
	ret = wmtime_cli_init();
	if (ret != WM_SUCCESS) {
		dbg("Error: wmtime_cli_init failed");
		appln_critical_error_handler((void *) -WM_FAIL);
	}

    ret = sysinfo_init();
    if (ret != WM_SUCCESS) {
        dbg("Error: sysinfo_init failed");
        appln_critical_error_handler((void *) -WM_FAIL);
    }

    ret = nw_utils_cli_init();
    if (ret != WM_SUCCESS) {
        dbg("Error: nw_utils_cli_init failed");
        appln_critical_error_handler((void *) -WM_FAIL);
    }
    ret = le_utils_cli_init();
    if (ret != WM_SUCCESS) {
        dbg("Error: le_utils_cli_init failed");
        appln_critical_error_handler((void *) -WM_FAIL);
    }
	return;
}

int main()
{
    // int count = 0;
    modules_init();

    dbg("Build Time: " __DATE__ " " __TIME__ "");
    // while (1) {
    //     count++;
    //     wmprintf("Hello World: iteration %d\r\n", count);

    //     /* Sleep  5 seconds */
    //     os_thread_sleep(os_msec_to_ticks(3000));
    // }

#if 1
	/* Start the application framework */
	if (app_framework_start(common_event_handler) != WM_SUCCESS) {
		dbg("Failed to start application framework");
				appln_critical_error_handler((void *) -WM_FAIL);
	}
#endif
	return 0;
}
