#include "halHeader.h"
#include <flash.h>
#include <wlan.h>
#include <rfget.h>
#include <httpc.h>
#include <mw300_uart.h>
#include <lelink/sw/io.h>

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

#define TO_DO_FOR_HONYAR_BUG    1
int halUartRead(void *dev, uint8_t *buf, uint32_t len) {
    int ret = 0;
    int i = 0;
    ret = uart_drv_read((mdev_t *)dev, buf, len);
#if TO_DO_FOR_HONYAR_BUG
    APPLOG("uart_drv_read [%d]", ret);
    if (ret > 0)
    for (i = 0; i < ret; i++) {
        APPPRINTF("%02x ", buf[i]);
    }
    APPPRINTF("\r\n");
#endif
    return ret;
}

int halUartWrite(void *dev, const uint8_t *buf, uint32_t len) {
    int ret = 0;
    int i = 0;
    ret = uart_drv_write((mdev_t *)dev, buf, len);
#if TO_DO_FOR_HONYAR_BUG
    APPLOG("uart_drv_write [%d] ", ret);
    if (ret > 0)
    for (i = 0; i < ret; i++) {
        APPPRINTF("%02x ", buf[i]);
    }
    APPPRINTF("\r\n");
#endif
    return ret;
}

static struct _gpioTable{
    int8_t id;
    int8_t gpio;
    GPIO_PinMuxFunc_Type fun;
} gpioTable[] = {
    {1, 48, GPIO48_GPIO48}, // key
    {2, 49, GPIO49_GPIO49}, // led
    {3, 39, GPIO39_GPIO39}, // hub
};

int halGPIOInit(gpioHand_t *table, int n) {
    int i, j;
    mdev_t *gpio_dev;
    struct _gpioTable *p;

    if(!table || n <= 0) {
        return -1;
    }
    gpio_drv_init();
    if(!(gpio_dev = gpio_drv_open("MDEV_GPIO"))) {
        return -1;
    }
    for( i = 0; i < n; i++ ) {
        p = NULL;
        for(j = 0; j < 3; j++) {
            if(gpioTable[j].id == table[i].id) {
                p = &gpioTable[j];
                break;
            }
        }
        if(!p) {
             continue;
        }
        GPIO_PinMuxFun(p->gpio, p->fun);
        switch(table[i].dir)
        {
            case GPIO_DIR_INPUT:
                GPIO_SetPinDir(p->gpio, GPIO_INPUT);
                break;
            case GPIO_DIR_OUTPUT:
                GPIO_SetPinDir(p->gpio, GPIO_OUTPUT);
                break;
            default:
                continue;
        }
        switch(table[i].mode)
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
                continue;
        }
        if(table[i].dir ==  GPIO_DIR_OUTPUT) {
            switch(table[i].state)
            {
                case GPIO_STATE_LOW:
                case GPIO_STATE_BLINK:
                    GPIO_WritePinOutput(p->gpio, GPIO_IO_LOW);
                    break;
                case GPIO_STATE_HIGH: 
                    GPIO_WritePinOutput(p->gpio, GPIO_IO_HIGH);
                    break;
                default:
                    continue;
            }
        } else {
            GPIO_IntConfig(p->gpio, GPIO_INT_DISABLE);
            // switch(table[i].inter) { default: continue; } // TODO: need to support interrupt
        }
        table[i].num = p->gpio;
        table[i].priv = gpio_dev;
        APPLOGE("Debug i = %d, id = %d, num = %d, dir = %d, mode = %d,  inter = %d,  state = %d", 
                i, table[i].id, table[i].num, table[i].dir, table[i].mode, table[i].inter, table[i].state);
    }
    return 0;
}

int halGPIOClose(void *table) {
    if(!table) {
        return -1;
    }
    return gpio_drv_close(((gpioHand_t *)table)->priv);
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
    int ret, v;

    v = (val ==  GPIO_STATE_LOW) ? GPIO_IO_LOW : GPIO_IO_HIGH;
    APPLOGE("halwrite, %d <- %d", gpio, v);
    ret = gpio_drv_write((mdev_t *)dev, gpio, v);
    if (0 > ret) {
        return -1;
    }    
    return sizeof(val);
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

