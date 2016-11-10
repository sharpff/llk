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

void lelinkPltCtrlProcess(void) {
    switch (cmdType) {
        case E_PLT_TYPE_CTRL: {
        	cmdType = E_PLT_TYPE_IDLE;
        	APPLOG("lelinkPltCtrlProcess ctrl\n");
            sengineSetStatus(cmdBuff, strlen(cmdBuff));
        }
        break;
        case E_PLT_TYPE_VERSION: {
        	char fwVer[64] = {0};
            cmdType = E_PLT_TYPE_IDLE;
		    getVer(fwVer, sizeof(fwVer));
		    APPLOG("======== version ========\n");
		    APPLOG("firmware: %s\r\n", fwVer);
		    APPLOG("======== version ========\n");
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
