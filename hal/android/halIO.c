#include <errno.h>
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

int halGetMac(uint8_t *mac, int len)
{
#if 1
    memcpy(mac, gNativeContext.mac, sizeof(gNativeContext.mac));
    return 0;
#else
	int sockfd;
	struct ifreq tmp;
	char mac_addr[30];

	if (6 > len || NULL == mac) {
		return -1;
	}
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		APPLOGE("Can't get mac. socket open error\r\n");
		return -1;
	}
	memset(&tmp, 0, sizeof(struct ifreq));
	strncpy(tmp.ifr_name, "wlan0", sizeof(tmp.ifr_name) - 1);
	if ((ioctl(sockfd, SIOCGIFHWADDR, &tmp)) < 0) {
		APPLOGE("Can't get mac. socket open error\r\n");
		close(sockfd);
		return -1;
	}
	mac[0] = tmp.ifr_hwaddr.sa_data[0];
	mac[1] = tmp.ifr_hwaddr.sa_data[1];
	mac[2] = tmp.ifr_hwaddr.sa_data[2];
	mac[3] = tmp.ifr_hwaddr.sa_data[3];
	mac[4] = tmp.ifr_hwaddr.sa_data[4];
	mac[5] = tmp.ifr_hwaddr.sa_data[5];
	close(sockfd);
#endif
	return 0;
}
