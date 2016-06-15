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
    APPLOG("uart_drv_init [%d] ", ret);
    ret = uart_drv_set_opts(UART1_ID, parity, stopBits, flowCtrl);
    APPLOG("uart_drv_set_opts [%d] ", ret);
    hdl = uart_drv_open(UART1_ID, baud);
    APPLOG("uart_drv_open [0x%p] ", hdl);

    return hdl;
}

int halUartClose(void *dev) {
    return uart_drv_close((mdev_t *)dev);
}

#define TO_DO_FOR_HONYAR_BUG    0
int halUartRead(void *dev, uint8_t *buf, uint32_t len) {
#if TO_DO_FOR_HONYAR_BUG
    int i = 0;
#endif
    int ret = uart_drv_read((mdev_t *)dev, buf, len);
    int tmpLen = 0;
    if (0 < ret) {
        do {
            tmpLen += ret;
            APPLOG("snap [%d]", ret);
            ret = uart_drv_read((mdev_t *)dev, buf + tmpLen, len - tmpLen);
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

int halUartWrite(void *dev, const uint8_t *buf, uint32_t len) {
    int ret = 0;
#if TO_DO_FOR_HONYAR_BUG
    int i = 0;
#endif
    ret = uart_drv_write((mdev_t *)dev, buf, len);
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

static struct _gpioTable{
    int8_t id;
    int8_t gpio;
    int8_t used;
    GPIO_PinMuxFunc_Type fun;
} gpioTable[] = {
    {1, 48, 0, GPIO48_GPIO48}, // key
    {2, 49, 0, GPIO49_GPIO49}, // led
    {3, 39, 0, GPIO39_GPIO39}, // hub
};
#define GPIO_SUPPORT_NUM   ((int)(sizeof(gpioTable)/sizeof(gpioTable[0])))

void* halGPIOInit(void) {
    mdev_t *gpio_dev;

    gpio_drv_init();
    if(!(gpio_dev = gpio_drv_open("MDEV_GPIO"))) {
        return NULL;
    }
    return (void *)gpio_dev;
}

int halGPIOOpen(int8_t id, int8_t dir, int8_t mode) {
    int i;
    struct _gpioTable *p;

    for(i = 0, p = gpioTable; i < GPIO_SUPPORT_NUM; i++, p++) {
        if(p->id == id && !p->used) {
            break;
        }
    }
    if(i == GPIO_SUPPORT_NUM) {
        return -1;
    }
    GPIO_PinMuxFun(p->gpio, p->fun);
    switch(dir)
    {
        case GPIO_DIR_INPUT:
            GPIO_SetPinDir(p->gpio, GPIO_INPUT);
            break;
        case GPIO_DIR_OUTPUT:
            GPIO_SetPinDir(p->gpio, GPIO_OUTPUT);
            break;
        default:
            return -1;
    }
    switch(mode)
    {
        case GPIO_MODE_DEFAULT:
            GPIO_PinModeConfig(p->gpio, PINMODE_DEFAULT);
            break;
        case GPIO_MODE_PULLUP:
            GPIO_PinModeConfig(p->gpio, PINMODE_PULLUP);
            break;
        case GPIO_MODE_PULLDOWN:
            GPIO_PinModeConfig(p->gpio, PINMODE_PULLDOWN);
            break;
        case GPIO_MODE_NOPULL:
            GPIO_PinModeConfig(p->gpio, PINMODE_NOPULL);
            break;
        case GPIO_MODE_RISTATE:
            GPIO_PinModeConfig(p->gpio, PINMODE_TRISTATE);
            break;
        default:
            return -1;
    }
    p->used = 1;
    return p->gpio;
}

int halGPIOClose(void *dev) {
    if(!dev) {
        return -1;
    }
    gpio_drv_close(dev);
    return 0;
}

int halGPIORead(void *dev, int gpio, int *val) {
    int ret = gpio_drv_read((mdev_t *)dev, gpio, val);

    if (0 > ret) {
        return -1;
    }
    *val = (*val ==  GPIO_IO_LOW) ? GPIO_STATE_LOW : GPIO_STATE_HIGH;
    return sizeof(*val);
}

int halGPIOWrite(void *dev, int gpio, const int val) {
    int v;

    v = (val ==  GPIO_STATE_LOW) ? GPIO_IO_LOW : GPIO_IO_HIGH;
    gpio_drv_write((mdev_t *)dev, gpio, v);
    return sizeof(val);
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

