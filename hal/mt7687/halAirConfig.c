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

#if ENABLE_WIFI_SOFT_AP
static int startApListen(void);
#endif

extern int8_t gin_airconfig_sniffer_got;
extern int8_t gin_airconfig_ap_connected;
extern void network_dhcp_start(uint8_t opmode);


int halDoConfig(void *ptr, int ptrLen) {
	int ret;

#if ENABLE_WIFI_SOFT_AP
    ret = !startApListen();
#endif
  
    if (gin_airconfig_sniffer_got) 
	{
        gin_airconfig_sniffer_got = 0;
        //app_sta_stop();//not need to stop STA;
    }
   #if 0
	ret = airconfig_start(NULL, NULL, 0);
    APPLOG("halDoConfig in hal [%d]", ret);
   #endif
	return ret;
}

int halDoConfiguring(void *ptr, int ptrLen) {
  #ifndef __MTK_MT7687_PLATFORM__
    APPLOG("halDoConfiguring in hal [%d]", gin_airconfig_sniffer_got);
  #else
    printf("halDoConfiguring in hal [%d]", gin_airconfig_sniffer_got);
  #endif
  
	return gin_airconfig_sniffer_got;
}

int wifi_connect_sta_start(ap_passport_t *args)
{
    LOG_I(common, "Enter connection init.");
    uint8_t opmode  = WIFI_MODE_STA_ONLY;
    uint8_t port = WIFI_PORT_STA;
    uint8_t *ssid = (uint8_t *)(args->ssid);
    wifi_auth_mode_t auth = WIFI_AUTH_MODE_WPA_PSK_WPA2_PSK;
    wifi_encrypt_type_t encrypt = WIFI_ENCRYPT_TYPE_TKIP_AES_MIX;
    uint8_t *password = (uint8_t *)(args->psk);
    uint8_t nv_opmode;

    if (1) 
	{
        wifi_config_get_opmode(&nv_opmode);
        if (nv_opmode != opmode) {
            wifi_config_set_opmode(opmode);
        }
        wifi_config_set_ssid(port, ssid ,(uint8_t)os_strlen((char *)ssid));
        wifi_config_set_security_mode(port, auth, encrypt);
        wifi_config_set_wpa_psk_key(port, password, (uint8_t)os_strlen((char *)password));
        wifi_config_reload_setting();

        network_dhcp_start(opmode);

		return 1;
    }
	else
	{
	   return -1;
	}
}

//Lelink lib get ssid & passwd;
int halDoApConnect(void *ptr, int ptrLen) 
{
    int ret = 0;
	ap_passport_t passport;
	os_memset(&passport, 0, sizeof(passport));
    if (ptr) {
        //ap_passport_t passport;
        PrivateCfg *privateCfg = (PrivateCfg *)ptr;//ssid ,passwd;
        gin_airconfig_sniffer_got = 1;
        os_strncpy(passport.ssid, (char *)(privateCfg->data.nwCfg.config.ssid), 36);
        os_strncpy(passport.psk, (char *)(privateCfg->data.nwCfg.config.psk), 36);
        //inner_set_ap_info(&passport);//get ssid & passwd,then switch to STA;
    }
    //ret = app_sta_start_by_network(&gin_sta_net);
    ret = wifi_connect_sta_start(&passport);
	
    /*APPLOG("halDoApConnect in hal[%d]", ret);
    if (WM_SUCCESS == ret) {
    	ret = 1;
    }
    else {
    	ret = -1;
    }*/
    
    return ret;
}

int halDoApConnecting(void *ptr, int ptrLen) {
    APPLOG("halDoApConnecting in hal[%d]", gin_airconfig_ap_connected);
    return gin_airconfig_ap_connected;
}

extern int g_supplicant_ready;

void lelink_softap_setup(char *ssid, char *wpa2_passphrase)
{
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

	while(!g_supplicant_ready){
		vTaskDelay(20);
	}

  #if 1
	if(1)
	{
		char ip_buf[] = "172.31.254.250";
		char mask_buf[] = "255.255.255.0";
		char start_ip[] = "172.31.254.251";
		char end_ip[] = "172.31.254.254";
		char primary_dns[] = "172.31.254.250";
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
	   
		printf("start dhcpd, stop dhcp. g_supplicant_ready:%d\n", g_supplicant_ready);
	}
  #endif//0
}

void lelink_softap_exit(void)
{
/*
    dhcpd_stop();
    wifi_config_set_opmode(WIFI_MODE_STA_ONLY);

    dhcp_start(&sta_if);
    netif_set_link_up(&sta_if);
 */
    struct netif *sta_if;
    struct netif *ap_if;
    ap_if = netif_find_by_type(NETIF_TYPE_AP);   
    dhcpd_stop();
    netif_set_link_down(ap_if);

    sta_if = netif_find_by_type(NETIF_TYPE_STA);
    netif_set_default(sta_if);
    // netif_set_status_callback(sta_if, ip_ready_callback);
    dhcp_start(sta_if);
}

extern int softApStarted(void);

static int thread_uapconfig_run = 0;
//static os_thread_t thread_uapconfig;
//static os_thread_stack_define(thread_stack_uapconfig, 1024 * 10);
//static void thread_uapconfig_proc(os_thread_arg_t thandle);
static void task_apconfig_proc(void *not_used)
{
    thread_uapconfig_run = 1;
    gin_airconfig_sniffer_got = !softApStarted();
    vTaskDelete(NULL);
    thread_uapconfig_run = 0;
}

static int startApListen(void)
{
    int ret = 0;

    if(!thread_uapconfig_run) 
	{
        /*ret = os_thread_create(&thread_uapconfig,
                "uapconfig",
                thread_uapconfig_proc,
                (void *)&thread_uapconfig,
                &thread_stack_uapconfig,
                OS_PRIO_3);*/
		if(pdPASS != xTaskCreate(task_apconfig_proc,
                              "thread_uapconfig_proc",
                              1024,
                              NULL,
                              1,
                              NULL))
            return -1;                
                       
    }
    return ret;
}

int halSoftApStart(char *ssid, char *wpa2_passphrase) {
	int ret = 0;
	
	/* prepare and setup softap */
	lelink_softap_setup(ssid, wpa2_passphrase);

  #if 0
	/* tcp server to get ssid & passwd */
	aws_softap_tcp_server();

	strncpy(ssid_buf, aws_ssid, os_strlen(aws_ssid));
	ssid_buf[os_strlen(aws_ssid)] = 0;
	strncpy(passwd_buf, aws_passwd, os_strlen(aws_passwd));
	passwd_buf[os_strlen(aws_passwd)] = 0;
	
	lelink_softap_exit(); 
  #endif//0
  
	return ret;
}

int halSoftApStop(void) {
    lelink_softap_exit();
	
    return 0;
}

