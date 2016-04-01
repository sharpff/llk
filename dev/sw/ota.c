#include "ota.h"
#include "io.h"
#include "sengine.h"

extern int getFlashMinSize();
extern int getRegion(E_FLASH_TYPE type, FlashRegion *region);

int leOTA(OTAType_t type, const char *url, const uint8_t *sig, int sigLen)
{
    char tmpurl[256];
    int i, ret, status = -1;
    OTAInfo_t info = {0};
    ScriptCfg *tmpScriptCfg = NULL;

    LELOG("update type = %d, url = %s", type, url);
    if(type < 0 || type >= OTA_TYPE_MAX) {
        LELOGW("Update type error, %d", type);
        goto skip_update;
    }
    ret = halHttpOpen(&info, url);
    if(ret < 0) {
        LELOGW("Http open error");
        goto skip_update;
    }
    switch (type) {
        case OTA_TYPE_FW:
            status = halUpdateFirmware(&info);
            break;
        case OTA_TYPE_FW_SCRIPT:
        case OTA_TYPE_IA_SCRIPT: {
                if (OTA_TYPE_FW_SCRIPT == type) {
                    tmpScriptCfg = ginScriptCfg;
                } else if (OTA_TYPE_IA_SCRIPT == type) {
                    tmpScriptCfg = ginScriptCfg2;
                }
                if (tmpScriptCfg) {
                    memset(tmpScriptCfg, 0, sizeof(ScriptCfg));
                    status = halUpdate((void *)&info, (uint8_t *)(tmpScriptCfg->data.script), sizeof(tmpScriptCfg->data.script));
                    if(0 < status) {
                        uint8_t pubkey[MAX_RSA_PUBKEY] = {0};
                        int pubkeyLen = 0;
                        tmpScriptCfg->data.size = status;
                        pubkeyLen = getTerminalPublicKey(pubkey, sizeof(pubkey));
                        LELOG("signatrue [%d]", ret);
                        ret = rsaVerify(pubkey, pubkeyLen, tmpScriptCfg->data.script, tmpScriptCfg->data.size, sig, sigLen);
                        LELOG("rsaVerify [%d]", ret);
                        if (OTA_TYPE_FW_SCRIPT == type) {
                            ret = lelinkStorageWriteScriptCfg(tmpScriptCfg, E_FLASH_TYPE_SCRIPT, 0);
                        } else if (OTA_TYPE_IA_SCRIPT == type) {
                            ret = lelinkStorageWriteScriptCfg2(tmpScriptCfg);
                            if (0 > ret) {
                                break;
                            }
                        }
                        LELOG("OTA script type [%d] ret[%d] status[%d]", type, ret, status);
                    } else {
                        status = -1;
                    }
                }
            }
            break;
        // test only -s
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
                } else {
                    flashSize = GET_PAGE_SIZE(sizeof(AuthCfg), getFlashMinSize());
                    flashType = E_FLASH_TYPE_AUTH;
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
                    ret = halFlashWrite(hdl, tmpPtr, flashSize, fr.addr);
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
            status = -1;
            LELOGE("Update type(%d) error", type);
            break;
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

