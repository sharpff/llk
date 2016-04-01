#include <rfget.h>
#include <httpc.h>
#include <lelink/sw/leconfig.h>
#include <lelink/sw/ota.h>

static size_t httpFetchData(void *priv, void *buf, size_t max_len);

int halHttpOpen(OTAInfo_t *info, const char *url)
{
    int status = -1;
    http_resp_t *resp;
    static http_session_t session;

    status = httpc_get(url, &session, &resp, NULL);
    if (status != 0) {
        APPLOGE("Unable to connect to server");
        goto err_out;
    }
    if (resp->status_code != 200) {
        APPLOGE("HTTP Error %d", resp->status_code);
        goto err_out;
    }
    if (resp->chunked) {
        APPLOGE("HTTP chunked fs update is not supported");
        goto err_out;
    }
    if (resp->content_length <= 0) {
        APPLOGE("HTTP size(%d) error", resp->content_length);
        goto err_out;
    }
    info->session = &session;
    info->imgLen = resp->content_length;
    return 0;
err_out:
    return -1;
}

void halHttpClose(OTAInfo_t *info)
{
    if(info && info->session) {
        http_close_session(info->session);
        info->session = NULL;
    }
}

int halUpdateFirmware(OTAInfo_t *info)
{
    struct partition_entry *p = (struct partition_entry *)rfget_get_passive_firmware();
    /* Perform FW update later */
    if (p == NULL) {
        APPLOGE("Failed to get passive partition");
        return -1;
    } 
    return update_firmware(httpFetchData, (void *)info, info->imgLen, p);
}

uint32_t halUpdate(OTAInfo_t *info, uint8_t *buf, uint32_t bufLen)
{
    int ret = 0, aSize = 256, nSize = 0;

    if(info == NULL || buf == NULL) {
        APPLOGE("Update script paremeter error!");
        return -1;
    }
    if(info->imgLen > MAX_PROFILE_SIZE) {
        APPLOGE("Script too large!");
        return -2;
    }
    APPLOG("Clear script...");
    
    while((ret = httpFetchData(info, &buf[nSize], aSize)) > 0) {
        if (nSize + ret > bufLen) {
            break;
        }
        nSize += ret;
    }
    if(nSize != info->imgLen) {
        APPLOGE("Get script data wrong, need %d bytes, but now %d bytes", info->imgLen, nSize);
        return -3;
    }
    APPLOG("Update script successed [%d]", nSize);
    return nSize;
}

static size_t httpFetchData(void *priv, void *buf, size_t max_len)
{
    int ret = 0;
    OTAInfo_t *info = (OTAInfo_t *) priv;

    if((ret = http_read_content(*(http_session_t *)info->session, buf, max_len)) > 0){
        info->nowLen += ret;
    }
    APPLOG("%d/%d", info->nowLen, info->imgLen);
    return ret;
}

