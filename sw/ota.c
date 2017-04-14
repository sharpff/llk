#include "ota.h"
#include "io.h"
#include "sengine.h"
#include "protocol.h"

extern int getFlashMinSize();
extern int getRegion(E_FLASH_TYPE type, FlashRegion *region);
static int ginOTAType = OTA_TYPE_NONE;
static char ginOTAUrl[MAX_BUF];
static uint8_t ginOTASig[RSA_LEN];


const char *otaGetLatestUrl() {
    return ginOTAUrl[0] ? ginOTAUrl : NULL;
}
void otaSetLatestUrl(const char *url, int lenUrl) {
    int len = 0; 
    if (NULL == url || 0 >= lenUrl) {
        return;
    }
    len = MIN(lenUrl, MAX_BUF - 1);
    memcpy(ginOTAUrl, url, len);
    ginOTAUrl[len] = 0;
    return;
}

const uint8_t *otaGetLatestSig() {
    return ginOTASig[0] ? ginOTASig : NULL;
}
void otaSetLatestSig(const uint8_t *sig) {
    // int len = 0; 
    if (NULL == sig) {
        return;
    }
    memcpy(ginOTASig, sig, sizeof(ginOTASig));
    return;
}

int otaGetLatestType() {
    return ginOTAType;
}
void otaSetLatestType(int type) {
    ginOTAType = type;
}

void otaInfoClean() {
    ginOTAType = OTA_TYPE_NONE;
    memset(ginOTASig, 0, sizeof(ginOTASig));
    memset(ginOTAUrl, 0, sizeof(ginOTAUrl));
}

int leOTA(OTAType_t type, const char *url, const uint8_t *sig, int sigLen)
{
    int ret, status = -1;
    OTAInfo_t info = {0};
    uint8_t *tmpScriptCfg = NULL;
    int tmpTotalSize = 0;

    LELOG("update type = %d, url = %s", type, url);
    if(type < 0) {
        LELOGW("Update type error, %d", type);
        goto skip_update;
    }
    info.type = type;
    ret = halHttpOpen(&info, url);
    if(ret < 0) {
        LELOGW("Http open error");
        goto skip_update;
    }
    switch (type) {
        case OTA_TYPE_FW:
            status = halUpdateFirmware(&info);
            break;
        case OTA_TYPE_SDEVFW: {
                info.isSDev = 1;
                status = halUpdateFirmware(&info);
            }break;
        case OTA_TYPE_FW_SCRIPT:
        case OTA_TYPE_IA_SCRIPT: {
            if (OTA_TYPE_FW_SCRIPT == type) {
                tmpScriptCfg = (uint8_t *)ginScriptCfg;
                tmpTotalSize = sizeof(ScriptCfg);
            } else if (OTA_TYPE_IA_SCRIPT == type) {
                tmpScriptCfg = (uint8_t *)ginScriptCfg2;
                tmpTotalSize = sizeof(ScriptCfg2);
            }
            memset(tmpScriptCfg, 0, tmpTotalSize);
            // status = halUpdate((void *)&info, (uint8_t *)(tmpScriptCfg->data.script), sizeof(tmpScriptCfg->data.script));
            status = halUpdate((void *)&info, tmpScriptCfg + sizeof(int), tmpTotalSize);
            if(0 < status) {
                *((int *)tmpScriptCfg) = status;
                #ifdef LELINK_OTA_VERIFICATION
                ret = lelinkVerifyBuf(tmpScriptCfg + sizeof(int), status);
                #else
                ret = 0;
                #endif
                if (0 > ret) { // recover the fw script, for ia script, it is no need to recover. cause ginScriptCfg2 is just a tmp var.
                        if (OTA_TYPE_FW_SCRIPT == type) {
                            ret = lelinkStorageReadScriptCfg(tmpScriptCfg, E_FLASH_TYPE_SCRIPT, 0);
                            LELOGW("recover E_FLASH_TYPE_SCRIPT ret[%d]", ret);
                        }
                        status = -8;
                        break;
                    }
                    if (OTA_TYPE_FW_SCRIPT == type) {
                        ret = lelinkStorageWriteScriptCfg(tmpScriptCfg, E_FLASH_TYPE_SCRIPT, 0);
                        if (0 > ret) {
                            status = -9;
                            break;
                        }
                    } else if (OTA_TYPE_IA_SCRIPT == type) {
                        ret = lelinkStorageWriteScriptCfg2(tmpScriptCfg);
                        if (0 > ret) {
                            status = -7;
                            break;
                        }
                    }
                    LELOG("OTA script type [%d] ret[%d] status[%d]", type, ret, status);
                } else {
                    status = -1;
                }
            }
            break;
        // test only -s
        case OTA_TYPE_SDEVINFO:
        case OTA_TYPE_PRIVATE:
        case OTA_TYPE_AUTH: {
                int flashSize = 0;
                // PrivateCfg privateCfg;
                // AuthCfg authCfg;
                void *tmpPtr = NULL;
                E_FLASH_TYPE flashType = E_FLASH_TYPE_MAX;
                if (OTA_TYPE_PRIVATE == type) {
                    flashSize = GET_PAGE_SIZE(sizeof(PrivateCfg), getFlashMinSize());
                    flashType = E_FLASH_TYPE_PRIVATE;
                } else if (OTA_TYPE_AUTH == type) {
                    flashSize = GET_PAGE_SIZE(sizeof(AuthCfg), getFlashMinSize());
                    flashType = E_FLASH_TYPE_AUTH;
                } else if (OTA_TYPE_SDEVINFO == type) {
                    flashSize = GET_PAGE_SIZE(sizeof(SDevInfoCfg), getFlashMinSize());
                    flashType = E_FLASH_TYPE_SDEV_INFO;  
                }
                tmpPtr = (void *)halCalloc(1, flashSize);
                status = halUpdate((void *)&info, (uint8_t *)tmpPtr, flashSize);
                if(0 < status) {
                    void *hdl = NULL;
                    FlashRegion fr;
                    ret = getRegion(flashType, &fr);
                    if (0 > ret) {
                        halFree(tmpPtr);
                        status = -2;
                        break;
                    }
                    hdl = (void *)halFlashOpen();
                    if (NULL == hdl) {
                        halFree(tmpPtr);
                        status = -3;
                        break;
                    }
                    LELOG("halFlashErase type[%d] flashType[%d] [0x%x][%d]", type, flashType, fr.addr, fr.size);
                    ret = halFlashErase(hdl, fr.addr, fr.size);
                    if (0 > ret) {
                        halFree(tmpPtr);
                        status = -4;
                        break;
                    }
                    ret = halFlashWrite(hdl, tmpPtr, flashSize, fr.addr, 0);
                    if (0 > ret) {
                        halFree(tmpPtr);
                        status = -5;
                        break;
                    }
                    halFlashClose(hdl);
                    halFree(tmpPtr);
                } else {
                    status = -1;
                }
            }
            break;
        // test only -e
        default:
            if (OTA_TYPE_OTHER >= type) {
                status = -1;
                LELOGE("Update type(%d) error", type);                
            }
            break;
    }

    if (OTA_TYPE_OTHER < type) {
        status = halUpdateFirmwareExt(&info);
    }

    if(0 > status) {
        LELOGE("Update error! status = %d", status);
    } else {
        LELOG("Update image successed [%d]!", status);
    }
skip_update:
    halHttpClose(&info);
    return status;
}

