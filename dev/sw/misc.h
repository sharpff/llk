#ifndef __MISC_H__
#define __MISC_H__

#include "leconfig.h"
#include "network.h"

int isNeedToRedirect(char *json, int jsonLen, char ip[MAX_IPLEN], uint16_t *port);

#endif