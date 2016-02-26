#include "ota.h"

int leOTA(OTAType_t type, const char *url, const char *sig)
{
    int ret, status = -1;
    OTAInfo_t info = {0};

    LELOG("update type = %d, url = %s\r\n", type, url);
    if(type < 0 || type >= OTA_TYPE_MAX) {
        LELOGE("Update type error, %d\r\n", type);
        goto skip_update;
    }
    ret = halHttpOpen(&info, url);
    if(ret < 0) {
        LELOGE("Http open error\r\n");
        goto skip_update;
    }
    switch (type) {
        case OTA_TYPE_FW:
            status = halUpdateFirmware(&info);
            break;
        case OTA_TYPE_FW_SCRIPT:
            status = halUpdateScript((void *)&info, &ginScriptCfg);
            if(!status && ginScriptCfg.data.size > 0) {
                ginScriptCfg.csum = crc8((const uint8_t *)&(ginScriptCfg.data), sizeof(ginScriptCfg.data));
                //lelinkStorageWriteScriptCfg();
            } else {
                status = -1;
            }
            break;
        case OTA_TYPE_IA_SCRIPT:
            status = -1;
            break;
        default:
            status = -1;
            LELOGE("Update type(%d) error\r\n", type);
            break;
    }
    if(status) {
        LELOGE("Update error! status = %d\r\n", status);
    } else {
        LELOG("Update image successed!\r\n");
    }
skip_update:
    halHttpClose(&info);
    return status;
}

