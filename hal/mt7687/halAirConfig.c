#include "halHeader.h"
#include "io.h"
#include "airconfig.h"
#include "wifi_api.h"
#include "ethernetif.h"
#include "dhcpd.h"
#include "dhcp.h"
#include "hal_log.h"
#include "hal_sys.h"
#include "stdio.h"
#include "stddef.h"
#include "os.h"
#include "network_init.h"

//static int startApListen(void);
extern int airconfig_start(void *pc, uint8_t *prov_key, int prov_key_len);
extern int airconfig_stop();
extern int8_t gin_airconfig_sniffer_got;
extern int8_t gin_airconfig_ap_connected;
extern void network_dhcp_start(uint8_t opmode);
extern wifi_scan_list_item_t g_ap_list[8];

static int g_softap_start = 0;

uint8_t ginSSID[16] = {0};
uint8_t ginPassword[16] = {0};
uint8_t ginChannel = 0;
uint8_t ginSanDone = 0;
static uint8_t ginFirstInFlag = 1;

int halDoConfig(void *ptr, int ptrLen) {
	int ret = 0;

    if (gin_airconfig_sniffer_got) {
        gin_airconfig_sniffer_got = 0;
        //app_sta_stop();//not need to stop STA;
    }
	ret = airconfig_start(NULL, NULL, 0);
    APPLOG("halDoConfig in hal [%d]", ret);
	return ret;
}

int halStopConfig(void) {
    int ret = airconfig_stop();
    hal_sys_reboot(HAL_SYS_REBOOT_MAGIC, WHOLE_SYSTEM_REBOOT_COMMAND);
    return ret;
}

int halDoConfiguring(void *ptr, int ptrLen) {
    APPLOG("Mointor configuring sniffer got [%d]", gin_airconfig_sniffer_got);
	return gin_airconfig_sniffer_got;
}

int wifi_station_start_to_connect(uint8_t *ssid, uint8_t *password) {
    APPLOG("Station mode connect to AP,  SSID[%s][%s]", ssid, password);
    wifi_config_set_opmode(WIFI_MODE_STA_ONLY);
    wifi_config_set_ssid(WIFI_PORT_STA, ssid ,(uint8_t)os_strlen((char *)ssid));
    wifi_config_set_security_mode(WIFI_PORT_STA, WIFI_AUTH_MODE_WPA_PSK_WPA2_PSK, WIFI_ENCRYPT_TYPE_TKIP_AES_MIX);
    wifi_config_set_wpa_psk_key(WIFI_PORT_STA, password, (uint8_t)os_strlen((char *)password));
    wifi_config_reload_setting();
    network_dhcp_start(WIFI_MODE_STA_ONLY);
    return 1;
}

int wifi_repeater_start_to_connect(uint8_t *ssid, uint8_t *password, uint8_t channel) {
    char newSSID[64] = {0};
    sprintf(newSSID, "%s_repeater", ssid);
    APPLOG("Repeater mode connect to AP,  SSID[%s][%s]", ssid, password);
    wifi_set_opmode(WIFI_MODE_REPEATER);
    wifi_config_set_ssid(WIFI_PORT_APCLI, ssid ,(uint8_t)os_strlen((char *)ssid));
    wifi_config_set_wpa_psk_key(WIFI_PORT_APCLI, password, (uint8_t)os_strlen((char *)password));
    wifi_config_set_security_mode(WIFI_PORT_APCLI, WIFI_AUTH_MODE_WPA2_PSK, WIFI_ENCRYPT_TYPE_TKIP_AES_MIX);
    wifi_config_set_channel(WIFI_PORT_APCLI, channel);
    wifi_config_set_bandwidth(WIFI_PORT_APCLI, WIFI_IOT_COMMAND_CONFIG_BANDWIDTH_2040MHZ);
    wifi_config_set_wireless_mode(WIFI_PORT_APCLI, WIFI_PHY_11BG_MIXED);
    wifi_config_set_ssid(WIFI_PORT_AP, (uint8_t *)newSSID, strlen(newSSID));
    wifi_config_set_wpa_psk_key(WIFI_PORT_AP, (uint8_t *)password, strlen(password));
    wifi_config_set_security_mode(WIFI_PORT_AP, WIFI_AUTH_MODE_WPA2_PSK, WIFI_ENCRYPT_TYPE_TKIP_AES_MIX);
    wifi_config_set_channel(WIFI_PORT_AP, channel);
    wifi_config_set_bandwidth(WIFI_PORT_AP, WIFI_IOT_COMMAND_CONFIG_BANDWIDTH_2040MHZ);
    wifi_config_set_wireless_mode(WIFI_PORT_AP, WIFI_PHY_11BG_MIXED);
    wifi_config_reload_setting();
    network_dhcp_start(WIFI_MODE_REPEATER);
	return 1;
}

static int wifi_scan_event_handler(wifi_event_t event_id, unsigned char *payload, unsigned int len) {
    if(event_id == WIFI_EVENT_IOT_SCAN_COMPLETE) {
        int i;
        //int count = 0;

        for(i=0;i<4;i++) {
            APPLOG("ssid %d, %s, %d", i, g_ap_list[i].ssid, g_ap_list[i].channel);
            if(strcmp((char*)ginSSID, (char*)g_ap_list[i].ssid) == 0) {
                wifi_connection_unregister_event_handler(WIFI_EVENT_IOT_SCAN_COMPLETE, (wifi_event_handler_t) wifi_scan_event_handler);
                ginChannel = g_ap_list[i].channel;
                ginSanDone = 1;
                break;
            }
        }
        os_memset(g_ap_list, 0, sizeof(g_ap_list));
    }
    return 0;
}

void wifi_register_scan_event(ap_passport_t *args) {
    uint8_t *ssid = (uint8_t *)(args->ssid);
    strcpy((char*)ginSSID,(char*)ssid);
    strcpy((char*)ginPassword,(char *)(args->psk));
    wifi_config_set_opmode(WIFI_MODE_STA_ONLY);
    wifi_config_reload_setting();
    //wifi_connection_scan_init(g_ap_list, 16);
    //wifi_connection_start_scan(NULL, 0, NULL, 0, 2);
    wifi_connection_register_event_handler(WIFI_EVENT_IOT_SCAN_COMPLETE, (wifi_event_handler_t) wifi_scan_event_handler);
}

//Lelink lib get ssid & passwd;
int halDoApConnect(void *ptr, int ptrLen) {
    int ret = 0;
    if (ptr) {
        PrivateCfg *privateCfg = (PrivateCfg *)ptr;//ssid ,passwd;
        if (ginFirstInFlag) {
            APPLOG("halDoApConnect ssid[%s] pwd[%s]", (char *)(privateCfg->data.nwCfg.config.ssid), (char *)(privateCfg->data.nwCfg.config.psk));
            ginFirstInFlag = 0;
            if(haalIsRepeater()) {
                ap_passport_t passport;
                os_memset(&passport, 0, sizeof(passport));
                os_strncpy(passport.ssid, (char *)(privateCfg->data.nwCfg.config.ssid), 36);
                os_strncpy(passport.psk, (char *)(privateCfg->data.nwCfg.config.psk), 36);
                wifi_register_scan_event(&passport);
            } else {
                ret = 1;
                wifi_station_start_to_connect((char*)(privateCfg->data.nwCfg.config.ssid), (char *)(privateCfg->data.nwCfg.config.psk));
            }
        }
        if(ginSanDone) {
            ginSanDone = 0;
            wifi_repeater_start_to_connect(ginSSID, ginPassword, ginChannel);
            ret = 1;
        }
    }
    return ret;
}

int halDoApConnecting(void *ptr, int ptrLen) {
    // APPLOG("halDoApConnecting ret[%d]", gin_airconfig_ap_connected);
    ginFirstInFlag = 1;
    return gin_airconfig_ap_connected;
}

int halGetWifiChannel() {
    return ginChannel;
}

extern int g_supplicant_ready;

void lelink_softap_setup(char *ssid, char *wpa2_passphrase) {
	/*
	 * wilress params: 11BGN
	 * channel: auto, or 1, 6, 11
	 * authentication: OPEN
	 * encryption: NONE
	 * gatewayip: 172.31.254.250, netmask: 255.255.255.0
	 * DNS server: 172.31.254.250.	IMPORTANT!!!  ios depend on it!
	 * DHCP: enable
	 * SSID: 32 ascii char at most
	 * softap timeout: 5min
	 */
    wifi_auth_mode_t auth = WIFI_AUTH_MODE_WPA_PSK_WPA2_PSK;
    wifi_encrypt_type_t encrypt = WIFI_ENCRYPT_TYPE_TKIP_AES_MIX;

	wifi_config_set_opmode(WIFI_MODE_AP_ONLY);
    //wifi_config_set_security_mode(WIFI_PORT_AP, WIFI_AUTH_MODE_OPEN, WIFI_ENCRYPT_TYPE_WEP_DISABLED);
	wifi_config_set_security_mode(WIFI_PORT_AP, auth, encrypt);
	wifi_config_set_channel(WIFI_PORT_AP, 6);
	wifi_config_set_ssid(WIFI_PORT_AP, (uint8_t *)ssid, os_strlen(ssid));
	wifi_config_set_wpa_psk_key(WIFI_PORT_AP, (uint8_t *)wpa2_passphrase, (uint8_t)os_strlen(wpa2_passphrase));
	wifi_config_reload_setting();
    g_softap_start = 1;
	while(!g_supplicant_ready){
		vTaskDelay(20);
	}

	{
		char ip_buf[] = "192.168.10.1";
		char mask_buf[] = "255.255.255.0";
		char start_ip[] = "192.168.10.2";
		char end_ip[] = "192.168.10.254";
		char primary_dns[] = "192.168.10.1";
		char secondary_dns[] = "8.8.4.4";
		struct ip4_addr addr;
		struct netif *sta_if;
		struct netif *ap_if;
		ap_if = netif_find_by_type(NETIF_TYPE_AP);		   
		sta_if = netif_find_by_type(NETIF_TYPE_STA);

		netif_set_status_callback(ap_if, NULL);

		inet_aton(mask_buf, &addr);
		netif_set_netmask(ap_if, &addr);
		inet_aton(ip_buf, &addr);
		netif_set_ipaddr(ap_if, &addr);
		netif_set_gw(ap_if, &addr);

		dhcp_stop(sta_if);
		netif_set_link_up(ap_if);
		netif_set_default(ap_if);
		dhcpd_settings_t dhcpd_settings;
		os_memset(&dhcpd_settings, 0, sizeof(dhcpd_settings_t));
		os_strncpy((char *)dhcpd_settings.dhcpd_server_address, ip_buf, sizeof(ip_buf));
		os_strncpy((char *)dhcpd_settings.dhcpd_netmask, mask_buf, sizeof(mask_buf));
		os_strncpy((char *)dhcpd_settings.dhcpd_gateway, ip_buf, sizeof(ip_buf));
		os_strncpy((char *)dhcpd_settings.dhcpd_primary_dns, primary_dns, sizeof(primary_dns));
		os_strncpy((char *)dhcpd_settings.dhcpd_secondary_dns, secondary_dns, sizeof(secondary_dns));
		os_strncpy((char *)dhcpd_settings.dhcpd_ip_pool_start, start_ip, sizeof(start_ip));
		os_strncpy((char *)dhcpd_settings.dhcpd_ip_pool_end, end_ip, sizeof(end_ip));
		dhcpd_start(&dhcpd_settings);
		APPLOG("start dhcpd, stop dhcp. g_supplicant_ready:%d\n", g_supplicant_ready);
	}
}

void lelink_softap_exit(void) {
/*
    dhcpd_stop();
    wifi_config_set_opmode(WIFI_MODE_STA_ONLY);

    dhcp_start(&sta_if);
    netif_set_link_up(&sta_if);
 */
    struct netif *sta_if;
    struct netif *ap_if;
    if(!g_softap_start) {
        return;
    }
    g_softap_start = 0;
    ap_if = netif_find_by_type(NETIF_TYPE_AP);   
    dhcpd_stop();
    netif_set_link_down(ap_if);

    sta_if = netif_find_by_type(NETIF_TYPE_STA);
    netif_set_default(sta_if);
    // netif_set_status_callback(sta_if, ip_ready_callback);
    dhcp_start(sta_if);
}

int halSoftApStart(const char *ssid, char *wpa2_passphrase, uint8_t *aesKey, int aesKeyLen) {
	int ret = 0;
	/* prepare and setup softap */
    // const static uint8_t testKey[] = { 0x4d, 0x90, 0xc5, 0x2b, 0xea, 0x52, 0x59, 0xb9, 0x5b, 0x53, 0xd3, 0x3c, 0x63, 0xa7, 0x06, 0xe2 }; 
    // os_memcpy(aesKey, testKey, aesKeyLen);
	lelink_softap_setup(ssid, wpa2_passphrase);
	return ret;
}

int halSoftApStop(int success) {
    lelink_softap_exit();
    return 0;
}
