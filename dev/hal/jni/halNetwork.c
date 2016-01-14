#include "halHeader.h"

int halNwNew(int selfPort, int block, int *sock, int *broadcastEnable) {

    int ret = 0, mode = 1;
    struct sockaddr_in local_addr;
    *sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (selfPort > 0) {
        bzero(&local_addr, sizeof(local_addr));

        local_addr.sin_family = AF_INET;
        local_addr.sin_port = htons(selfPort);
        APPLOG("bind: sock[%d] port[%d]\r\n", *sock, selfPort);
        if((ret = bind(*sock, (struct sockaddr *)&local_addr, sizeof(local_addr))) != 0) 
        {
            APPLOGE("Bind udp port fail: %d.\r\n", ret);
            close(*sock);
            return -1;
        }
    }

	fcntl(*sock, F_SETFL, O_NONBLOCK, 1);
    setsockopt(*sock, SOL_SOCKET, SO_BROADCAST, (char *)broadcastEnable, sizeof(int));

    return 0;
}

int halNwDelete(int sock) {
    close(sock);
    return 0;
}

int halNwUDPSendto(int sock, const char *ip, int port, const uint8_t *buf, int len) {
    int ret;
    struct sockaddr_in to_addr;
    
    memset((char *)&to_addr, 0, sizeof(to_addr));

    to_addr.sin_family = AF_INET;
    to_addr.sin_port = htons(port);
    to_addr.sin_addr.s_addr = inet_addr(ip);
    ret = sendto(sock, buf, len, 0, (struct sockaddr *)&to_addr, sizeof(to_addr));
 
    return ret;
}

int halNwUDPRecvfrom(int sock, uint8_t *buf, int len, char *ip, int lenIP, int *port) {
    int ret;
    struct sockaddr_in from_addr;
    socklen_t len_from = sizeof(struct sockaddr_in);
    ret = recvfrom(sock, buf, len, 0, (struct sockaddr *)&from_addr, (socklen_t *)&len_from);
    if (ret > 0)
    {
        const char *p = (const char *)inet_ntoa(from_addr.sin_addr); 
        int len1 = strlen(p);
        if (lenIP < len1)
        {
            len1 = lenIP;
        }
        *port = htons(from_addr.sin_port);
        strncpy(ip, p, len1);
    }
    return ret;
}

#if 0
int halGetSelfAddr(char *ip, int size, int *port)
{
    void * tmpAddrPtr=NULL;
    struct ifaddrs * ifAddrStruct=NULL;

    size = (strlen(SELF_IP)+1) > size ? size : (strlen(SELF_IP)+1);
    memcpy(ip, SELF_IP, size);// TODO: caution

    return strlen(ip);
}
#else
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <net/if.h>
#define  MAXINTERFACES  5

int halGetSelfAddr(char *ip, int size, int *port)
{
	int sockfd, nIf;
	struct ifconf ifc;
	char ifName[] = "wlan0", *p;
	struct ifreq buf[MAXINTERFACES];
	ifc.ifc_len = sizeof(buf);
	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = (caddr_t) buf;

	if ((sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		APPLOGE("socket ERR\r\n");
		return -1;
	}

	ip[0] = '\0';
	if (!ioctl(sockfd, SIOCGIFCONF, (char *) &ifc)) {
		nIf = ifc.ifc_len / sizeof(struct ifreq);
		while (nIf-- > 0)
		{
			if (strcmp(buf[nIf].ifr_name, ifName) == 0) {
				if (!(ioctl(sockfd, SIOCGIFADDR, (char *) &buf[nIf]))) {
					p = (char *)inet_ntoa(((struct sockaddr_in*) (&buf[nIf].ifr_addr))->sin_addr);
					strncpy(ip, p, size);
					break;
				}
			}
		}
	}
	close(sockfd);
	return strlen(ip);
}
#endif
