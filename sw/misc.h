#ifndef __MISC_H__
#define __MISC_H__

#include "leconfig.h"
#include "network.h"
#include "io.h"

#define NUM_TOKENS 256

// 
#define JSON_NAME_REDIRECT "redirect"
#define JSON_NAME_IP "IP"
#define JSON_NAME_PORT "port"
#define JSON_NAME_UTC "utc"
#define JSON_NAME_WHATTYPE "whatCvtType"
#define JSON_NAME_UART_CONF         "uart"
#define JSON_NAME_UART_BAUD "baud"
#define JSON_NAME_UUID "uuid"
#define JSON_NAME_URL "url"
#define JSON_NAME_TYPE "type"
#define JSON_NAME_GPIO_CONF         "gpio"
#define JSON_NAME_GPIO_ID           "id"
#define JSON_NAME_GPIO_DIR          "dir"
#define JSON_NAME_GPIO_MODE         "mode"
#define JSON_NAME_GPIO_BLINK        "blink"
#define JSON_NAME_GPIO_STATE        "state"
#define JSON_NAME_GPIO_TYPE         "type"
#define JSON_NAME_GPIO_TIME_SHORT   "shortTime"
#define JSON_NAME_GPIO_TIME_LONG    "longTime"
#define JSON_NAME_PIPE_NAME         "name"
#define JSON_NAME_KEY               "key"
#define JSON_NAME_VAL               "val"
#define JSON_NAME_NAME              "name"
#define JSON_NAME_LOCK              "lock"
#define JSON_NAME_CTRL              "ctrl"
#define JSON_NAME_STATUS            "status"
#define JSON_NAME_SDEV     			"sDev"
#define JSON_NAME_SDEV_GET_LIST     "sDevGetList"
#define JSON_NAME_SDEV_GET_INFO     "sDevGetInfo"
#define JSON_NAME_SDEV_QUERY_EPT    "sDevQryEpt"
#define JSON_NAME_SDEV_QUERY_MAN    "sDevQryMan"
#define JSON_NAME_SDEV_QUERY_INFO   "sDevQryEptInfo"
#define JSON_NAME_SDEV_STATUS       "sDevStatus"
#define JSON_NAME_SDEV_JOIN         "sDevJoin"
#define JSON_NAME_SDEV_INDEX        "idx"
#define JSON_NAME_SDEV_EPT          "ept"
#define JSON_NAME_SDEV_MAN          "man"
#define JSON_NAME_SDEV_PID          "pid"
#define JSON_NAME_SDEV_DID          "did"
#define JSON_NAME_SDEV_CLU          "clu"
#define JSON_NAME_SDEV_MAC		    "mac"
#define JSON_NAME_LOG2MASTER        "log2M"

typedef enum {
	CLOUD_MSG_KEY_NONE = 0,
	CLOUD_MSG_KEY_LOCK,
	CLOUD_MSG_KEY_DO_IA,
	CLOUD_MSG_KEY_DO_SHARE, // reserved for business
	CLOUD_MSG_KEY_DO_IA_OK, // reserved for app sdk
	CLOUD_MSG_KEY_DO_STATUS_CHANGED, // reserved for app sdk
	CLOUD_MSG_KEY_LOG2MASTER,
	CLOUD_MSG_KEY_REDIRECT_AP, // only remote ctrl 
}CloudMsgKey;

int isNeedToRedirect(const char *json, int jsonLen, char ip[MAX_IPLEN], uint16_t *port);
int syncUTC(const char *json, int jsonLen);
int getUartInfo(const char *json, int jsonLen, int *baud, int *dataBits, int *stopBits, char *parity, int *flowCtrl);
int getGPIOInfo(const char *json, int jsonLen, gpioHand_t *table, int n);
int getPipeInfo(const char *json, int jsonLen, char *name, int size);
int getWhatCvtType(const char *json, int jsonLen);
int getJsonUTC32(char *json, int jsonLen/*, const char *rmtJson, int rmtJsonLen*/);

int cloudMsgHandler(const char *data, int len);

int printOut(const char *fmt, ...);

#endif
