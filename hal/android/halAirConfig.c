#include "halHeader.h"


int halDoConfig(void *ptr, int ptrLen) {
	return 1;
}

int halStopConfig(void) {
	return 1;
}

int halDoConfiguring(void *ptr, int ptrLen) {
	return 1;
}

int halDoApConnect(void *ptr, int ptrLen) {
    return 1;
}

int halDoApConnecting(void *ptr, int ptrLen) {
    return 1;
}

int halSoftApStart(char *ssid, char *wpa2_passphrase) {
    return 0;
}

int halSoftApStop(void) {
    return 0;
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
