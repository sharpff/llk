#include "halHeader.h"
#include <errno.h>

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

void *halGPIOInit(void) {
    return NULL;
}

int halGPIOClose(void *dev) {
    return 0;
}

int halGPIOOpen(int8_t id, int8_t dir, int8_t mode) {
    return -1;
}

int halGPIORead(void *dev, int gpioId, int *val) {
    return 0;
}

int halGPIOWrite(void *dev, int gpioId, const int val) {
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

void *halPipeOpen(char *name) {
    return (void *)0xffffffff;
}

int halPipeClose(void *dev) {
    return 0;
}

int halPipeRead(void *dev, uint8_t *buf, uint32_t len) {
    uint8_t data[] = {0xa5, 0xa5, 0x5a, 0x5a, 0xb9, 0xc0, 0x04, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x04};

    memcpy(buf, data, sizeof(data));
    return sizeof(data);
}

int halPipeWrite(void *dev, const uint8_t *buf, uint32_t len) {
    return len;
}

