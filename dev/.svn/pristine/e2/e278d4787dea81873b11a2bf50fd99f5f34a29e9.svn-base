#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
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

int halGetMac(uint8_t *mac, int len)
{
	int sockfd;
	struct ifreq tmp;
	char mac_addr[30];

	if (6 > len || NULL == mac) {
		return -1;
	}
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		LELOGE("Can't get mac. socket open error\r\n");
		return -1;
	}
	memset(&tmp, 0, sizeof(struct ifreq));
	strncpy(tmp.ifr_name, "wlan0", sizeof(tmp.ifr_name) - 1);
	if ((ioctl(sockfd, SIOCGIFHWADDR, &tmp)) < 0) {
		LELOGE("Can't get mac. socket open error\r\n");
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
	return 0;
}
