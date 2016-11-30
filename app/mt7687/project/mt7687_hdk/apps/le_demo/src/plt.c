
#include "leconfig.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "halHeader.h"

typedef enum {
    E_PLT_TYPE_IDLE, 
    E_PLT_TYPE_CTRL,
    E_PLT_TYPE_VERSION, 
    E_PLT_TYPE_OTA, 
    E_PLT_TYPE_REBOOT,
    E_PLT_TYPE_PERMITJOIN, 
    E_PLT_TYPE_MAX,
}E_PLT_TYPE;

extern int sengineSetStatus(char *json, int jsonLen);
extern int getVer(char fwVer[64], int size);
extern int halGetMac(uint8_t *mac, int len);
static const char *cmdFormat = "{\"light\":%d,\"mode\":%d,\"timeout\":%d,\"brightness\":%d,\"red\":%d,\"green\":%d,\"blue\":%d}";
//static const char *cmdFormat = "{\"pwm\":[{\"id\":%d,\"val\":%d},{\"id\":%d,\"val\":%d},{\"id\":%d,\"val\":%d},{\"id\":%d,\"val\":%d}]}";
static char cmdBuff[1024];
static int cmdType = 0;

uint8_t le_ledon(uint8_t len, char *param[]) {
    memset(cmdBuff, 0, 1024);
    sprintf(cmdBuff, cmdFormat, 1, 0, 0, 1024, 1024, 1024, 1024);
    cmdType = E_PLT_TYPE_CTRL;
    return 0;
}
    
uint8_t le_ledoff(uint8_t len, char *param[]) {
    memset(cmdBuff, 0, 1024);
    sprintf(cmdBuff, cmdFormat, 0, 0, 0, 0, 0, 0, 0);
    cmdType = E_PLT_TYPE_CTRL;
    return 0;
}

uint8_t le_ledset(uint8_t len, char *param[]) {
    int type = 1;
    int duty = 2;
    if (len < 2) {
        APPLOGE("error, format is [le ledset 1 1024]\n");
        return 0;
    }
    type = atoi(param[0]);
    duty = atoi(param[1]);
    memset(cmdBuff, 0, 1024);
    if(type == 1) {
        sprintf(cmdBuff, cmdFormat, 1, 0, 0, 1024, duty, 0, 0);
    } else if (type == 2) {
        sprintf(cmdBuff, cmdFormat, 1, 0, 0, 1024, 0, duty, 0);
    } else if (type == 3) {
        sprintf(cmdBuff, cmdFormat, 1, 0, 0, 1024, 0, 0, duty);
    } else if (type == 4) {
        sprintf(cmdBuff, cmdFormat, 1, 1, 10, 0, 0, 0, 0);
    } else if (type == 5) {
        sprintf(cmdBuff, cmdFormat, 1, 2, 10, 0, 0, 0, 0);
    } else if (type == 6) {
        sprintf(cmdBuff, cmdFormat, 1, 3, 10, 0, 0, 0, 0);
    } else if (type == 7) {
        sprintf(cmdBuff, cmdFormat, 1, 4, 10, 0, 0, 0, 0);
    } else {
        APPLOGE("type error\n");
        return 0;
    }
    cmdType = E_PLT_TYPE_CTRL;
    return 0;
}

uint8_t le_ledsetRGB(uint8_t len, char *param[]) {
    int R, G, B;
    if (len < 3) {
        APPLOGE("error, format is [le ledsetRGB 1024 1024 1024]\n");
        return 0;
    }

    R = atoi(param[0]);
    G = atoi(param[1]);
    B = atoi(param[2]);
    memset(cmdBuff, 0, 1024);
    sprintf(cmdBuff, cmdFormat,  1, 0, 0, 1024, R, G, B);
    cmdType = E_PLT_TYPE_CTRL;
    return 0;
}

uint8_t le_permitjoin(uint8_t len, char *param[]) {
	cmdType = E_PLT_TYPE_PERMITJOIN;
    return 0;
}

uint8_t le_reboot(uint8_t len, char *param[]) {
	cmdType = E_PLT_TYPE_REBOOT;
    return 0;
}

uint8_t le_version(uint8_t len, char *param[]) {
	cmdType = E_PLT_TYPE_VERSION;
    return 0;
}

static uint8_t lelinkGetSN(uint8_t* data) {
	uint32_t valid_bit;
	SYSefuse_Read16Bytes(0x80, (uint32_t *)data, &valid_bit, 0);
	if (valid_bit == 0) {
    	return 0;
    }
	SYSefuse_Read16Bytes(0x90, (uint32_t *)data+16, &valid_bit, 0);
	if (valid_bit == 0) {
    	return 0;
    }
    return 32;
}

void lelinkPltCtrlProcess(void) {
    switch (cmdType) {
        case E_PLT_TYPE_CTRL: {
        	cmdType = E_PLT_TYPE_IDLE;
        	APPLOG("lelinkPltCtrlProcess ctrl\n");
            sengineSetStatus(cmdBuff, strlen(cmdBuff));
        }
        break;
        case E_PLT_TYPE_VERSION: {
        	char fwVer[64] = {0}, count;
		    uint8_t mac[8] = {0};
		    cmdType = E_PLT_TYPE_IDLE;
		    APPLOG("---------lelinkInfo----------\r\n");
		    getVer(fwVer, sizeof(fwVer));
		    APPLOG("ver: %s", fwVer);
		    halGetMac(mac, sizeof(mac));
		    APPLOG("mac: %02x:%02x:%02x:%02x:%02x:%02x", 
		        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		    memset(fwVer, 0 , 64);
		    count = lelinkGetSN(fwVer);
		    if(count) {
			    APPLOG("sn:  %s", fwVer);
		    } else {
		    	APPLOG("sn: data error");
		    }
		    APPLOG("-----------------------------\r\n");
        }
        break;
        case E_PLT_TYPE_OTA: {
            cmdType = E_PLT_TYPE_IDLE;
        }
        break;
        case E_PLT_TYPE_PERMITJOIN: {
        	//uint8_t cmd[15] = {0x01, 0x02, 0x10, 0x49, 0x02, 0x10, 0x02, 0x14, 0x7E, 0xFF, 0xFC, 0x30, 0x02, 0x10, 0x03};
   			//halUartWrite(&uartHandler, cmd, 15);
            cmdType = E_PLT_TYPE_IDLE;
        }
        break;
        case E_PLT_TYPE_REBOOT: {
            cmdType = E_PLT_TYPE_IDLE;
            halReboot();
        }
        break;
        default: 
        	break;
    }
}
