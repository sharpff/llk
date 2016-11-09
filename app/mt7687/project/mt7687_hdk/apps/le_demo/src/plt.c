
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
extern int bytes2hexStr(const uint8_t *src, int srcLen, uint8_t *dst, int dstLen);

static const char *cmdFormat = "{\"pwm\":[{\"id\":%d,\"val\":%d},{\"id\":%d,\"val\":%d},{\"id\":%d,\"val\":%d},{\"id\":%d,\"val\":%d}]}";
static char cmdBuff[1024];
static int cmdType = 0;

uint8_t le_ledon(uint8_t len, char *param[]) {
    memset(cmdBuff, 0, 1024);
    sprintf(cmdBuff, cmdFormat, 33, 1024, 34, 1024, 35, 1024, 18, 1024);
    cmdType = E_PLT_TYPE_CTRL;
    return 0;
}
    
uint8_t le_ledoff(uint8_t len, char *param[]) {
    memset(cmdBuff, 0, 1024);
    sprintf(cmdBuff, cmdFormat, 33, 1024, 34, 1024, 35, 1024, 18, 0);
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
        sprintf(cmdBuff, cmdFormat, 33, duty, 34, 0, 35, 0, 18, 1024);
    } else if (type == 2) {
        sprintf(cmdBuff, cmdFormat, 33, 0, 34, duty, 35, 0, 18, 1024);
    } else if (type == 3) {
        sprintf(cmdBuff, cmdFormat, 33, 0, 34, 0, 35, duty, 18, 1024);
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
    sprintf(cmdBuff, cmdFormat, 33, G, 34, B, 35, R, 18, 1024);
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
	uint8_t buff[16];
	SYSefuse_Read16Bytes(0, (uint32_t *)buff, &valid_bit, 0);
	if (valid_bit == 0) {
    	return 0;
    }
    memcpy(data, buff, 16);
	SYSefuse_Read16Bytes(16, (uint32_t *)buff, &valid_bit, 0);
	if (valid_bit == 0) {
    	return 0;
    }
    memcpy(data+16, buff, 16);
    SYSefuse_Read16Bytes(32, (uint32_t *)buff, &valid_bit, 0);
	if (valid_bit == 0) {
    	return 0;
    }
    memcpy(data+32, buff, 2);
    return 34;
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
		    uint8_t sn[36] = {0};
		    uint8_t sn_hex[72] = {0};
		    cmdType = E_PLT_TYPE_IDLE;
		    APPLOG("---------lelinkInfo----------\r\n");
		    getVer(fwVer, sizeof(fwVer));
		    APPLOG("ver: %s", fwVer);
		    halGetMac(mac, sizeof(mac));
		    APPLOG("mac: %02x:%02x:%02x:%02x:%02x:%02x", 
		        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		    count = lelinkGetSN(sn);
		    if(count) {
		    	bytes2hexStr(sn, count, sn_hex, sizeof(sn_hex));
			    APPLOG("sn: %s", sn_hex);
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
