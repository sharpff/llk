#ifndef __MISC_H__
#define __MISC_H__

#include "leconfig.h"
#include "network.h"

int isNeedToRedirect(const char *json, int jsonLen, char ip[MAX_IPLEN], uint16_t *port);
int syncUTC(const char *json, int jsonLen);
int getUartInfo(const char *json, int jsonLen, int *baud, int *dataBits, int *stopBits, char *parity, int *flowCtrl);
int getWhatCvtType(const char *json, int jsonLen);
int getJsonUTC32(char *json, int jsonLen);


#endif