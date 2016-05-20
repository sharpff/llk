#ifndef __MISC_H__
#define __MISC_H__

#include "leconfig.h"
#include "network.h"
#include "io.h"

typedef enum {
	CLOUD_MSG_KEY_NONE = 0,
	CLOUD_MSG_KEY_LOCK,
	CLOUD_MSG_KEY_DO_IA,
	CLOUD_MSG_KEY_DO_SHARE
}CloudMsgKey;

int isNeedToRedirect(const char *json, int jsonLen, char ip[MAX_IPLEN], uint16_t *port);
int syncUTC(const char *json, int jsonLen);
int getUartInfo(const char *json, int jsonLen, int *baud, int *dataBits, int *stopBits, char *parity, int *flowCtrl);
int getGPIOInfo(const char *json, int jsonLen,  gpioHand_t *table, int n);
int getWhatCvtType(const char *json, int jsonLen);
int getJsonUTC32(char *json, int jsonLen/*, const char *rmtJson, int rmtJsonLen*/);

int cloudMsgHandler(const char *data, int len);

#endif
