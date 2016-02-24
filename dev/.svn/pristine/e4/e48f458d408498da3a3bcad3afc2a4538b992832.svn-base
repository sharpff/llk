#include "halHeader.h"
#include <flash.h>
#include <wlan.h>
#include <rfget.h>
#include <httpc.h>
#include <mw300_uart.h>

void *halUartOpen(int baud, int dataBits, int stopBits, int parity, int flowCtrl) {
    int ret = 0;
    mdev_t *hdl = NULL;
    switch (stopBits) {
        case 1: {
            stopBits = UART_STOPBITS_1;
        } break;
        case 2: {
            stopBits = UART_STOPBITS_2;
        } break;
        default:
            return NULL;
        break;
    }

    switch (dataBits) {
        case 8: {
            dataBits = UART_8BIT;
        } break;
        case 9: {
            dataBits = UART_9BIT;
        } break;
        default:
            return NULL;
        break;
    }

    ret = uart_drv_init(UART1_ID, dataBits);
    APPLOG("uart_drv_init [%d] \r\n", ret);
    ret = uart_drv_set_opts(UART1_ID, parity, stopBits, flowCtrl);
    APPLOG("uart_drv_set_opts [%d] \r\n", ret);
    hdl = uart_drv_open(UART1_ID, baud);
    APPLOG("uart_drv_open [0x%p] \r\n", hdl);

    return hdl;
}

int halUartClose(void *dev) {
    return uart_drv_close((mdev_t *)dev);
}

int halUartRead(void *dev, uint8_t *buf, uint32_t len) {
    int ret = 0;
    ret = uart_drv_read((mdev_t *)dev, buf, len);
    APPLOG("uart_drv_read [%d]\r\n", ret);
    return ret;
}

int halUartWrite(void *dev, const uint8_t *buf, uint32_t len) {
    int ret = 0;
    ret = uart_drv_write((mdev_t *)dev, buf, len);
    APPLOG("uart_drv_write [%d] \r\n", ret);
    return ret;
}


int halFlashInit(void)
{
    flash_drv_init();
    return 0;
}

int halFlashDeinit(void)
{
    return 0;
}

void *halFlashOpen(void)
{
    return (void *)flash_drv_open(FL_INT);
}

int halFlashClose(void *dev)
{
    return flash_drv_close((mdev_t *)dev);
}

int halFlashErase(void *dev, uint32_t startAddr, uint32_t size)
{
    /* 
     * Marvell's fatal bug for erase. do not erase it while do writting. 
     * presume that erasing will be token over.
     */
    return flash_drv_erase((mdev_t *)dev, startAddr, size);
    return 0;
}

int halFlashWrite(void *dev, const uint8_t *data, int len, uint32_t startAddr)
{
    return flash_drv_write((mdev_t *)dev, data, len, startAddr);
    return 0;
}

int halFlashRead(void *dev, uint8_t *data, int len, uint32_t startAddr)
{
    return flash_drv_read((mdev_t *)dev, data, len, startAddr);
}

int halGetMac(uint8_t *mac, int len) {
    if (MLAN_MAC_ADDR_LENGTH > len || NULL == mac) {
        return -1;
    }
    wlan_get_mac_address(mac);
    APPLOG("mac [%02x:%02x:%02x:%02x:%02x:%02x]\r\n", 
        mac[0], 
        mac[1], 
        mac[2], 
        mac[3], 
        mac[4], 
        mac[5]);
    return 0;
}

typedef enum {
    UPDATE_TYPE_FW,
    UPDATE_TYPE_FW_SCRIPT,
    UPDATE_TYPE_LK_SCRIPT,
    UPDATE_TYPE_MAX,
} updateType_t;

typedef struct _updateInfo {
    http_session_t session;
    unsigned int imgLen;
    unsigned int nowLen;
} updateInfo_t;

static size_t fetchDataFromUrlCb(void *priv, void *buf, size_t max_len)
{
    int ret;
    updateInfo_t *handle = (updateInfo_t *) priv;

    ret = http_read_content(handle->session, buf, max_len);
    handle->nowLen += ret;
    APPLOG("Updating: Trying to read %d bytes from stream, ret = %d. %d/%d\r\n", 
            max_len, ret, handle->nowLen, handle->imgLen);
    return ret;
}

int halUpdateImage(int type, const char *url, const char *sig)
{
    http_resp_t *resp;
    int status = -WM_FAIL;
    updateInfo_t info = {0};

    APPLOG("update type = %d, url = %s\r\n", type, url);
    APPLOG("waiting update, now version: %s-%s\r\n", __DATE__, __TIME__);
    if(type < 0 || type >= UPDATE_TYPE_MAX) {
        APPLOGE("Update type error, %d\r\n", type);
        goto skip_update;
    }
    status = httpc_get(url, &info.session, &resp, NULL);
    if (status != 0) {
        APPLOGE("Unable to connect to server\r\n");
        goto skip_update;
    }   
    if (resp->status_code != 200) {
        APPLOGE("HTTP Error %d", resp->status_code);
        status = -WM_FAIL;
        goto skip_update;
    }
    if (resp->chunked) {
        APPLOGE("HTTP chunked fs update is not supported\r\n");
        status = -WM_FAIL;
        goto skip_update;
    }
    switch (type) {
        case UPDATE_TYPE_FW:
            {
                struct partition_entry *p;

                p = (struct partition_entry *)rfget_get_passive_firmware();
                /*const char *update_url = "http://115.182.63.167/fei/le_demo.bin";*/
                /* Perform FW update later */
                if (p == NULL) {
                    APPLOGE("Failed to get passive partition\r\n");
                    status = -WM_FAIL;
                } else {
                    info.imgLen = resp->content_length;
                    status = update_firmware(fetchDataFromUrlCb, (void *)&info, info.imgLen, p);
                }
            }
            break;
        case UPDATE_TYPE_FW_SCRIPT:
            status = -WM_FAIL;
            break;
        case UPDATE_TYPE_LK_SCRIPT:
            status = -WM_FAIL;
            break;
        default:
            status = -WM_FAIL;
            APPLOGE("Update type(%d) error\r\n", type);
            break;
    }
    if(status == WM_SUCCESS) {
        APPLOG("Update image successed!\r\n");
    } else {
        APPLOGE("Update error! status = %d\r\n", status);
    }
skip_update:
    http_close_session(&info.session);
    return status;
}

