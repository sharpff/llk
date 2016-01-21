#include "halHeader.h"
#include <flash.h>
#include <wlan.h>

int halIOWrite(int *type, const char *protocol, int protocolLen, uint8_t *io, int ioLen, void *userData) {
    return 0;
}

int halIORead(int *type, const uint8_t *io, int ioLen, char *protocol, int protocolLen, void *userData) {
    return 0;
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
    return flash_drv_erase((mdev_t *)dev, startAddr, size);
}

int halFlashWrite(void *dev, const uint8_t *data, int len, uint32_t startAddr)
{
    return flash_drv_write((mdev_t *)dev, data, len, startAddr);
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
    APPLOG("mac [%02x:%02x:%02x:%02x:%02x:%02x]\r\n", 
        mac[0], 
        mac[1], 
        mac[2], 
        mac[3], 
        mac[4], 
        mac[5]);
    return 0;
}