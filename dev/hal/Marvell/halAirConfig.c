#include "halHeader.h"

extern int airconfig_start(void *pc, uint8_t *prov_key, int prov_key_len);
extern int airconfig_stop();
extern int8_t gin_airconfig_ap_connected;
extern int8_t gin_airconfig_sniffer_got;
extern struct wlan_network gin_sta_net;


int halDoConfig(void *ptr, int ptrLen) {
	int ret;
	ret = airconfig_start(NULL, NULL, 0);
    APPLOG("halDoConfig in hal [%d]\r\n", ret);
	return ret;
}

int halDoConfiguring(void *ptr, int ptrLen) {
    APPLOG("halDoConfiguring in hal [%d]\r\n", gin_airconfig_sniffer_got);
	return gin_airconfig_sniffer_got;
}

int halDoApConnect(void *ptr, int ptrLen) {
	int ret = 0;
    ret = app_sta_start_by_network(&gin_sta_net);
    APPLOG("halDoApConnect in hal[%d]\r\n", ret);
    if (WM_SUCCESS == ret) {
    	ret = 1;
    }
    else {
    	ret = -1;
    }
    return ret;
}

int halDoApConnecting(void *ptr, int ptrLen) {
    APPLOG("halDoApConnecting in hal[%d]\r\n", gin_airconfig_ap_connected);
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
