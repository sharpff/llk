
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "timers.h"
#include "type_def.h"
#include "smt_core.h"
#include "smt_conn.h"
#include "smt_api.h"
#include "wifi_api.h"
#include "wifi_scan.h"
#include "semphr.h"
#include "syslog.h"
#include "wifi_rx_desc.h"
#include "nvdm.h"
#include "os.h"
#include "wlan.h"
#include "airconfig.h"

#define AIRCONFIG_CHANNEL_TIMEOUT (12*1000/portTICK_PERIOD_MS)

#ifndef ETH_ALEN
#define ETH_ALEN        6
#endif

typedef enum AK_STATUS{
    AK_RCV_PRE,
    AK_RCV_DATA,
    AK_FIN,
}ak_status_t;

uint8_t gin_airconfig_current_channel;

static ak_status_t ak_status;
static char ginSsid[33] = {0};

static uint8_t gin_airconfig_channel_locked;
extern int gin_airconfig_channel_cared[MAX_CHANNEL_CARE];
extern int8_t gin_airconfig_sniffer_got;

extern smtcn_info   saved_smtcn_info;
extern int airconfig_stop();
extern void atomic_write_smtcn_flag(uint8_t flag_value);

static TimerHandle_t lelink_lock_timer = NULL;

static void lelink_lock_timeout( TimerHandle_t tmr )
{
	printf("<INFO> Airkiss lock channel timeout.</INFO>\n");
    smtcn_continue_switch();
}

static int lelink_init(const unsigned char *key, const unsigned char key_length)
{
	int ret = 0;

	//channel period;
	lelink_lock_timer = xTimerCreate( "lelink_lock_timer",
									//(locked_channel_timems/portTICK_PERIOD_MS), /*the period being used.*/
									(AIRCONFIG_CHANNEL_TIMEOUT/portTICK_PERIOD_MS), /*the period being used.*/
									pdFALSE,
									NULL,
									lelink_lock_timeout);
	
	if (lelink_lock_timer == NULL) 
	{
		printf("<ERR>ak_lock_timer create fail.</ERR>\n");
		return -1;
	}

	ak_status = AK_RCV_PRE;
	return ret;
}


static void lelink_cleanup(void)
{
	if (lelink_lock_timer != NULL) 
	{
		xTimerDelete(lelink_lock_timer, tmr_nodelay);
		lelink_lock_timer = NULL;
	}

}

int lelink_reset(void)
{

	ak_status = AK_RCV_PRE;
	
	return 0;
}

int lelink_recv(char *p, int len)
{	
	wlan_frame_t* frame = (wlan_frame_t*)p;
	
	// uint8_t latest_ssid_crc8 = 0;
	if (frame->frame_type == BEACON) 
	{
#if 1
		os_snprintf(ginSsid, frame->frame_data.beacon_info.ssid_len + 1,
			"%s", frame->frame_data.beacon_info.ssid);
		//printf("BEACON => [%s] \r\n", frame->frame_data.beacon_info.ssid);
		// latest_ssid_crc8 = crc8(ginSsid, strlen(ginSsid));
		// wmprintf("SSID: %s crc[0x%02x-0x%02x]\r\n", ginSsid, latest_ssid_crc8);
#endif      
	} 
	else if (frame->frame_type == DATA)
	{
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
		//if (!gin_airconfig_running)
			//return;

		if (!gin_airconfig_channel_locked)
		{
			// only multicast
			if (!(0x01 == tmp_dest[0] && 0x00 == tmp_dest[1] && 0x5E == tmp_dest[2])) {
				// only broadcast
				for (i = 0; i < ETH_ALEN; i++) {
					if (tmp_dest[i] != 0xFF) {
						return AIRCONFIG_NW_STATE_NONE;
					}
				}
			}

			// if (((tmp_src[0] == 0x58) &&
			//	  (tmp_src[1] == 0x20) &&
			//	   (tmp_src[2] == 0xb1) &&
			//	   (tmp_src[3] == 0x79) &&
			//	   (tmp_src[4] == 0xe8) && 
			//	   (tmp_src[5] == 0x5f))) {
			//	   wmprintf("ch[%d], ieee80211_hdr->TRU:%02X:%02X:%02X:%02X:%02X:%02X--SRC:%02X:%02X:%02X:%02X:%02X:%02X--DST:%02X:%02X:%02X:%02X:%02X:%02X---%d\r\n",
			//		   gin_airconfig_current_channel, tmp_bssid[0], tmp_bssid[1], tmp_bssid[2], tmp_bssid[3], tmp_bssid[4], tmp_bssid[5],
			//		   tmp_src[0], tmp_src[1], tmp_src[2], tmp_src[3], tmp_src[4], tmp_src[5],
			//		   tmp_dest[0], tmp_dest[1], tmp_dest[2], tmp_dest[3], tmp_dest[4], tmp_dest[5],
			//		   len
			//	   );
			// }

			item.data = len;
			os_memcpy(item.mac_src, tmp_src, sizeof(item.mac_src));
			os_memcpy(item.mac_dst, tmp_dest, sizeof(item.mac_dst));
			os_memcpy(item.mac_bssid, tmp_bssid, sizeof(item.mac_bssid));
			airconfig_nw_state_t state = (airconfig_nw_state_t)airconfig_do_sync(&item, gin_airconfig_current_channel, gin_airconfig_channel_cared, &base);
			switch (state) 
			{
				case AIRCONFIG_NW_STATE_NONE:
				break;
				case AIRCONFIG_NW_STATE_ERROR_PARAM:
				break; 
				case AIRCONFIG_NW_STATE_CHANNEL_LOCKED: 
				{
					printf("[Prov] state => gin_airconfig_channel_locked \r\n");
					gin_airconfig_channel_locked = 1;
					//smtcn_stop_switch();
					// os_thread_sleep(AIRCONFIG_INFINITE_INTERVAL);
				}
				break; 
			}

			return state;
		} 
		else if (0 == os_memcmp(item.mac_src, tmp_src, sizeof(item.mac_src))) 
		{
			ap_passport_t passport;
			// wmprintf("[Prov] sniffer Channel# [%d] \r\n", gin_airconfig_current_channel);
			int ret = airconfig_get_info(len, base, &passport, ginSsid, os_strlen(ginSsid));
			// int ret = airconfig_get_info(len, base, &passport, NULL, 0);
			if (ret) 
			{
				//APPLOG("ssid[%s] psk[%s] base[%d]\r\n", passport.ssid, passport.psk, base);
				printf("ssid[%s] psk[%s] base[%d]\r\n", passport.ssid, passport.psk, base);
				airconfig_stop();
				//inner_set_ap_info(&passport);
				gin_airconfig_sniffer_got = 1;

				/*SSID*/
				saved_smtcn_info.ssid_len = os_strlen(passport.ssid);
				if(saved_smtcn_info.ssid_len > WIFI_MAX_LENGTH_OF_SSID)
					saved_smtcn_info.ssid_len = WIFI_MAX_LENGTH_OF_SSID;
				os_memcpy(saved_smtcn_info.ssid, passport.ssid, saved_smtcn_info.ssid_len);

				/*password*/
				saved_smtcn_info.pwd_len = os_strlen(passport.psk);
				if(saved_smtcn_info.pwd_len > WIFI_LENGTH_PASSPHRASE)
					saved_smtcn_info.pwd_len = WIFI_LENGTH_PASSPHRASE;
				os_memcpy(saved_smtcn_info.pwd, passport.psk, saved_smtcn_info.pwd_len);

				return AIRCONFIG_NW_STATE_COMPLETED;
			}
			else
				return AIRCONFIG_NW_STATE_NONE;
		}
	}

	return AIRCONFIG_NW_STATE_NONE;
}

static int lelink_get_info(void)
{
    xTimerStop(lelink_lock_timer, tmr_nodelay);
    atomic_write_smtcn_flag(SMTCN_FLAG_FIN);
    return 0;
}

static int lelink_input(char* phdr, int len)
{
    //printf_high("enter ak_rx_handler\n");
    int ret;
    ret = lelink_recv(phdr, len);
    //ak_dbg("<INFO>airkiss_recv result is:%d</INFO>\n", ret);

    switch(ak_status)
	{
	    case AK_RCV_PRE:
	        if(ret == AIRCONFIG_NW_STATE_CHANNEL_LOCKED)
			{
	            //LOG_I(airkiss, "<INFO>Airkiss channel locked</INFO>\n");
	            printf("<INFO>Airkiss channel locked</INFO>\n");
	            smtcn_stop_switch();
	            xTimerStart(lelink_lock_timer, tmr_nodelay);
	            ak_status = AK_RCV_DATA;
	        }
	        break;

	    case AK_RCV_DATA:
	        if(ret == AIRCONFIG_NW_STATE_COMPLETED)
			{
			    ///LOG_I(airkiss, "<INFO>Airkiss Finished</INFO>\n");
			    printf("<INFO>Airkiss Finished</INFO>\n");
	            ak_status = AK_FIN;
			    lelink_get_info();
		    }
	        break;

	    case AK_FIN:
	        break;
    }

    return ret;
}

const smtcn_proto_ops lelink_proto_ops = {
    .init               =   lelink_init,
    .cleanup            =   lelink_cleanup,
    .switch_channel_rst =   lelink_reset,
    .rx_handler         =   lelink_input,
};

