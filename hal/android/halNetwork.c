#include "halHeader.h"
#include "halCenter.h"

int halNwNew(int selfPort, int block, int *sock, int *broadcastEnable) {

    int ret = 0, mode = 1;
    struct sockaddr_in local_addr;
    *sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    uint16_t port = (uint16_t)selfPort;
    if (port > 0) {
        bzero(&local_addr, sizeof(local_addr));

        local_addr.sin_family = AF_INET;
        local_addr.sin_port = htons(port);
        if((ret = bind(*sock, (struct sockaddr *)&local_addr, sizeof(local_addr))) != 0) 
        {
            #ifdef BIND_DEBUG
            while (ret) {
                port += 1;
                local_addr.sin_port = htons(port);
                if((ret = bind(*sock, (struct sockaddr *)&local_addr, sizeof(local_addr))) != 0) {
                    APPLOGE("rebinding... udp port[%d] fail: %d.\r\n", port, ret);
                }
            }
            #else
            APPLOGE("Bind udp port[%d] fail: %d.\r\n", port, ret);
            close(*sock);
            return -1;
            #endif
        }
        APPLOG("bind: sock[%d] port[%d]\r\n", *sock, port);
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
    APPLOGW("sendto %s:%d, ret = %d", ip, port, ret);
 
    return ret;
}

// int halNwUDPRecvfrom(int sock, uint8_t *buf, int len, char *ip, int lenIP, int *port) {
//     int ret;
//     struct sockaddr_in from_addr;
//     socklen_t len_from = sizeof(struct sockaddr_in);
//     ret = recvfrom(sock, buf, len, 0, (struct sockaddr *)&from_addr, (socklen_t *)&len_from);
//     if (ret > 0)
//     {
//         const char *p = (const char *)inet_ntoa(from_addr.sin_addr); 
//         int len1 = strlen(p);
//         if (lenIP < len1)
//         {
//             len1 = lenIP;
//         }
//         *port = htons(from_addr.sin_port);
//         strncpy(ip, p, len1);
//     }
//     return ret;
// }

int halNwUDPRecvfrom(int sock, uint8_t *buf, int len, char *ip, int sizeIP, uint16_t *port) {
    int ret;
    struct sockaddr_in from_addr;
    socklen_t len_from = sizeof(struct sockaddr_in);
    ret = recvfrom(sock, buf, len, 0, (struct sockaddr *)&from_addr, (socklen_t *)&len_from);
    if (ret >= 0) {
        char p[16] = {0};
        inet_ntop(AF_INET, (void *)&(from_addr.sin_addr), p, sizeof(p));
        int size = strlen(p) + 1;
        if (sizeIP < size) {
            size = sizeIP;
        }
        *port = htons(from_addr.sin_port);
        memcpy(ip, p, size - 1);
        ip[size - 1] = 0;
    }
    return ret;
}

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

int halGetBroadCastAddr(char *broadcastAddr, int len) {
    const char *ip = "1.2.3.255";
    strcpy(broadcastAddr, ip);
    return strlen(ip);
}
