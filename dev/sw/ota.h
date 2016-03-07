#ifndef _OTA_H_
#define _OTA_H_

#include "leconfig.h"
#include "sengine.h"

typedef enum {
    OTA_TYPE_FW,
    OTA_TYPE_FW_SCRIPT,
    OTA_TYPE_IA_SCRIPT,
    OTA_TYPE_MAX,
} OTAType_t;

typedef struct _updateInfo {
    void *session;
    unsigned int imgLen;
    unsigned int nowLen;
} OTAInfo_t;

// hal
int halHttpOpen(OTAInfo_t *info, const char *url);
int halUpdateFirmware(OTAInfo_t *info);
int halUpdateScript(OTAInfo_t *info, char *buf, int size);
void halHttpClose(OTAInfo_t *info);

// sw
int leOTA(OTAType_t type, const char *url, const char *sig);

#endif /* end of include guard: _OTA_H_ */
