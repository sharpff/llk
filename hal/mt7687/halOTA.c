#include "leconfig.h"
#include "ota.h"
#include "httpclient.h"

#include "flash_map.h"

#include "fota_76x7.h"
#include "fota_config.h"
#include "fota.h"
#include "hal_sys.h"
#include "os.h"

#include "httpc.h"
static size_t httpFetchData(void *priv, void *buf, size_t max_len);

#define FOTA_BUF_SIZE    (1024)
#define FOTA_URL_BUF_LEN    (256)

log_create_module(fota_dl_api, PRINT_LEVEL_INFO);

static fota_partition_t s_flash_table[] = {
    {
        .id         = FOTA_PARITION_LOADER,
        .address    = 0x00000,
        .length     = FLASH_LOADER_SIZE
    },
    {
        .id         = FOTA_PARITION_NCP,
        .address    = CM4_FLASH_N9_RAMCODE_ADDR,
        .length     = FLASH_N9_RAM_CODE_SIZE
    },
    {
        .id         = FOTA_PARITION_CM4,
        .address    = CM4_FLASH_CM4_ADDR,
        .length     = FLASH_CM4_XIP_CODE_SIZE
    },
    {
        .id         = FOTA_PARITION_TMP,
        .address    = CM4_FLASH_TMP_ADDR,
        .length     = FLASH_TMP_SIZE
    }
};

#define FLASH_TABLE_ENTRIES (sizeof(s_flash_table) / sizeof(fota_partition_t))


fota_flash_t fota_flash_default_config = {
    .table          = &s_flash_table[0],
    .table_entries  = FLASH_TABLE_ENTRIES,
    .block_size     = 4096
};

httpclient_t g_fota_httpclient = {0};
char get_url[FOTA_URL_BUF_LEN];

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
    os_memset(get_url, 0, FOTA_URL_BUF_LEN);
    os_strncpy(get_url, url, os_strlen(url));
    g_fota_httpclient.socket = http_get_sockfd_from_handle(session);
    g_fota_httpclient.remote_port = 80;
    return 0;
err_out:
    return -1;
}

void halHttpClose(OTAInfo_t *info) {
    if(info && info->session) {
        http_close_session((http_session_t *)&info->session);
        info->session = NULL;
	    // TODO: only used for mt7687, temp resolution
		os_memset(&g_fota_httpclient, 0x00, sizeof(httpclient_t));
		os_memset(get_url, 0x00, FOTA_URL_BUF_LEN);
    }
}

static int32_t _fota_http_retrieve_get_for_lelink(char* get_url, char* buf, uint32_t len) {
	int32_t ret = HTTPCLIENT_ERROR_CONN;
	fota_status_t write_ret;
	httpclient_data_t client_data = {0};
	uint32_t count = 0;
	uint32_t recv_temp = 0;
	uint32_t data_len = 0;

	client_data.response_buf = buf;
	client_data.response_buf_len = len;

	ret = httpclient_send_request(&g_fota_httpclient, get_url, HTTPCLIENT_GET, &client_data);
	if (ret < 0) {
		APPLOGE("[FOTA CLI] http client fail to send request.");
		return ret;
	}

	do {
		ret = httpclient_recv_response(&g_fota_httpclient, &client_data);
		if (ret < 0) {
			return ret;
		}

		if (recv_temp == 0) {
			recv_temp = client_data.response_content_len;
		}

		data_len = recv_temp - client_data.retrieve_len;

		count += data_len;
		recv_temp = client_data.retrieve_len;
		// vTaskDelay(100);/* Print log may block other task, so sleep some ticks */
		write_ret = fota_write(FOTA_PARITION_TMP, (const uint8_t*)client_data.response_buf, data_len);
		if (FOTA_STATUS_OK != write_ret) {
			 APPLOGE("fail to write flash, write_ret = %d", write_ret);
			return ret;
		}
	} while (ret == HTTPCLIENT_RETRIEVE_MORE_DATA);

	//APPLOG("[FOTA CLI]total length: %d", client_data.response_content_len);
	if (count != client_data.response_content_len || httpclient_get_response_code(&g_fota_httpclient) != 200) {
		 APPLOGE("[FOTA CLI]data received not completed, or invalid error code");
		return -1;
	}

	return ret;
}


int halUpdateFirmware(OTAInfo_t *info) {
	int32_t ret = 0;
	char *buf = NULL;
    int totalSize = 0;

    if(NULL == info || NULL == info->session) {
        APPLOGE("halUpdateFirmware paremeter error!");
        return -1;
    }

	if (fota_init(&fota_flash_default_config) != FOTA_STATUS_OK) {
		APPLOGE("[FOTA CLI] fota init fail. \n");
		return -2;
	}

	buf = pvPortMalloc(FOTA_BUF_SIZE);
	if (NULL == buf) {
		APPLOGE("buf malloc failed.");
		return -3;
	}

	do {
		int nSize = 0;
		nSize = httpFetchData(info, buf, FOTA_BUF_SIZE);
		if (0 >= nSize) {
			APPLOG("httpFetchData OVER [%d]", ret);
			break;
		}
		// APPLOG("has read bytes [%d]", nSize);

		ret = fota_write(FOTA_PARITION_TMP, (const uint8_t*)buf, nSize);
		if (FOTA_STATUS_OK != ret) {
			APPLOGE("fail to write flash, ret = %d", ret);
			vPortFree(buf);
			return -4;
		}
		totalSize += nSize;
	} while (totalSize < info->imgLen);

	if(totalSize != info->imgLen) {
		APPLOGE("halUpdateFirmware data wrong, need %d bytes, but now %d bytes", info->imgLen, totalSize);
		vPortFree(buf);
		return -5;
	}
	APPLOG("Download result = %d", (int)ret);
	vPortFree(buf);
	buf = NULL;

	// test only
	ret = lelinkVerify(p->start, info->imgLen);
    // ret = 0;

    if (ret) {
    	APPLOGE("lelinkVerify error!");
    	return -6;
    }

	if ( 0 == ret) {
		fota_trigger_update();
		fota_ret_t err;
		err = fota_trigger_update();
		if (0 == err ) {
			hal_sys_reboot(HAL_SYS_REBOOT_MAGIC, WHOLE_SYSTEM_REBOOT_COMMAND);
			 APPLOGE("Reboot device!");
			return 0;
		} else {
			 APPLOGE("Trigger FOTA error!");
			return -7;
		}
		return 0;
	} else {
		return -8;
	}
}

uint32_t halUpdate(OTAInfo_t *info, uint8_t *buf, uint32_t bufLen) {
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

    if((ret = http_read_content((http_session_t)info->session, buf, max_len)) > 0){
        info->nowLen += ret;
    }
    APPLOG("%d/%d", info->nowLen, info->imgLen);
    return ret;
}