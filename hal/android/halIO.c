#include <errno.h>
#include <android/log.h>
#include "halHeader.h"
#include "halCenter.h"

void *halUartOpen(int baud, int dataBits, int stopBits, int parity, int flowCtrl) {
    return (void *)0xffffffff;
}

int halUartClose(void *dev) {
    return 0;
}

int halUartRead(void *dev, uint8_t *buf, uint32_t len) {
    return 0;
}

int halUartWrite(void *dev, const uint8_t *buf, uint32_t len) {
    return len;
}

void *halGPIOInit(void) 
{
    return NULL;
}

int halGPIOClose(void *dev) 
{
    return 0;
}

int halGPIOOpen(int8_t id, int8_t dir, int8_t mode) 
{
    return -1;
}

int halGPIORead(void *dev, int gpioId, int *val) 
{
    return 0;
}

int halGPIOWrite(void *dev, int gpioId, const int val) {
    return 0;
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
    return 0;
}

int halFlashDeinit(void)
{
    return 0;
}

void *halFlashOpen(void)
{
    return (void *)0xffffffff;
}

int halFlashClose(void *dev)
{
    return 0;
}

int halFlashErase(void *dev, uint32_t startAddr, uint32_t size){
    return 0;
}

int halFlashWrite(void *dev, const uint8_t *data, int len, uint32_t startAddr){
    int ret = 0;

    return ret;
}

int halFlashRead(void *dev, uint8_t *data, int len, uint32_t startAddr){
    int ret = 0;

    switch (startAddr) {
        case 0x1C2000:
            ret = sizeof(gNativeContext.authCfg);
            memcpy((char *)data, &gNativeContext.authCfg, ret);
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

void halPrint(const char *log) {
    /*ANDROID_LOG_DEBUG ANDROID_LOG_WARN ANDROID_LOG_ERROR*/
    __android_log_print(ANDROID_LOG_DEBUG, TAG_LOG, log);
}

