#include "leconfig.h"
int halFlashInit(void)
{
    return 0;
}

int halFlashDeinit(void)
{
    return 0;
}

int halIOWrite(int type, const char *protocol, int protocolLen, uint8_t *io, int ioLen, void *userData) {
    return 0;
}

int halIORead(int type, const uint8_t *io, int ioLen, char *protocol, int protocolLen, void *userData) {
    return 0;
}


void *halFlashOpen(void){
	return (void *)0xFFFFFFFF;
}

int halFlashClose(void *dev){
    return 0;
}

int halFlashErase(void *dev, uint32_t startAddr, uint32_t size){
    return 0;
}

int halFlashWrite(void *dev, const uint8_t *data, int len, uint32_t startAddr){
    return 0;
}

int halFlashRead(void *dev, uint8_t *data, int len, uint32_t startAddr){
    return 0;
}

int halGetMac(uint8_t *mac, int len) {
    if (6 > len || NULL == mac) {
        return -1;
    }

    mac[0] = 0x12;
    mac[1] = 0x34;
    mac[2] = 0x56;
    mac[3] = 0xAB;
    mac[4] = 0xCD;
    mac[5] = 0xEF;
    return 0;
}
