#include "leconfig.h"
#include "ota.h"
#include "httpc.h"
static size_t httpFetchData(void *priv, void *buf, size_t max_len);

#define FOTA_BUF_SIZE    (1024)
#define FOTA_URL_BUF_LEN    (256)
// static httpclient_t g_fota_httpclient = {0};
static char get_url[FOTA_URL_BUF_LEN];

int halHttpOpen(OTAInfo_t *info, const char *url) {
    int status = -1;
    char tmpurl[MAX_BUF] = {0};
    http_resp_t *resp;
    http_session_t session;

    info->session = NULL;
    strcpy(tmpurl, url);
    url = tmpurl;
again:
    status = httpc_get(url, &session, &resp, NULL);
    if (status != 0) {
        APPLOGE("Unable to connect to server");
        goto err_out;
    }
    if (resp->status_code > 300 && resp->status_code < 400) {
        char *pv = NULL;
        status = http_get_response_hdr_value(session, "Location", &pv);
        if(status) {
            APPLOGE("Can't get moved Location, status = %d", status);
            goto err_out;
        }
        APPLOGW("Http moved:%s", pv);
        strcpy(tmpurl, pv);
        http_close_session(&session);
        goto again;
    } else if (resp->status_code != 200) {
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

    info->session = (void *)session;
    info->imgLen = resp->content_length;
    // TODO: only used for mt7687, temp resolution
    memset(get_url, 0, FOTA_URL_BUF_LEN);
    strncpy(get_url, url, strlen(url));
    // g_fota_httpclient.socket = http_get_sockfd_from_handle(session);
    // g_fota_httpclient.remote_port = 80;
    return 0;
err_out:
    return -1;
}

void halHttpClose(OTAInfo_t *info) {
	if(info && info->session) {
		http_close_session((http_session_t *)&info->session);
		info->session = NULL;
	    // TODO: only used for mt7687, temp resolution
		// memset(&g_fota_httpclient, 0x00, sizeof(httpclient_t));
		memset(get_url, 0x00, FOTA_URL_BUF_LEN);
	}
}


int halUpdateFirmware(OTAInfo_t *info) {
    return 0;
}


uint32_t halUpdate(OTAInfo_t *info, uint8_t *buf, uint32_t bufLen) {
    int ret = 0, aSize = 256, nSize = 0;

    if(info == NULL || buf == NULL) {
        APPLOGE("Update script paremeter error!");
        return -1;
    }
    if(info->imgLen > bufLen) {
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

    if((ret = http_read_content((http_session_t)info->session, buf, max_len)) > 0){
        info->nowLen += ret;
    }
    APPLOG("%d/%d", info->nowLen, info->imgLen);
    return ret;
}