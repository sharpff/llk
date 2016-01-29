#ifndef AIRCONFIG_CTRL_H
#define AIRCONFIG_CTRL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "leconfig.h"

typedef enum {
    AIRCONFIG_CTRL_NONE,
    AIRCONFIG_CTRL_ERROR_PARAM,
    AIRCONFIG_CTRL_DONE
}airconfig_nw_state_ctrl_t;


/*
 * SSID
 * PASSWD
 * AES
 * TYPE: 1-multicast, 2-broadcast
 * DELAY: 1000 ms or ...
 * E.g. SSID=letv-office,PASSWD=987654321,AES=912EC803B2CE49E4A541068D495AB570,TYPE=1,DELAY=1000
 */
void *airconfig_new(const char *param);
int airconfig_do_config(void *context);
void airconfig_delete(void *context);

// #define DEBUG_AIR_CONFIG

#ifdef __cplusplus
}
#endif

#endif /* AIRCONFIG_H */
