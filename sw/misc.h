#ifndef __MISC_H__
#define __MISC_H__

#include "leconfig.h"
#include "network.h"
#include "io.h"

int isNeedToRedirect(const char *json, int jsonLen, char ip[MAX_IPLEN], uint16_t *port);
int syncUTC(const char *json, int jsonLen);
int getUartInfo(const char *json, int jsonLen, int *baud, int *dataBits, int *stopBits, char *parity, int *flowCtrl);
int getGPIOInfo(const char *json, int jsonLen,  gpioHand_t *table, int n);
int getWhatCvtType(const char *json, int jsonLen);
int getJsonUTC32(char *json, int jsonLen/*, const char *rmtJson, int rmtJsonLen*/);


#endif
