#include "halHeader.h"
#include <errno.h>

void *halUartOpen(int baud, int dataBits, int stopBits, int parity, int flowCtrl) {
    return (void *)0xffffffff;
}

int halUartClose(void *dev) {
    return 0;
}

int halUartRead(void *dev, uint8_t *buf, uint32_t len) {
    return len;
}

int halUartWrite(void *dev, const uint8_t *buf, uint32_t len) {
    return len;
}

void *halGPIOInit(int gpioId, int isInput, int initVal) {
    return NULL;
}
int halGPIOClose(void *dev) {
    return 0;
}
int halGPIORead(void *dev, uint8_t *buf, uint32_t len) {
    return 0;
}
int halGPIOWrite(void *dev, const uint8_t *buf, uint32_t len) {
    return 0;
}


int halFlashInit(void)
{
    return 0;
}

int halFlashDeinit(void)
{
    return 0;
}


static int ginMinSize = 0x1000; // 4k
#define GET_PAGE_SIZE(l) \
    ((((l - 1) / ginMinSize) + 1)*ginMinSize)
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
    int fd, ret, append;
    char fileName[64] = {0};
    sprintf(fileName, "./0x%x.bin", startAddr);
    fd = open(fileName, O_WRONLY | O_CREAT);
    if (0 >= fd) {
        APPLOG("WRITE FAILED [%d]\r\n", errno);
        return fd;
    }

    ret = write(fd, data, len);
    append = GET_PAGE_SIZE(len) - len;
    if (0 < append) {
        uint8_t byte = 0xFF;
        while (append--) {
            ret += write(fd, &byte, 1);
        }
    }
    close(fd);
    APPLOG("WRITE OK [%s] size[0x%x]\r\n", fileName, ret);
    return ret;
}

int halFlashRead(void *dev, uint8_t *data, int len, uint32_t startAddr){
    int fd, ret;
    char fileName[64] = {0};
    sprintf(fileName, "./0x%x.bin", startAddr);
    fd = open(fileName, O_RDONLY);
    if (0 >= fd) {
        // printf("errno [%d]", errno);
        // APPLOG("READ FAILED [%d]", errno);
        return fd;
    }
    ret = read(fd, data, len);
    close(fd);
    // APPLOG("READ OK [%s]", fileName);
    return ret;
}

int halGetMac(uint8_t *mac, int len) {
    if (6 > len || NULL == mac) {
        return -1;
    }

    mac[0] = 0xEE;
    mac[1] = 0xEE;
    mac[2] = 0xEE;
    mac[3] = 0xEE;
    mac[4] = 0xEE;
    mac[5] = 0xEE;
    return 0;
}
