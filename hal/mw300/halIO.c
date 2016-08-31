#include "halHeader.h"
#include <flash.h>
#include <wlan.h>
#include <rfget.h>
#include <httpc.h>
#include <mw300_uart.h>
#if defined(__MRVL_SDK3_3__)
#include "io.h"
#else
#include <lelink/sw/io.h>
#endif

void *halUartOpen(uartHandler_t* handler) {
    int ret = 0;
    mdev_t *hdl = NULL;
    switch (handler->stopBits) {
        case 1: {
            handler->stopBits = UART_STOPBITS_1;
        } break;
        case 2: {
            handler->stopBits = UART_STOPBITS_2;
        } break;
        default:
            return NULL;
        break;
    }

    switch (handler->dataBits) {
        case 8: {
            handler->dataBits = UART_8BIT;
        } break;
        case 9: {
            handler->dataBits = UART_9BIT;
        } break;
        default:
            return NULL;
        break;
    }

    ret = uart_drv_init(handler->id, handler->dataBits);
    APPLOG("uart_drv_init [%d] ", ret);
    ret = uart_drv_set_opts(handler->id, handler->parity, handler->stopBits, handler->flowCtrl);
    APPLOG("uart_drv_set_opts [%d] ", ret);
    hdl = uart_drv_open(handler->id, handler->baud);
    APPLOG("uart_drv_open [0x%p] ", hdl);
    handler->handler = (void*)hdl;
    return hdl;
}

int halUartClose(uartHandler_t* handler) {
    return uart_drv_close((mdev_t *)handler->handler);
}

#define TO_DO_FOR_HONYAR_BUG    0
int halUartRead(uartHandler_t* handler, uint8_t *buf, uint32_t len) {
#if TO_DO_FOR_HONYAR_BUG
    int i = 0;
#endif
    int ret = uart_drv_read((mdev_t *)handler->handler, buf, len);
    int tmpLen = 0;
    if (0 < ret) {
        do {
            tmpLen += ret;
            //APPLOG("snap [%d]", ret);
            ret = uart_drv_read((mdev_t *)handler->handler, buf + tmpLen, len - tmpLen);
        } while (0 < ret);
#if TO_DO_FOR_HONYAR_BUG
        APPLOG("halUartRead tmpLen [%d]", tmpLen);
        for (i = 0; i < tmpLen; i++) {
            APPPRINTF("%02x ", buf[i]);
        }
        APPPRINTF("\r\n");
#endif
    }
    return tmpLen;
}

int halUartWrite(uartHandler_t* handler, const uint8_t *buf, uint32_t len) {
    int ret = 0;
#if TO_DO_FOR_HONYAR_BUG
    int i = 0;
#endif
    ret = uart_drv_write((mdev_t *)handler->handler, buf, len);
#if TO_DO_FOR_HONYAR_BUG
    APPLOG("---------------------------------------------------------");
    APPLOG("uart_drv_write [%d] ", ret);
    if (ret > 0)
    for (i = 0; i < ret; i++) {
        APPPRINTF("%02x ", buf[i]);
    }
    APPPRINTF("\r\n");
    APPLOG("---------------------------------------------------------\r\n");
#endif
    return ret;
}

mdev_t *g_gpio_dev = NULL;

void halCommonInit(commonManager_t* dev) {
    int i = 0;
    gpio_drv_init();
    if(!(g_gpio_dev = gpio_drv_open("MDEV_GPIO"))) {
        return;
    }
    for(i=0; i<dev->num; i++) {
        GPIO_PinMuxFun(dev->table[i].id, dev->table[i].mux);
    }
}

void* halGPIOInit(void) {
    return (void *)g_gpio_dev;
}

int halGPIOOpen(gpioHandler_t* handler) {
    handler->handler = (void *)g_gpio_dev;
    switch(handler->dir)
    {
        case GPIO_DIR_INPUT:
            GPIO_SetPinDir(handler->id, GPIO_INPUT);
            break;
        case GPIO_DIR_OUTPUT:
            GPIO_SetPinDir(handler->id, GPIO_OUTPUT);
            break;
        default:
            return -1;
    }
    switch(handler->mode)
    {
        case GPIO_MODE_DEFAULT:
            GPIO_PinModeConfig(handler->id, PINMODE_DEFAULT);
            break;
        case GPIO_MODE_PULLUP:
            GPIO_PinModeConfig(handler->id, PINMODE_PULLUP);
            break;
        case GPIO_MODE_PULLDOWN:
            GPIO_PinModeConfig(handler->id, PINMODE_PULLDOWN);
            break;
        case GPIO_MODE_NOPULL:
            GPIO_PinModeConfig(handler->id, PINMODE_NOPULL);
            break;
        case GPIO_MODE_RISTATE:
            GPIO_PinModeConfig(handler->id, PINMODE_TRISTATE);
            break;
        default:
            return -1;
    }
    return handler->id;
}

int halGPIOClose(gpioHandler_t* handler) {
    if(!g_gpio_dev) {
        return -1;
    }
    gpio_drv_close(g_gpio_dev);
    g_gpio_dev = NULL;
    return 0;
}

int halGPIORead(gpioHandler_t* handler, int *val) {
    int ret = gpio_drv_read((mdev_t *)handler->handler, handler->id, val);
    if (0 > ret) {
        return -1;
    }
    *val = (*val ==  GPIO_IO_LOW) ? GPIO_STATE_LOW : GPIO_STATE_HIGH;
    return sizeof(*val);
}

int halGPIOWrite(gpioHandler_t* handler, const int val) {
    int v;
    v = (val ==  GPIO_STATE_LOW) ? GPIO_IO_LOW : GPIO_IO_HIGH;
    gpio_drv_write((mdev_t *)handler->handler, handler->id, v);
    return sizeof(val);
}

void halPWMWrite(pwmHandler_t *handler, uint32_t percent) {

}

void halPWMRead(pwmHandler_t *handler, uint32_t *percent) {

}

void halPWMSetFrequency(pwmHandler_t *handler) {

}

int halPWMClose(pwmHandler_t *handler) {
    return 0;
}

int halPWMOpen(pwmHandler_t *handler) {
    return 0;
}

void* halPWMInit(int clock) {
    return 0xffffffff;
}

void *halPipeOpen(char *name) {
    return (void *)0xffffffff;
}

int halPipeClose(void *dev) {
    return 0;
}

int halPipeRead(void *dev, uint8_t *buf, uint32_t len) {
    return 0;
}

int halPipeWrite(void *dev, const uint8_t *buf, uint32_t len) {
    return len;
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

int halFlashWrite(void *dev, const uint8_t *data, int len, uint32_t startAddr, int32_t offsetToBegin)
{
    return flash_drv_write((mdev_t *)dev, data, len, startAddr + offsetToBegin);
    return 0;
}

int halFlashRead(void *dev, uint8_t *data, int len, uint32_t startAddr, int32_t offsetToBegin)
{
    int ret = flash_drv_read((mdev_t *)dev, data, len, startAddr + offsetToBegin);
    if (0 == ret) {
        return len;
    }
    return -1;
}

int halGetMac(uint8_t *mac, int len) {
    if (MLAN_MAC_ADDR_LENGTH > len || NULL == mac) {
        return -1;
    }
    wlan_get_mac_address(mac);
    APPLOG("mac [%02x:%02x:%02x:%02x:%02x:%02x]", 
        mac[0], 
        mac[1], 
        mac[2], 
        mac[3], 
        mac[4], 
        mac[5]);
    // mac[5] = 67;
    // wlan_set_mac_addr(mac);
    return 0;
}

void halPrint(const char *log) {
    wmprintf(log);
}