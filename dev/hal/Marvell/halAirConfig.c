#include "halHeader.h"
#include <lelink/sw/io.h>
#include <lelink/sw/airconfig.h>

extern int airconfig_start(void *pc, uint8_t *prov_key, int prov_key_len);
extern int airconfig_stop();
extern void inner_set_ap_info(const ap_passport_t *passport);
extern int8_t gin_airconfig_ap_connected;
extern int8_t gin_airconfig_sniffer_got;
extern struct wlan_network gin_sta_net;
static int startApListen(void);

int halDoConfig(void *ptr, int ptrLen) {
	int ret;

#if ENABLE_WIFI_SOFT_AP
    ret = !startApListen();
#else
    if (gin_airconfig_sniffer_got) {
        gin_airconfig_sniffer_got = 0;
        app_sta_stop();
    }
	ret = airconfig_start(NULL, NULL, 0);
#endif
    APPLOG("halDoConfig in hal [%d]", ret);
	return ret;
}

int halDoConfiguring(void *ptr, int ptrLen) {
    APPLOG("halDoConfiguring in hal [%d]", gin_airconfig_sniffer_got);
	return gin_airconfig_sniffer_got;
}

int halDoApConnect(void *ptr, int ptrLen) {
    int ret = 0;
    if (ptr) {
        ap_passport_t passport;
        PrivateCfg *privateCfg = (PrivateCfg *)ptr;
        gin_airconfig_sniffer_got = 1;
        strcpy(passport.ssid, (char *)(privateCfg->data.nwCfg.config.ssid));
        strcpy(passport.psk, (char *)(privateCfg->data.nwCfg.config.psk));
        inner_set_ap_info(&passport);
    }
    ret = app_sta_start_by_network(&gin_sta_net);
    APPLOG("halDoApConnect in hal[%d]", ret);
    if (WM_SUCCESS == ret) {
    	ret = 1;
    }
    else {
    	ret = -1;
    }
    return ret;
}

int halDoApConnecting(void *ptr, int ptrLen) {
    APPLOG("halDoApConnecting in hal[%d]", gin_airconfig_ap_connected);
    return gin_airconfig_ap_connected;
}



//
//int ezconn_init()
//{
	//return 0;
//}
//
///* for device */
//int ezconn_sniffer_start()
//{
	//return 0;
	//
//}
//int ezconn_sniffer_stop()
//{
	//return 0;
	//
//}
//
//void ezconn_exit()
//{
	//
//}
//
//
extern int softApStarted(void);
static int thread_uapconfig_run = 0;
static os_thread_t thread_uapconfig;
static os_thread_stack_define(thread_stack_uapconfig, 1024 * 10);
static void thread_uapconfig_proc(os_thread_arg_t thandle);

static void thread_uapconfig_proc(os_thread_arg_t thandle)
{
    thread_uapconfig_run = 1;
    gin_airconfig_sniffer_got = !softApStarted();
    os_thread_self_complete((os_thread_t *)thandle);
    thread_uapconfig_run = 0;
}

static int startApListen(void)
{
    int ret = 0;

    if(!thread_uapconfig_run) {
        ret = os_thread_create(&thread_uapconfig,
                "uapconfig",
                thread_uapconfig_proc,
                (void *)&thread_uapconfig,
                &thread_stack_uapconfig,
                OS_PRIO_3);
    }
    return ret;
}

int halSoftApStart(char *ssid, char *wpa2_passphrase)
{
    int ret;

    ret = app_uap_start_with_dhcp(ssid, wpa2_passphrase);
    if(ret < 0) {
        APPLOGE("Failed to app_uap_start_with_dhcp");
        return -1;
    }
    return ret;
}

int halSoftApStop(void)
{
    return app_uap_stop();
}

