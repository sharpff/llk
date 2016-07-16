// #include "leconfig.h"
#include "halHeader.h"
#include "ota.h"
#include "httpclient.h"

#include "flash_map.h"

#include "fota_76x7.h"
#include "fota_config.h"
#include "fota.h"
#include "hal_sys.h"
#include "os.h"


#define FOTA_BUF_SIZE    (1024 * 4 + 1)
#define FOTA_URL_BUF_LEN    (256)

log_create_module(fota_dl_api, PRINT_LEVEL_INFO);

static fota_partition_t s_flash_table[] =
{
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


fota_flash_t fota_flash_default_config =
{
    .table          = &s_flash_table[0],
    .table_entries  = FLASH_TABLE_ENTRIES,
    .block_size     = 4096
};

httpclient_t g_fota_httpclient = {0};
char get_url[FOTA_URL_BUF_LEN];

//static size_t httpFetchData(void *priv, void *buf, size_t max_len);

static int32_t _fota_http_retrieve_head(char* get_url, char* buf, uint32_t len, int* p_response_content_len)
{
    int32_t ret = HTTPCLIENT_ERROR_CONN;
    httpclient_data_t client_data = {0};
   
    client_data.response_buf = buf;
    client_data.response_buf_len = len;

    ret = httpclient_send_request(&g_fota_httpclient, get_url, HTTPCLIENT_HEAD, &client_data);
    if (ret < 0) 
	{      
        LOG_E(fota_dl_api, "[Lelink] http client fail to send request.\n");
        return ret;
    }

   
    ret = httpclient_recv_response(&g_fota_httpclient, &client_data);
    if (ret < 0) {
        return ret;
    }

    (*p_response_content_len) = client_data.response_content_len;
   
    LOG_I(fota_dl_api, "[Lelink]total length: %d\n", client_data.response_content_len);
    if (httpclient_get_response_code(&g_fota_httpclient) != 200) 
	{
        LOG_E(fota_dl_api, "[Lelink]data received not completed, or invalid error code\r\n");
        
        return -1;
    }

    return ret;
}

int halHttpOpen(OTAInfo_t *info, const char *url)
{
    //char get_url[FOTA_URL_BUF_LEN];
    int32_t ret = HTTPCLIENT_ERROR_CONN;
    uint32_t len_param = os_strlen(url);
	info->session = NULL;
	int resp_content_length = 0;

	//httpclient_t session = {0};

    if (len_param < 1) {
      return -1;
    }
    os_memset(get_url, 0, FOTA_URL_BUF_LEN);
    LOG_I(fota_dl_api, "length: %d\n", os_strlen(url));
    os_strncpy(get_url, url, os_strlen(url));
	
    char* buf = pvPortMalloc(FOTA_BUF_SIZE);
    if (buf == NULL) {
        LOG_E(fota_dl_api, "buf malloc failed.\r\n");
        return -3;
    }
  
    ret = httpclient_connect(&g_fota_httpclient, get_url, HTTP_PORT);

	if (!ret) 
	{
        ret = _fota_http_retrieve_head(get_url, buf, FOTA_BUF_SIZE, &resp_content_length);
    }
	else 
    {
        LOG_E(fota_dl_api, "[FOTA CLI] http client connect error. \r");
    }
    LOG_I(fota_dl_api, "Download result = %d \r\n", (int)ret);

	info->session = (void *)&g_fota_httpclient;
    info->imgLen = resp_content_length;

	vPortFree(buf);
    buf = NULL;
	
    return 0;
}

void halHttpClose(OTAInfo_t *info)
{
	if(info && info->session) 
	{
        httpclient_close((httpclient_t *)info->session, HTTP_PORT);
        info->session = NULL;
		os_memset(&g_fota_httpclient, 0x00, sizeof(httpclient_t));
		os_memset(get_url, 0x00, FOTA_URL_BUF_LEN);
    }
}

static int32_t _fota_http_retrieve_get_for_lelink(char* get_url, char* buf, uint32_t len)
{
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
		
		LOG_E(fota_dl_api, "[FOTA CLI] http client fail to send request.\n");
		return ret;
	}

	do {
		ret = httpclient_recv_response(&g_fota_httpclient, &client_data);
		if (ret < 0) {
			return ret;
		}

		if (recv_temp == 0)
		{
			recv_temp = client_data.response_content_len;
		}

		LOG_I(fota_dl_api, "[FOTA CLI]retrieve_len = %d \n", client_data.retrieve_len);
		
		data_len = recv_temp - client_data.retrieve_len;
		LOG_I(fota_dl_api, "[FOTA CLI]data_len = %u \n", data_len);
		
		count += data_len;
		recv_temp = client_data.retrieve_len;
		vTaskDelay(100);/* Print log may block other task, so sleep some ticks */
		LOG_I(fota_dl_api, "[FOTA CLI]total data received %u\n", count);
		//client_data.response_buf[5] = '\0';
		//if (strcmp(string_table[seg++], client_data.response_buf) != 0) {
		//	  return -1;
		//}
		write_ret = fota_write(FOTA_PARITION_TMP, (const uint8_t*)client_data.response_buf, data_len);
		if (FOTA_STATUS_OK != write_ret) {
			LOG_E(fota_dl_api, "fail to write flash, write_ret = %d\n", write_ret);
			return ret;
		}

		LOG_I(fota_dl_api, "[FOTA CLI] download progrses = %u\n", count * 100 / client_data.response_content_len);
		
	} while (ret == HTTPCLIENT_RETRIEVE_MORE_DATA);

	LOG_I(fota_dl_api, "[FOTA CLI]total length: %d\n", client_data.response_content_len);
	if (count != client_data.response_content_len || httpclient_get_response_code(&g_fota_httpclient) != 200) 
	{
		LOG_E(fota_dl_api, "[FOTA CLI]data received not completed, or invalid error code\r\n");
		
		return -1;
	}

	return ret;
}


int halUpdateFirmware(OTAInfo_t *info)
{
	int32_t ret = HTTPCLIENT_ERROR_CONN;

	if (fota_init(&fota_flash_default_config) != FOTA_STATUS_OK) {
		LOG_E(fota_dl_api, "[FOTA CLI] fota init fail. \n");
		return -2;
	}
	char* buf = pvPortMalloc(FOTA_BUF_SIZE);
	if (buf == NULL) {
		LOG_E(fota_dl_api, "buf malloc failed.\r\n");
		return -3;
	}

	//ret = httpclient_connect(&_s_fota_httpclient, get_url, HTTP_PORT);
	if(info->session)
		ret = HTTPCLIENT_OK;
		
	if (!ret) {
		ret = _fota_http_retrieve_get_for_lelink(get_url, buf, FOTA_BUF_SIZE);
	}else {
		LOG_E(fota_dl_api, "[FOTA CLI] http client connect error. \r");
	}

	LOG_I(fota_dl_api, "Download result = %d \r\n", (int)ret);

	//httpclient_close(()info->session, HTTP_PORT);

	vPortFree(buf);
	buf = NULL;
	if ( 0 == ret) {
		fota_trigger_update();
		fota_ret_t err;
		err = fota_trigger_update();
		if (0 == err ){
			hal_sys_reboot(HAL_SYS_REBOOT_MAGIC, WHOLE_SYSTEM_REBOOT_COMMAND);
			LOG_I(fota_dl_api, "Reboot device!");
			return 0;
		} else {
			LOG_E(fota_dl_api, "Trigger FOTA error!");
			return -1;
		}
		return 0;
	} else {
		return -1;
	}
}

static int32_t http_retrieve_get_for_lelink(char* get_url, char* buf, uint32_t len)
{
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
		
		LOG_E(fota_dl_api, "[FOTA CLI] http client fail to send request.\n");
		return ret;
	}

	do {
		ret = httpclient_recv_response(&g_fota_httpclient, &client_data);
		if (ret < 0) {
			return ret;
		}

		if (recv_temp == 0)
		{
			recv_temp = client_data.response_content_len;
		}

		LOG_I(fota_dl_api, "[FOTA CLI]retrieve_len = %d \n", client_data.retrieve_len);
		
		data_len = recv_temp - client_data.retrieve_len;
		LOG_I(fota_dl_api, "[FOTA CLI]data_len = %u \n", data_len);
		
		count += data_len;
		recv_temp = client_data.retrieve_len;
		vTaskDelay(100);/* Print log may block other task, so sleep some ticks */
		LOG_I(fota_dl_api, "[FOTA CLI]total data received %u\n", count);
		//client_data.response_buf[5] = '\0';
		//if (strcmp(string_table[seg++], client_data.response_buf) != 0) {
		//	  return -1;
		//}
		write_ret = fota_write(FOTA_PARITION_TMP, (const uint8_t*)client_data.response_buf, data_len);
		if (FOTA_STATUS_OK != write_ret) {
			LOG_E(fota_dl_api, "fail to write flash, write_ret = %d\n", write_ret);
			return ret;
		}

		LOG_I(fota_dl_api, "[FOTA CLI] download progrses = %u\n", count * 100 / client_data.response_content_len);
		
	} while (ret == HTTPCLIENT_RETRIEVE_MORE_DATA);

	LOG_I(fota_dl_api, "[FOTA CLI]total length: %d\n", client_data.response_content_len);
	if (count != client_data.response_content_len || httpclient_get_response_code(&g_fota_httpclient) != 200) 
	{
		LOG_E(fota_dl_api, "[FOTA CLI]data received not completed, or invalid error code\r\n");
		
		return -1;
	}

	return ret;
}

uint32_t halUpdate(OTAInfo_t *info, uint8_t *buf, uint32_t bufLen)
{
    int/* aSize = 256,*/ nSize = 0;
	uint32_t recv_temp = 0;
	uint32_t data_len = 0;
	
    httpclient_data_t client_data = {0};
	
	HTTPCLIENT_RESULT ret = HTTPCLIENT_ERROR_CONN;
	client_data.response_buf = (char *)buf;
	client_data.response_buf_len = bufLen;

	ret = httpclient_send_request(&g_fota_httpclient, get_url, HTTPCLIENT_GET, &client_data);

	if (ret < 0) {
		
		LOG_E(fota_dl_api, "[FOTA CLI] http client fail to send request.\n");
		return ret;
	}

    if(info == NULL || buf == NULL) 
	{
        //APPLOGE("Update script paremeter error!");
        printf("Update script paremeter error!");
        return -1;
    }
    if(info->imgLen > MAX_PROFILE_SIZE) 
	{
        //APPLOGE("Script too large!");
        printf("Update script paremeter error!");
        return -2;
    }
    //APPLOG("Clear script...");
    printf("Clear script...");
    
    //while((ret = httpFetchData(info, &buf[nSize], aSize)) > 0) 
    //while((ret = httpclient_recv_response(&g_fota_httpclient, &client_data)) > 0) 
    do
	{
		ret = httpclient_recv_response(&g_fota_httpclient, &client_data);
	    if (ret < 0) {
			return ret;
		}

		if (recv_temp == 0)
		{
			recv_temp = client_data.response_content_len;
		}

		LOG_I(fota_dl_api, "[FOTA CLI]retrieve_len = %d \n", client_data.retrieve_len);
		
		data_len = recv_temp - client_data.retrieve_len;
		LOG_I(fota_dl_api, "[FOTA CLI]data_len = %u \n", data_len);
		
		recv_temp = client_data.retrieve_len;
		
        if (nSize + data_len > bufLen) 
		{
            break;
        }
        nSize += data_len;
    } while (ret == HTTPCLIENT_RETRIEVE_MORE_DATA);
	
    if(nSize != info->imgLen) 
	{
        //APPLOGE("Get script data wrong, need %d bytes, but now %d bytes", info->imgLen, nSize);
        printf("Get script data wrong, need %d bytes, but now %d bytes", info->imgLen, nSize);
        return -3;
    }
    //APPLOG("Update script successed [%d]", nSize);
    printf("Update script successed [%d]", nSize);
	
    return nSize;
}
