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

// #include <push_button.h>
// static int ginGPIOFlag = 0;
// void callbackGPIORaisingEdgeInt(int pin, void *data) {
//     ginGPIOFlag = 1;
// }
// void callbackGPIOFallingEdgeInt(int pin, void *data) {
//     ginGPIOFlag = 0;
// }
static int ginGpioId;
void *halGPIOInit(int gpioId, int isInput, int initVal) {
    mdev_t * gpio_dev;
    gpio_drv_init();
    /*
     * TODO: all these configuration need to be got from profile
     */
    APPLOG("halGPIOInit gpioId[%d], isInput[%d], initVal[%d]", gpioId, isInput, initVal);
    gpio_dev = gpio_drv_open("MDEV_GPIO");
    if (gpio_dev) {
        // gpioid 39 <=> GPIO_39
        GPIO_PinMuxFun(gpioId, PINMUX_FUNCTION_0);
        GPIO_SetPinDir(gpioId, isInput ? GPIO_INPUT : GPIO_OUTPUT);

        // initVal:
        // PINMODE_DEFAULT = 0,                      /*!< GPIO pin mode default define */
        // PINMODE_PULLUP,                          /*!< GPIO pin mode pullup define */
        // PINMODE_PULLDOWN,                        /*!< GPIO pin mode pulldown define */
        // PINMODE_NOPULL,                          /*!< GPIO pin mode nopull define */
        // PINMODE_TRISTATE,                        /*!< GPIO pin mode tristate define */
        GPIO_PinModeConfig(gpioId, initVal);        
        ginGpioId = gpioId;
    }
    // gpio_drv_set_cb(gpio_dev, gpioId, GPIO_INT_RISING_EDGE,  NULL, callbackGPIORaisingEdgeInt);
    // gpio_drv_set_cb(gpio_dev, gpioId, GPIO_INT_FALLING_EDGE,  NULL, callbackGPIOFallingEdgeInt);

    return (void *)gpio_dev;
}
int halGPIOClose(void *dev) {
    return 0;
}
int halGPIORead(void *dev, int gpioId, int *val) {
    int ret = gpio_drv_read((mdev_t *)dev, gpioId, val);
    // APPLOG("gpio read ret[%d] val[%d]", ret, val);
    if (0 > ret) {
        return 0;
    }
    return sizeof(*val);
}
int halGPIOWrite(void *dev, int gpioId, const int val) {
    int ret = gpio_drv_write((mdev_t *)dev, gpioId, val);
    if (0 > ret) {
        return 0;
    }    
    // APPLOG("gpio write ret[%d] ", ret);
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

