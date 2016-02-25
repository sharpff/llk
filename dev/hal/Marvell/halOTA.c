#include <rfget.h>
#include <httpc.h>
#include <lelink/sw/leconfig.h>
#include <lelink/sw/utility.h>
#include <lelink/sw/ota.h>

static size_t httpFetchData(void *priv, void *buf, size_t max_len);

int halHttpOpen(updateInfo_t *info, const char *url)
{
    int status = -1;
    http_resp_t *resp;
    static http_session_t session;

    status = httpc_get(url, &session, &resp, NULL);
    if (status != 0) {
        APPLOGE("Unable to connect to server\r\n");
        goto err_out;
    }
    if (resp->status_code != 200) {
        APPLOGE("HTTP Error %d", resp->status_code);
        goto err_out;
    }
    if (resp->chunked) {
        APPLOGE("HTTP chunked fs update is not supported\r\n");
        goto err_out;
    }
    if (resp->content_length <= 0) {
        APPLOGE("HTTP size(%d) error\r\n", resp->content_length);
        goto err_out;
    }
    info->session = &session;
    info->imgLen = resp->content_length;
    return 0;
err_out:
    return -1;
}

void halHttpClose(updateInfo_t *info)
{
    if(info && info->session) {
        http_close_session(info->session);
        info->session = NULL;
    }
}

int halUpdateFirmware(updateInfo_t *info)
{
    struct partition_entry *p = (struct partition_entry *)rfget_get_passive_firmware();
    /* Perform FW update later */
    if (p == NULL) {
        APPLOGE("Failed to get passive partition\r\n");
        return -1;
    } 
    return update_firmware(httpFetchData, (void *)info, info->imgLen, p);
}

int halUpdateScript(updateInfo_t *info, ScriptCfg *scriptCfg)
{
    int ret = 0, aSize = 256, nSize = 0;

    if(info == NULL || scriptCfg == NULL) {
        APPLOGE("Update script paremeter error!\r\n");
        return -1;
    }
    if(info->imgLen > MAX_SCRIPT_SIZE) {
        APPLOGE("Script too large!\r\n");
        return -1;
    }
    APPLOG("Clear script...\r\n");
    scriptCfg->data.size = 0;
    while((ret = httpFetchData(info, &scriptCfg->data.script[nSize], aSize)) > 0) {
        nSize += ret;
    }
    if(nSize != info->imgLen) {
        APPLOGE("Get script data wrong, need %d bytes, but now %d bytes\r\n", info->imgLen, nSize);
        return -1;
    }
    scriptCfg->data.size = nSize;
    scriptCfg->csum = crc8((const uint8_t *)&(scriptCfg->data), sizeof(scriptCfg->data));
    /*lelinkStorageWriteScriptCfg();*/
    APPLOG("Update script successed\r\n");
    return 0;
}

static size_t httpFetchData(void *priv, void *buf, size_t max_len)
{
    int ret;
    updateInfo_t *info = (updateInfo_t *) priv;

    if((ret = http_read_content(*(http_session_t *)info->session, buf, max_len)) > 0){
        info->nowLen += ret;
    }
    APPLOG("Updating: Trying to read %d bytes from stream, ret = %d. %d/%d\r\n", 
            max_len, ret, info->nowLen, info->imgLen);
    return ret;
}

