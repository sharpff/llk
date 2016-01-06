#ifndef AIRCONFIG_H
#define AIRCONFIG_H

#include "leconfig.h"

#define MAX_CHANNEL_CARE 4

typedef enum {
    AIRCONFIG_NW_STATE_NONE,
    AIRCONFIG_NW_STATE_ERROR_PARAM,
    AIRCONFIG_NW_STATE_CHANNEL_LOCKED
}airconfig_nw_state_t;

/*
 * to fill the item by sniffer info
 * data: the length or current package
 */
typedef struct {
    uint16_t data;
    uint8_t mac_src[6];
    uint8_t mac_dst[6];
    uint8_t mac_bssid[6];
}target_item_t;

/*
 * the max ssid or passwd is 32 byes
 */
typedef struct {
	char ssid[36];
	char passwd[36];
}ap_passport_t;

int airconfig_do_sync(const target_item_t *item, int channel, int channel_locked[MAX_CHANNEL_CARE], uint16_t *base);
int airconfig_get_info(int len, int base, ap_passport_t *account, const char *ssid, int len_ssid);
int airconfig_reset(void);

#endif /* AIRCONFIG_H */
