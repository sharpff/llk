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

typedef enum {
	CLOUD_MSG_KEY_NONE = 0,
	CLOUD_MSG_KEY_LOCK,
	CLOUD_MSG_KEY_DO_IA,
	CLOUD_MSG_KEY_DO_SHARE
}CloudMsgKey;

int isNeedToRedirect(const char *json, int jsonLen, char ip[MAX_IPLEN], uint16_t *port);
int syncUTC(const char *json, int jsonLen);
int getUartInfo(const char *json, int jsonLen, int *baud, int *dataBits, int *stopBits, char *parity, int *flowCtrl);
int getGPIOInfo(const char *json, int jsonLen, gpioHand_t *table, int n);
int getPipeInfo(const char *json, int jsonLen, char *name, int size);
int getWhatCvtType(const char *json, int jsonLen);
int getJsonUTC32(char *json, int jsonLen/*, const char *rmtJson, int rmtJsonLen*/);

int cloudMsgHandler(const char *data, int len);

#endif
