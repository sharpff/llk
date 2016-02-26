#ifndef _OTA_H_
#define _OTA_H_

#include "leconfig.h"
#include "sengine.h"

typedef enum {
    UPDATE_TYPE_FW,
    UPDATE_TYPE_FW_SCRIPT,
    UPDATE_TYPE_LK_SCRIPT,
    UPDATE_TYPE_MAX,
} updateType_t;

typedef struct _updateInfo {
    void *session;
    unsigned int imgLen;
    unsigned int nowLen;
} updateInfo_t;

// hal
int halHttpOpen(updateInfo_t *info, const char *url);
int halUpdateFirmware(updateInfo_t *info);
int halUpdateScript(updateInfo_t *info, ScriptCfg *scriptCfg);
void halHttpClose(updateInfo_t *info);

// sw
int leOTA(updateType_t type, const char *url, const char *sig);

#endif /* end of include guard: _OTA_H_ */
