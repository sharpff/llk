
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

#define AIRCONFIG_CHANNEL_TIMEOUT (8*1000/portTICK_PERIOD_MS)

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

static void lelink_lock_timeout( TimerHandle_t tmr ) {
	printf("<INFO> Airkiss lock channel timeout.</INFO>\n");
    smtcn_continue_switch();
}

static int lelink_init(const unsigned char *key, const unsigned char key_length) {
	int ret = 0;
	lelink_lock_timer = xTimerCreate( "lelink_lock_timer",
									(AIRCONFIG_CHANNEL_TIMEOUT/portTICK_PERIOD_MS),
									pdFALSE,
									NULL,
									lelink_lock_timeout);
	
	if (lelink_lock_timer == NULL) {
		printf("<ERR>ak_lock_timer create fail.</ERR>\n");
		return -1;
	}
	ak_status = AK_RCV_PRE;
	return ret;
}

static void lelink_cleanup(void) {
	if (lelink_lock_timer != NULL) {
		xTimerDelete(lelink_lock_timer, tmr_nodelay);
		lelink_lock_timer = NULL;
	}
}

int lelink_reset(void) {
	gin_airconfig_channel_locked = 0;
	ak_status = AK_RCV_PRE;
	airconfig_reset();
    /*airhug_reset();*/
	return 0;
}

#ifdef MONITOR_CONFIG4
#include "airhug.h"
#include "io.h"
int lelink_recv(char *p, int len) {
    int ret = 0;
	wlan_frame_t* frame = (wlan_frame_t*)p;
	if (frame->frame_type == BEACON) {
		os_snprintf(ginSsid, frame->frame_data.beacon_info.ssid_len + 1, "%s", frame->frame_data.beacon_info.ssid);
	} else if (frame->frame_type == DATA) {
		const uint8_t *tmp_dest;
		const uint8_t *tmp_src;
		const uint8_t *tmp_bssid;
		tmp_dest = (uint8_t*)frame->frame_data.data_info.dest;
		tmp_bssid = (uint8_t*)frame->frame_data.data_info.bssid;
		tmp_src  = (uint8_t*)frame->frame_data.data_info.src; 
        // APPLOG("SRC %02x:%02x:%02x:%02x:%02x:%02x DST %02x:%02x:%02x:%02x:%02x:%02x BSSID %02x:%02x:%02x:%02x:%02x:%02x len %3d",
        //         tmp_src[0], tmp_src[1], tmp_src[2],tmp_src[3],tmp_src[4],tmp_src[5],
        //         tmp_dest[0], tmp_dest[1], tmp_dest[2],tmp_dest[3],tmp_dest[4],tmp_dest[5],
        //         tmp_bssid[0], tmp_bssid[1], tmp_bssid[2],tmp_bssid[3],tmp_bssid[4],tmp_bssid[5], len);
        ret = airhug_feed_data(tmp_src, tmp_dest, tmp_bssid, len);
        if(ret == 1) {
            gin_airconfig_channel_locked = 1;
            return AIRCONFIG_NW_STATE_CHANNEL_LOCKED;
        } else if(ret == 2) {
            APPLOG("airhug_get ...");
            if(!airhug_get(saved_smtcn_info.ssid, WIFI_MAX_LENGTH_OF_SSID, saved_smtcn_info.pwd, WIFI_LENGTH_PASSPHRASE)) {
                /*SSID*/
                saved_smtcn_info.ssid_len = os_strlen(saved_smtcn_info.ssid);
                /*password*/
                saved_smtcn_info.pwd_len = os_strlen(saved_smtcn_info.pwd);
                APPLOG("AIRHUG GET: '%s' '%s'", saved_smtcn_info.ssid, saved_smtcn_info.pwd);
                {
                    int ret = 0;
                    PrivateCfg cfg;
                    lelinkStorageReadPrivateCfg(&cfg);
                    APPLOG("read last ssid[%s], psk[%s], configStatus[%d]", 
                            cfg.data.nwCfg.config.ssid,
                            cfg.data.nwCfg.config.psk, 
                            cfg.data.nwCfg.configStatus);
                    strcpy(cfg.data.nwCfg.config.ssid, saved_smtcn_info.ssid);
                    cfg.data.nwCfg.config.ssid_len = saved_smtcn_info.ssid_len;
                    cfg.data.nwCfg.config.ssid[cfg.data.nwCfg.config.ssid_len] = '\0';
                    strcpy(cfg.data.nwCfg.config.psk, saved_smtcn_info.pwd);
                    cfg.data.nwCfg.config.psk_len = saved_smtcn_info.pwd_len;
                    cfg.data.nwCfg.config.psk[cfg.data.nwCfg.config.psk_len] = '\0';
                    cfg.data.nwCfg.configStatus = 1;
                    ret = lelinkStorageWritePrivateCfg(&cfg);
                    APPLOG("WRITEN config[%d] configStatus[%d]", ret, cfg.data.nwCfg.configStatus);
                }
                gin_airconfig_sniffer_got = 1;
                return AIRCONFIG_NW_STATE_COMPLETED;
            } else {
                gin_airconfig_channel_locked = 0;
                APPLOG("airhug_get() error!!!");
            }
            airhug_reset();
        }
    }
	return AIRCONFIG_NW_STATE_NONE;
}
#else
int lelink_recv(char *p, int len) {
	wlan_frame_t* frame = (wlan_frame_t*)p;
	if (frame->frame_type == BEACON) {
		os_snprintf(ginSsid, frame->frame_data.beacon_info.ssid_len + 1,
			"%s", frame->frame_data.beacon_info.ssid);
	} else if (frame->frame_type == DATA) {
		const uint8_t *tmp_dest;
		const uint8_t *tmp_src;
		const uint8_t *tmp_bssid;
		static uint16_t base = 0;
		static target_item_t item;
		int i = 0;
		tmp_dest = (uint8_t*)frame->frame_data.data_info.dest;
		tmp_bssid = (uint8_t*)frame->frame_data.data_info.bssid;
		tmp_src  = (uint8_t*)frame->frame_data.data_info.src; 

		if (!gin_airconfig_channel_locked) {
			// only multicast
			if (!(0x01 == tmp_dest[0] && 0x00 == tmp_dest[1] && 0x5E == tmp_dest[2])) {
				// only broadcast
				for (i = 0; i < ETH_ALEN; i++) {
					if (tmp_dest[i] != 0xFF) {
						return AIRCONFIG_NW_STATE_NONE;
					}
				}
			}

			item.data = len;
			os_memcpy(item.mac_src, tmp_src, sizeof(item.mac_src));
			os_memcpy(item.mac_dst, tmp_dest, sizeof(item.mac_dst));
			os_memcpy(item.mac_bssid, tmp_bssid, sizeof(item.mac_bssid));
			airconfig_nw_state_t state = (airconfig_nw_state_t)airconfig_do_sync(&item, gin_airconfig_current_channel, gin_airconfig_channel_cared, &base);
			switch (state) {
				case AIRCONFIG_NW_STATE_NONE:
				break;
				case AIRCONFIG_NW_STATE_ERROR_PARAM:
				break; 
				case AIRCONFIG_NW_STATE_CHANNEL_LOCKED: {
					printf("[Prov] state => gin_airconfig_channel_locked \r\n");
					gin_airconfig_channel_locked = 1;
					//smtcn_stop_switch();
					// os_thread_sleep(AIRCONFIG_INFINITE_INTERVAL);
				}
				break; 
			}
			return state;
		} else if (0 == os_memcmp(item.mac_src, tmp_src, sizeof(item.mac_src))) {
			ap_passport_t passport;
			int ret = airconfig_get_info(len, base, &passport, ginSsid, os_strlen(ginSsid));	
			if (ret) {
				//APPLOG("ssid[%s] psk[%s] base[%d]\r\n", passport.ssid, passport.psk, base);
				printf("ssid[%s] psk[%s] base[%d]\r\n", passport.ssid, passport.psk, base);
				airconfig_stop();
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
			} else {
				return AIRCONFIG_NW_STATE_NONE;
			}
		}
	}
	return AIRCONFIG_NW_STATE_NONE;
}
#endif

static int lelink_get_info(void) {
    xTimerStop(lelink_lock_timer, tmr_nodelay);
    atomic_write_smtcn_flag(SMTCN_FLAG_FIN);
    return 0;
}

static int lelink_input(char* phdr, int len) {
    int ret;
    ret = lelink_recv(phdr, len);
    switch(ak_status) {
	    case AK_RCV_PRE:
	        if(ret == AIRCONFIG_NW_STATE_CHANNEL_LOCKED) {
	            printf("<INFO>Airkiss channel locked</INFO>\n");
	            smtcn_stop_switch();
	            xTimerStart(lelink_lock_timer, tmr_nodelay);
	            ak_status = AK_RCV_DATA;
	        }
	        break;

	    case AK_RCV_DATA:
	        if(ret == AIRCONFIG_NW_STATE_COMPLETED) {
			    printf("<INFO>Airkiss Finished</INFO>\n");
	            ak_status = AK_FIN;
	            airconfig_stop();
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
