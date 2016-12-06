#include "leconfig.h"
#include "io.h"

__attribute__((weak)) int aalUserRead(userHandler_t* handler, uint8_t *buf, uint32_t len) {
    return 0;
}

__attribute__((weak)) int aalUserWrite(userHandler_t* handler, const uint8_t *buf, uint32_t len) {
    return 0;
}

__attribute__((weak)) void *halUartOpen(uartHandler_t* handler) {
    return (void *)0xffffffff;
}

__attribute__((weak)) int halUartClose(uartHandler_t* handler) {
    return 0;
}

__attribute__((weak)) int halUartRead(uartHandler_t* handler, uint8_t *buf, uint32_t len) {
    return 0;
}

__attribute__((weak)) int halUartWrite(uartHandler_t* handler, const uint8_t *buf, uint32_t len) {
    return 0;
}

__attribute__((weak)) void *halGPIOInit(void) {
    return (void *)0xffffffff;
}

__attribute__((weak)) int halGPIOClose(gpioHandler_t* handler) {
    return 0;
}

__attribute__((weak)) int halGPIOOpen(gpioHandler_t* handler) {
    return -1;
}

__attribute__((weak)) int halGPIORead(gpioHandler_t* handler, int *val) {
    *val = 1;
    return 0;
}

__attribute__((weak)) int halGPIOWrite(gpioHandler_t* handler, const int val) {
    return 0;
}

__attribute__((weak)) void halPWMWrite(pwmHandler_t *handler, uint32_t percent) {
    return;
}

__attribute__((weak)) void halPWMRead(pwmHandler_t *handler, uint32_t *percent) {
    return;
}

__attribute__((weak)) void halPWMSetFrequency(pwmHandler_t *handler) {
    return;
}

__attribute__((weak)) int halPWMClose(pwmHandler_t *handler) {
    return 0;
}

__attribute__((weak)) int halPWMOpen(pwmHandler_t *handler) {
    return 0;
}

__attribute__((weak)) void* halPWMInit(int clock) {
    return (void *)0xffffffff;
}

__attribute__((weak)) int halEINTClose(eintHandler_t *handler) {
    return 0;
}

__attribute__((weak)) int halEINTOpen(eintHandler_t *handler) {
    return 0;
}

__attribute__((weak)) int halEINTRead(eintHandler_t* handler, int *val) {
    return 0;
}

__attribute__((weak)) void halCommonInit(commonManager_t* dev) {
    return;
}

__attribute__((weak)) void *halPipeOpen(char *name) {
    return (void *)0xffffffff;
}

__attribute__((weak)) int halPipeClose(void *dev) {
    return 0;
}

__attribute__((weak)) int halPipeRead(void *dev, uint8_t *buf, uint32_t len) {
    return 0;
}

__attribute__((weak)) int halPipeWrite(void *dev, const uint8_t *buf, uint32_t len) {
    return len;
}

__attribute__((weak)) int halGetWifiChannel() {
    return 0;
}

__attribute__((weak)) void aalSetLedStatus(RLED_STATE_t st) {

}

__attribute__((weak)) int salResetConfigData(void) {
	return 0;
}
