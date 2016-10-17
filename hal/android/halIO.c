#include <errno.h>
// #include <android/log.h>
#include "leconfig.h"
#include "halHeader.h"
#include "halCenter.h"

void *halUartOpen(uartHandler_t* handler) {
    return (void *)0xffffffff;
}

int halUartClose(uartHandler_t* handler) {
    return 0;
}

int halUartRead(uartHandler_t* handler, uint8_t *buf, uint32_t len) {
    return 0;
}

int halUartWrite(uartHandler_t* handler, const uint8_t *buf, uint32_t len) {
    return len;
}

void *halGPIOInit(void) {
    return NULL;
}

int halGPIOClose(gpioHandler_t* handler) {
    return 0;
}

int halGPIOOpen(gpioHandler_t* handler) {
    return -1;
}

int halGPIORead(gpioHandler_t* handler, int *val) {
    return 0;
}

int halGPIOWrite(gpioHandler_t* handler, const int val) {
    return 0;
}

void halPWMWrite(pwmHandler_t *handler, uint32_t percent) {
    return;
}

void halPWMRead(pwmHandler_t *handler, uint32_t *percent) {
    return;
}

void halPWMSetFrequency(pwmHandler_t *handler) {
    return;
}

int halPWMClose(pwmHandler_t *handler) {
    return 0;
}

int halPWMOpen(pwmHandler_t *handler) {
    return 0;
}

void* halPWMInit(int clock) {
    return (void *)-1;
}

void halCommonInit(commonManager_t* dev) {
    return;
}

void *halPipeOpen(char *name) {
    return (void *)-1;
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
    return 0;
}

int halFlashDeinit(void)
{
    return 0;
}

void *halFlashOpen(void)
{
    return (void *)-1;
}

int halFlashClose(void *dev)
{
    return 0;
}

int halFlashErase(void *dev, uint32_t startAddr, uint32_t size){
    return 0;
}

int halFlashWrite(void *dev, const uint8_t *data, int len, uint32_t startAddr, int32_t offsetToBegin){
    int ret = 0;
    return ret;
}

int halFlashRead(void *dev, uint8_t *data, int len, uint32_t startAddr, int32_t offsetToBegin){
    int ret = 0;

    switch (startAddr) {
        case 0x1C2000:
            ret = sizeof(gNativeContext.authCfg);
            memcpy((char *)data, &gNativeContext.authCfg, ret);
            break;
        case 0x1C3000:
            ret = sizeof(gNativeContext.scriptCfg);
            memcpy((char *)data, &gNativeContext.scriptCfg, ret);
            break;
        case 0x1C8000:
            ret = sizeof(gNativeContext.privateCfg);
            memcpy((char *)data, &gNativeContext.privateCfg, ret);
            break;
        default:
            break;
    }
    return ret;
}
#ifdef __ANDROID__
#include <android/log.h>
#define androidLog __android_log_print
#else
#define ANDROID_LOG_DEBUG 1
#define ANDROID_LOG_WARN 2
#define ANDROID_LOG_ERROR 3
#define TAG_LOG 0
#define androidLog(l, t, i) printf("%s", i);
#endif
void halPrint(const char *log) {
    /*ANDROID_LOG_DEBUG ANDROID_LOG_WARN ANDROID_LOG_ERROR*/
    const char *str1 = strstr(log, "[W]");
    if (str1) {
        androidLog(ANDROID_LOG_WARN, TAG_LOG, log);
        return;
    }
    const char *str2 = strstr(log, "[E]");
    if (str2) {
        androidLog(ANDROID_LOG_ERROR, TAG_LOG, log);
        return;
    }
    androidLog(ANDROID_LOG_DEBUG, TAG_LOG, log);
}

