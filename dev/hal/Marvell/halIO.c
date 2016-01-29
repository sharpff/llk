#include "halHeader.h"
#include <flash.h>
#include <wlan.h>
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