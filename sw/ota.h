#ifndef _OTA_H_
#define _OTA_H_
#ifdef __cplusplus
extern "C"
{
    
#endif
#include "leconfig.h"

typedef enum {
    OTA_TYPE_NONE = 0,
    OTA_TYPE_RESERVED_PF,
    OTA_TYPE_FW, // 2
    OTA_TYPE_RESERVED_PROTOCOL,
    OTA_TYPE_FW_SCRIPT, // 4
    OTA_TYPE_IA_SCRIPT, // 5
    OTA_TYPE_AUTH, // 6
    OTA_TYPE_PRIVATE, // 7
    OTA_TYPE_SDEVINFO, // 8
    OTA_TYPE_SDEVFW, // 9
    OTA_TYPE_MAX
} OTAType_t;

typedef struct _updateInfo {
    void *session;
    unsigned int imgLen;
    unsigned int nowLen;
    uint8_t isSDev;
    uint8_t reserved[3];
} OTAInfo_t;

// hal
int halHttpOpen(OTAInfo_t *info, const char *url);
void halHttpClose(OTAInfo_t *info);
int halUpdateFirmware(OTAInfo_t *info);
uint32_t halUpdate(OTAInfo_t *info, uint8_t *buf, uint32_t bufLen);

// sw
int leOTA(OTAType_t type, const char *url, const uint8_t *sig, int sigLen);
void otaInfoClean();

#ifdef __cplusplus
}
#endif

#endif /* end of include guard: _OTA_H_ */
