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
		APPLOGE("[FOTA CLI] fota init fail. ");
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

    #ifdef LELINK_OTA_VERIFICATION
	ret = lelinkVerify(FOTA_PARITION_TMP, info->imgLen);
    #else
    ret = 0;
    #endif

    if (ret) {
    	APPLOGE("lelinkVerify error!");
    	return -6;
    }

    if (info->isCo) {
        ret = CoOTASetFlag(1);
    } else {
        if (0 == ret) {
            fota_trigger_update();
            fota_ret_t err;
            err = fota_trigger_update();
            if (0 == err ) {
                hal_sys_reboot(HAL_SYS_REBOOT_MAGIC, WHOLE_SYSTEM_REBOOT_COMMAND);
                APPLOG("Reboot device!");
                ret = 0;
            } else {
                APPLOGE("Trigger FOTA error!");
                ret = -7;
            }
        } else {
            ret = -8;
        }
    }
    return ret;
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

#include "hal_uart.h"
#include "hal_gpio.h"
#include "io.h"
#define BOOTLOADER_MAX_MESSAGE_LENGTH 255
#define DELAYMS_FOR_IO 5
#define memcpy os_memcpy

static int eUART_Write(uartHandler_t* handler, uint8_t *pu8Data, int iLength);
static int eBL_WriteMessage(uartHandler_t* handler, uint8_t eType, uint8_t u8HeaderLength, uint8_t *pu8Header, uint8_t u8Length, uint8_t *pu8Data) {
    uint8_t u8CheckSum = 0;
    int n;

    uint8_t au8Msg[256];

    APPLOG("u8HeaderLength[%d] u8Length[%d]", u8HeaderLength, u8Length);
    /* total message length cannot be > 255 bytes */
    if(u8HeaderLength + u8Length >= 0xfe)
    {
        APPLOGE("Length too big");
        return -1;
    }

    /* Message length */
    au8Msg[0] = u8HeaderLength + u8Length + 2;

    /* Message type */
    au8Msg[1] = (uint8_t)eType;

    /* Message header */
    memcpy(&au8Msg[2], pu8Header, u8HeaderLength);

    /* Message payload */
    memcpy(&au8Msg[2 + u8HeaderLength], pu8Data, u8Length);

    APPPRINTF("Tx: ");
    for(n = 0; n < u8HeaderLength + u8Length + 2; n++)
    {
        APPPRINTF("%02x ", au8Msg[n]);
        u8CheckSum ^= au8Msg[n];
    }
    APPPRINTF("\r\n");

    /* Message checksum */
    au8Msg[u8HeaderLength + u8Length + 2] = u8CheckSum;

    /* Write whole message to UART */
    return eUART_Write(handler, au8Msg, u8HeaderLength + u8Length + 3);
}

static int eUART_Read(uartHandler_t* handler, int iTimeoutMicroseconds, int iBufferLen, uint8_t *pu8Buffer, int *piBytesRead) {
    int ret = 0, count = 0, singleMS = DELAYMS_FOR_IO, tmpLen = 0;
    do {
        // APPLOGW("eUART_Read test[%d/%d]", tmpLen, iBufferLen);
        ret = halUartRead(handler, pu8Buffer + tmpLen, iBufferLen - tmpLen);
        if (0 >= ret) {
            // APPLOGW("eUART_Read count[%d]", count);
            halDelayms(singleMS);
            count++;
        } else {
            tmpLen += ret;
        }
    } while (tmpLen < iBufferLen && count*singleMS*1000 < iTimeoutMicroseconds);
    *piBytesRead = tmpLen;
    if (tmpLen > 0) {
        int n = 0;        
        APPLOGW("eUART_Read len[%d]", tmpLen);
        APPPRINTF("eUART_Read: ");
        for (n = 0; n < tmpLen; n++) {
            APPPRINTF("%02x ", pu8Buffer[n]);
        }
        APPPRINTF("\r\n");
    }
    return tmpLen > 0 ? 0 : -1;
}

static int eUART_Write(uartHandler_t* handler, uint8_t *pu8Data, int iLength) {
    int iBytesWritten;
    int iTotalBytesWritten = 0;
    
    if(pu8Data == NULL) {
        return -1;
    }

    do {
        iBytesWritten = halUartWrite(handler, &pu8Data[iTotalBytesWritten], iLength - iTotalBytesWritten);
        if(0 >= iBytesWritten) {
            halDelayms(DELAYMS_FOR_IO);
        } else {
            iTotalBytesWritten += iBytesWritten;
        }
    } while(iTotalBytesWritten < iLength);

    APPLOG("eUART_Write iLength[%d] iTotalBytesWritten[%d]", iLength, iTotalBytesWritten);
    return 0;
}

static uint8_t eBL_ReadMessage(uartHandler_t* handler, int iTimeoutMicroseconds, uint8_t *peType, uint8_t *pu8Length, uint8_t *pu8Data)
{

    int n;
    int eStatus;
    uint8_t au8Msg[BOOTLOADER_MAX_MESSAGE_LENGTH];
    uint8_t u8CalculatedCheckSum = 0;
    uint8_t u8Length = 0;
    uint8_t eResponse = 0;
    int iAttempts = 0;
    int iBytesRead = 0;
    int iTotalBytesRead = 0;

    /* Get the length byte */
    if((eUART_Read(handler, iTimeoutMicroseconds, 1, &u8Length, &iBytesRead) != 0) || (iBytesRead != 1))
    {
        APPLOGE("Error getting length");
        return 0xf6/*E_BL_RESPONSE_NO_RESPONSE*/;
    }

    //APPLOG("Incoming message length %d", u8Length);

    /* Message must have at least 3 bytes, maximum is implicit */
    if (u8Length < 3)
    {
        return 0xf6/*E_BL_RESPONSE_NO_RESPONSE*/;
    }

    /* Add length to checksum */
    u8CalculatedCheckSum ^= u8Length;

    do
    {
        APPLOG("eBL_ReadMessage iTimeoutMicroseconds[%d] u8Length[%d] iTotalBytesRead[%d] iBytesRead[%d]", 
            iTimeoutMicroseconds, u8Length, iTotalBytesRead, iBytesRead);
        /* Get the rest of the message */
        eStatus = eUART_Read(handler, iTimeoutMicroseconds, u8Length - iTotalBytesRead, &au8Msg[iTotalBytesRead], &iBytesRead);
        if(eStatus != 0) {
            APPLOG("Error reading message from UART [%d]", eStatus);
            return 0xf6/*E_BL_RESPONSE_NO_RESPONSE*/;
        }

        iTotalBytesRead += iBytesRead;
        iAttempts++;

    } while ((iTotalBytesRead < u8Length) && (iBytesRead > 0 || iAttempts < 10));

    if(iTotalBytesRead != u8Length)
    {
        APPPRINTF("Got %d bytes but expected %d after %d attempts: ", iTotalBytesRead, u8Length, iAttempts);
        for(n = 0; n < iTotalBytesRead; n++)
        {
            APPPRINTF("%02x ", au8Msg[n]);
        }
        APPPRINTF("\r\n");

        return 0xf6/*E_BL_RESPONSE_NO_RESPONSE*/;
    }

    /* Add rest of message to checksum */
    APPPRINTF("Rx: %02x ", u8Length);
    for(n = 0; n < u8Length; n++)
    {
        APPPRINTF("%02x ", au8Msg[n]);
        u8CalculatedCheckSum ^= au8Msg[n];
    }
    APPPRINTF("\r\n");

    if(u8CalculatedCheckSum != 0x00)
    {
        APPLOG("Checksum bad, got %02x expected %02x", u8CalculatedCheckSum, 0);
        return 0xfc/*E_BL_RESPONSE_CRC_ERROR*/;
    }

    *peType = au8Msg[0];
    eResponse = au8Msg[1];
    if (pu8Length)
    {
        *pu8Length = u8Length - 3;

        if (pu8Data)
        {
            memcpy(pu8Data, &au8Msg[2], *pu8Length);
        }
    }

    APPLOG("Got response 0x%02x", eResponse);

    return eResponse;
}

static uint8_t eBL_Request(uartHandler_t* handler, int iTimeoutMicroseconds, uint8_t eTxType, uint8_t u8HeaderLen, uint8_t *pu8Header, uint8_t u8TxLength, uint8_t *pu8TxData,
                          uint8_t *peRxType, uint8_t *pu8RxLength, uint8_t *pu8RxData)
{
    /* Check data is not too long */
    if(u8TxLength > 0xfd)
    {
        APPLOGE("Data too long");
        // return E_BL_RESPONSE_ERROR;
        return 0xf0;
    }

    /* Send message */
    if(eBL_WriteMessage(handler, eTxType, u8HeaderLen, pu8Header, u8TxLength, pu8TxData) != 0)
    {
        // return E_BL_RESPONSE_ERROR;
        APPLOGE("eBL_WriteMessage");
        return 0xf0;
    }
    // halDelayms(100);
    // TODO: 读取不完整的补丁

    if (0x27 == eTxType || 0x07) {
        uint8_t buf[8] = {0};
        int readBytes = 0;
        eUART_Read(handler, 2*1000*1000, sizeof(buf), buf, &readBytes);
        *peRxType = buf[1];
        return buf[2];
    }

    /* Get the response to the request */
    return eBL_ReadMessage(handler, iTimeoutMicroseconds, peRxType, pu8RxLength, pu8RxData);
}

static int eBL_CheckResponse(const char *pcFunction, int eResponse, uint8_t eRxType, uint8_t eExpectedRxType)
{
    APPLOG("%s: Response %02x", pcFunction, eResponse);
    switch (eResponse)
    {
        case(0x00):
            break;
        case (0xff/*E_BL_RESPONSE_NOT_SUPPORTED*/):
            return -1; // E_PRG_UNSUPPORTED_OPERATION;
        case (0xfe/*E_BL_RESPONSE_WRITE_FAIL*/):
            return -2; // E_PRG_ERROR_WRITING;
        case (0xf9/*E_BL_RESPONSE_READ_FAIL*/):
            return -3; // E_PRG_ERROR_READING;
        default:
            return -4; // E_PRG_COMMS_FAILED;
    }

    if (eRxType != eExpectedRxType)
    {
        APPLOG("%s: Got type 0x%02x, expected 0x%02x", pcFunction, eRxType, eExpectedRxType);
        return -5; // E_PRG_COMMS_FAILED;
    }
    return 0;
}

static void *OpenUART(int baud) {
    uartHandler_t hdlUART = {0};
    hal_gpio_init(36);
    hal_pinmux_set_function(36, HAL_GPIO_36_UART2_RX_CM4);
    hal_gpio_deinit(36);
    hal_gpio_init(37);
    hal_pinmux_set_function(37, HAL_GPIO_37_UART2_TX_CM4);
    hal_gpio_deinit(37);
    hdlUART.id = 1;
    // hdlUART.baud = 1000000; not support
    hdlUART.baud = baud;
    hdlUART.dataBits = 8;
    hdlUART.stopBits = 1;
    hdlUART.parity = 0;
    return halUartOpen(&hdlUART);
}

int eBL_FlashSelectDevice(uartHandler_t* handler, uint8_t u8ManufacturerID, uint8_t u8DeviceID, uint8_t u8ChipSelect)
{
    int n;
    int eResponse = 0;
    uint8_t eRxType = 0;
    uint8_t au8Buffer[6];
    uint8_t u8FlashType = 8;
    int eStatus;

    au8Buffer[0] = u8FlashType;
    au8Buffer[1] = 0;
    au8Buffer[2] = 0;
    au8Buffer[3] = 0;
    au8Buffer[4] = 0;

    eResponse = eBL_Request(handler, 1*1000*1000, 0x2c/*E_BL_MSG_TYPE_FLASH_SELECT_TYPE_REQUEST*/, 5, au8Buffer, 0, NULL, &eRxType, NULL, NULL);
    return eBL_CheckResponse(__FUNCTION__, eResponse, eRxType, 0x2d/*E_BL_MSG_TYPE_FLASH_SELECT_TYPE_RESPONSE*/);
}

int eBL_FlashErase(uartHandler_t* handler)
{
    int eResponse = 0;
    uint8_t eRxType = 0;

    eResponse = eBL_Request(handler, 10*1000*1000, 0x07/*E_BL_MSG_TYPE_FLASH_ERASE_REQUEST*/, 0, NULL, 0, NULL, &eRxType, NULL, NULL);
    return eBL_CheckResponse(__FUNCTION__, eResponse, eRxType, 0x08/*E_BL_MSG_TYPE_FLASH_ERASE_RESPONSE*/);
}

int eBL_FlashWrite(uartHandler_t* handler, uint32_t u32Address, uint8_t u8Length, uint8_t *pu8Buffer)
{
    uint8_t au8CmdBuffer[4];
    int eResponse = 0;
    uint8_t eRxType = 0;

    if(u8Length > 0xfc || pu8Buffer == NULL)
    {
        return -1;
    }

    au8CmdBuffer[0] = (uint8_t)(u32Address >> 0)  & 0xff;
    au8CmdBuffer[1] = (uint8_t)(u32Address >> 8)  & 0xff;
    au8CmdBuffer[2] = (uint8_t)(u32Address >> 16) & 0xff;
    au8CmdBuffer[3] = (uint8_t)(u32Address >> 24) & 0xff;

    eResponse = eBL_Request(handler, 1*1000*1000, 0x09/*E_BL_MSG_TYPE_FLASH_PROGRAM_REQUEST*/, 4, au8CmdBuffer, u8Length, pu8Buffer, &eRxType, NULL, NULL);
    return eBL_CheckResponse(__FUNCTION__, eResponse, eRxType, 0x0a/*E_BL_MSG_TYPE_FLASH_PROGRAM_RESPONSE*/);
}

int eBL_SetBaudrate(uartHandler_t* handler, uint32_t u32Baudrate)
{
    int eResponse = 0;
    uint8_t eRxType = 0;
    uint8_t au8Buffer[6];
    uint32_t u32Divisor = 9;

    // APPLOG("Set BL Baud rate to %d", u32Baudrate);

    // Divide 1MHz clock by baudrate to get the divisor
    // u32Divisor = (uint32_t)roundf(1000000.0 / (float)u32Baudrate);

    APPLOG("Set divisor %d", u32Divisor);

    au8Buffer[0] = (uint8_t)u32Divisor;
    au8Buffer[1] = 0;
    au8Buffer[2] = 0;
    au8Buffer[3] = 0;
    au8Buffer[4] = 0;

    eResponse = eBL_Request(handler, 1000*1000, 0x27/*E_BL_MSG_TYPE_SET_BAUD_REQUEST*/, 1, au8Buffer, 0, NULL, &eRxType, NULL, NULL);
    return eBL_CheckResponse(__FUNCTION__, eResponse, eRxType, 0x28/*E_BL_MSG_TYPE_SET_BAUD_RESPONSE*/);
}

int CoOTASetFlag(int flag) {
    return 0;
}

int CoOTAGetFlag() {
    return 1;
}
// [LEAPP] CoOTAProcessing PULLDOWN over @halOTA.c:632
// [LEAPP] CoOTAProcessing to Open UART @halOTA.c:636
// SELECT 
// halUartWrite [8]: 07 2c 08 00 00 00 00 23 
// [LEAPP] eUART_Write iLength[8] iTotalBytesWritten[8] @halOTA.c:376
// [LEAPP[W]] eUART_Read len[4] @halOTA.c:349
// eUART_Read: 03 2d 00 2e 
// [LEAPP] Set divisor 9 @halOTA.c:584
// [LEAPP] u8HeaderLength[1] u8Length[0] @halOTA.c:298
// Tx: 03 27 09 
// BAUD 
// halUartWrite [4]: 03 27 09 2d 
// [LEAPP] eUART_Write iLength[4] iTotalBytesWritten[4] @halOTA.c:376
// [LEAPP[W]] eUART_Read len[4] @halOTA.c:349
// eUART_Read: 03 28 00 2b 
// [LEAPP] eBL_SetBaudrate: Response 00 @halOTA.c:505
// [LEAPP] u8HeaderLength[0] u8Length[0] @halOTA.c:298
// Tx: 02 07 
// ERASE
// halUartWrite [3]: 02 07 05 
// [LEAPP] eUART_Write iLength[3] iTotalBytesWritten[3] @halOTA.c:376
// [LEAPP[W]] eUART_Read len[4] @halOTA.c:349
// eUART_Read: 03 08 00 0b 
// [LEAPP] eBL_FlashErase: Response 00 @halOTA.c:505

void CoOTAProcessing(void) {
    void *uart = NULL;
    uint8_t buf[512] = {0};
    int readBytes = 0, ret = 0, baud = 115200, baudBoot = 38400;
    int n;
    uint8_t u8ChunkSize;
// 01 00    11 00    00    11 03
// 01 02 10 11 02 10 02 10 11 03
    const uint8_t reset[] = {0x01, 0x02, 0x10, 0x11, 0x02, 0x10, 0x02, 0x10, 0x11, 0x03};
    gpioHandler_t hdlGPIO = {0};
    uartHandler_t hdlUART = {0};
    APPLOG("CoOTAProcessing START");
    if (!CoOTAGetFlag()) {
        APPLOG("CoOTAGetFlag NO");
        return;
    }

    // pull down MISO
    hal_gpio_init(24);
    hal_pinmux_set_function(24, HAL_GPIO_24_GPIO24);
    hal_gpio_deinit(24);
    hdlGPIO.id = 24;
    hdlGPIO.dir = GPIO_DIR_OUTPUT;
    hdlGPIO.mode = GPIO_MODE_PULLDOWN;
    if (0 > halGPIOOpen(&hdlGPIO)) {
        APPLOGE("CoOTAProcessing halGPIOOpen failed");
        return;
    }
    halDelayms(700);
    // halDelayms(20*1000);
    APPLOG("CoOTAProcessing PULLDOWN over");
    // halDelayms(3*1000);

    // reset through UART
    APPLOG("CoOTAProcessing to Open UART");
    #if 0
    hal_gpio_init(36);
    hal_pinmux_set_function(36, HAL_GPIO_36_UART2_RX_CM4);
    hal_gpio_deinit(36);
    hal_gpio_init(37);
    hal_pinmux_set_function(37, HAL_GPIO_37_UART2_TX_CM4);
    hal_gpio_deinit(37);
    hdlUART.id = 1;
    // hdlUART.baud = 1000000; not support
    hdlUART.baud = baud;
    hdlUART.dataBits = 8;
    hdlUART.stopBits = 1;
    hdlUART.parity = 0;
    uart = halUartOpen(&hdlUART);
    if (NULL == uart) {
        halGPIOWrite(&hdlGPIO, 1);
        halGPIOClose(&hdlGPIO);
        APPLOGE("CoOTAProcessing halUartOpen failed");
        return;
    }
    // halDelayms(2000);
    eUART_Write(&hdlUART, reset, sizeof(reset));
    // halUartRead(&hdlUART, buf, sizeof(buf));
    // halDelayms(6000);
    // APPLOG("CoOTAProcessing to Flash Co");
    // halDelayms(30*1000);
    halGPIOWrite(&hdlGPIO, 1);
    halGPIOClose(&hdlGPIO);

    // 2. uart update & entry normal mode
    // int m = 50;
    do {
        eUART_Read(&hdlUART, 300*1000, sizeof(buf), buf, &readBytes);
        // readBytes = halUartRead(&hdlUART, buf, sizeof(buf));
        APPLOGW("CoOTAProcessing halUartRead [%d]", readBytes);
        // halDelayms(500);
    // } while(m--);
    } while(readBytes > 0);
    halDelayms(3000);
    halUartClose(&hdlUART);
    halDelayms(5000);
    #endif

    // boot init
    hal_gpio_init(36);
    hal_pinmux_set_function(36, HAL_GPIO_36_UART2_RX_CM4);
    hal_gpio_deinit(36);
    hal_gpio_init(37);
    hal_pinmux_set_function(37, HAL_GPIO_37_UART2_TX_CM4);
    hal_gpio_deinit(37);
    hdlUART.id = 1;
    // hdlUART.baud = 1000000; not support
    hdlUART.baud = baudBoot;
    hdlUART.dataBits = 8;
    hdlUART.stopBits = 1;
    hdlUART.parity = 0;
    uart = halUartOpen(&hdlUART);
    if (NULL == uart) {
        APPLOGE("CoOTAProcessing halUartOpen failed for Boot");
        return;
    }

    int a = 0;
    do {
        // ret = eBL_FlashSelectDevice(&hdlUART, 0xCC, 0xEE, 0);
        {
            int bytesRead = 0;
            uint8_t tmp[] = {0x07, 0x2c, 0x08, 0x00, 0x00, 0x00, 0x00, 0x23};
            eUART_Write(&hdlUART, tmp, sizeof(tmp));
            eUART_Read(&hdlUART, 1*1000*1000, sizeof(tmp), tmp, &bytesRead);
            // halUartWrite(&hdlUART, tmp, sizeof(tmp));
            // halDelayms(1000);
            // halUartRead(&hdlUART, tmp, sizeof(tmp));
        }
        halDelayms(1000);
    } while (a--);

    if (0 != ret) {
        halUartClose(&hdlUART);
        APPLOG("CoOTAProcessing eBL_FlashSelectDevice failed [%d]", ret);
        return;
    }
    
    ret = eBL_SetBaudrate(&hdlUART, baud);
    if (0 != ret) {
        halUartClose(&hdlUART);
        APPLOG("CoOTAProcessing eBL_SetBaudrate failed [%d]", ret);
        return;
    }
    halUartClose(&hdlUART);

    hal_gpio_init(36);
    hal_pinmux_set_function(36, HAL_GPIO_36_UART2_RX_CM4);
    hal_gpio_deinit(36);
    hal_gpio_init(37);
    hal_pinmux_set_function(37, HAL_GPIO_37_UART2_TX_CM4);
    hal_gpio_deinit(37);
    hdlUART.id = 1;
    // hdlUART.baud = 1000000; not support
    hdlUART.baud = baud;
    hdlUART.dataBits = 8;
    hdlUART.stopBits = 1;
    hdlUART.parity = 0;
    uart = halUartOpen(&hdlUART);
    if (NULL == uart) {
        APPLOGE("CoOTAProcessing halUartOpen failed for Boot");
        return;
    }
    ret = eBL_FlashErase(&hdlUART);
    if (0 != ret) {
        halUartClose(&hdlUART);
        APPLOG("CoOTAProcessing eBL_FlashErase failed [%d]", ret);
        return;
    }

    // TODO: 193216 is Co FW size
    uint32_t u32ImageLength = 193216;
    uint32_t u32FlashOffset = 0;
#define chunkSize 128
    uint8_t chunk[chunkSize] = {0};
    uint32_t startAddr = CM4_FLASH_TMP_ADDR;
    int eStatus = 0;
    void *hdlFlash = NULL;
    hdlFlash = halFlashOpen();
    if (NULL == hdlFlash) {
        return;
    }
    for(n = 0; n < u32ImageLength; n += u8ChunkSize) {
        if((u32ImageLength - n) > chunkSize) {
            u8ChunkSize = chunkSize;
        }
        else {
            u8ChunkSize = u32ImageLength - n;
        }
        halFlashRead(hdlFlash, chunk, u8ChunkSize, startAddr + n, u8ChunkSize);
        APPLOG("CoOTAProcessing eBL_FlashWrite startAddr[%x + %x] u8ChunkSize[%d] ", startAddr, n, u8ChunkSize);
        if((eStatus = eBL_FlashWrite(&hdlUART, u32FlashOffset + n, u8ChunkSize, chunk)) != 0) {
            APPLOGE("CoOTAProcessing eBL_FlashWrite failed [%d]", eStatus);
            halFlashClose(hdlFlash);
            return;
        }
    }
    halFlashClose(hdlFlash);

    // 3. reset Co flag
    halUartClose(&hdlUART);
    CoOTASetFlag(0);
    APPLOG("CoOTAProcessing END");

}