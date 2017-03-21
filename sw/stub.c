#include "leconfig.h"
#include "state.h"
#include "io.h"

LELINK_WEAK int aalUserRead(userHandler_t* handler, uint8_t *buf, uint32_t len) {
    return 0;
}

LELINK_WEAK int aalUserWrite(userHandler_t* handler, const uint8_t *buf, uint32_t len) {
    return 0;
}

LELINK_WEAK void halCBStateChanged(StateId from, StateId to) {
}

LELINK_WEAK void *halUartOpen(uartHandler_t* handler) {
    return (void *)0xffffffff;
}

LELINK_WEAK int halUartClose(uartHandler_t* handler) {
    return 0;
}

LELINK_WEAK int halUartRead(uartHandler_t* handler, uint8_t *buf, uint32_t len) {
    return 0;
}

LELINK_WEAK int halUartWrite(uartHandler_t* handler, const uint8_t *buf, uint32_t len) {
    return 0;
}

LELINK_WEAK void *halGPIOInit(void) {
    return (void *)0xffffffff;
}

LELINK_WEAK int halGPIOClose(gpioHandler_t* handler) {
    return 0;
}

LELINK_WEAK int halGPIOOpen(gpioHandler_t* handler) {
    return -1;
}

LELINK_WEAK int halGPIORead(gpioHandler_t* handler, int *val) {
    *val = 1;
    return 0;
}

LELINK_WEAK int halGPIOWrite(gpioHandler_t* handler, const int val) {
    return 0;
}

LELINK_WEAK void halPWMWrite(pwmHandler_t *handler, uint32_t percent) {
    return;
}

LELINK_WEAK void halPWMRead(pwmHandler_t *handler, uint32_t *percent) {
    return;
}

LELINK_WEAK void halPWMSetFrequency(pwmHandler_t *handler) {
    return;
}

LELINK_WEAK int halPWMClose(pwmHandler_t *handler) {
    return 0;
}

LELINK_WEAK int halPWMOpen(pwmHandler_t *handler) {
    return 0;
}

LELINK_WEAK void* halPWMInit(int clock) {
    return (void *)0xffffffff;
}

LELINK_WEAK int halEINTClose(eintHandler_t *handler) {
    return 0;
}

LELINK_WEAK int halEINTOpen(eintHandler_t *handler) {
    return 0;
}

LELINK_WEAK int halEINTRead(eintHandler_t* handler, int *val) {
    return 0;
}

LELINK_WEAK void halCommonInit(commonManager_t* dev) {
    return;
}

LELINK_WEAK void *halPipeOpen(char *name) {
    return (void *)0xffffffff;
}

LELINK_WEAK int halPipeClose(void *dev) {
    return 0;
}

LELINK_WEAK int halPipeRead(void *dev, uint8_t *buf, uint32_t len) {
    return 0;
}

LELINK_WEAK int halPipeWrite(void *dev, const uint8_t *buf, uint32_t len) {
    return len;
}

LELINK_WEAK int halGetWifiChannel() {
    return 0;
}

LELINK_WEAK void aalSetLedStatus(RLED_STATE_t st) {

}

LELINK_WEAK int salResetConfigData(void) {
	return 0;
}

LELINK_WEAK unsigned long halLogTimeStamp(void) {
      return 0;
}

LELINK_WEAK int halWatchDogInit(void) {
    return 0;
}

LELINK_WEAK int halWatchDogDeInit(void) {
    return 0;
}

LELINK_WEAK int halWatchDogFeed(void) {
    return 0;
}

LELINK_WEAK size_t halGetSReservedHeap() {
    return 10*1024;
}
