#include "ota.h"

int leOTA(updateType_t type, const char *url, const char *sig)
{
    int ret, status = -1;
    updateInfo_t info = {0};

    LELOG("update type = %d, url = %s\r\n", type, url);
    if(type < 0 || type >= UPDATE_TYPE_MAX) {
        LELOGE("Update type error, %d\r\n", type);
        goto skip_update;
    }
    ret = halHttpOpen(&info, url);
    if(ret < 0) {
        LELOGE("Http open error\r\n");
        goto skip_update;
    }
    switch (type) {
        case UPDATE_TYPE_FW:
            status = halUpdateFirmware(&info);
            break;
        case UPDATE_TYPE_FW_SCRIPT:
            status = halUpdateScript((void *)&info, &ginScriptCfg);
            break;
        case UPDATE_TYPE_LK_SCRIPT:
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

