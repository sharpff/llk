#include "halHeader.h"

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

    ioctlsocket(*sock, FIONBIO, &mode);
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
    if (ret > 0)
    {
        const char *p = (const char *)inet_ntoa(from_addr.sin_addr); 
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



int halGetSelfAddr(char *ip, int size, int *port) {
    struct wlan_ip_config ipConfig;
    int ret = 0;
    extern int wlan_get_state();
    ret = wlan_get_address(&ipConfig);
    if (WM_SUCCESS == ret) {
        net_inet_ntoa(ipConfig.ipv4.address, ip);
        ret = strlen(ip);
    } else {
        ret = 0;
    }
    // APPLOG("wlan_get_address [%d] wlan_get_state[%d] ip[%s]\r\n", ret, wlan_get_state(), ip);
    return ret;
}

int halGetBroadCastAddr(char *broadcastAddr, int len) {
    const char *ip = "255.255.255.255";
    strcpy(broadcastAddr, ip);
    return strlen(ip);
}

int halGetHostByName(const char *name, char ip[4][32], int len) { 
    struct hostent* hostinfo;
    struct sockaddr_in tmp;
    if (!isalpha((uint8_t)name[0])) {
        return -1;
    }
    hostinfo = gethostbyname(name);
    APPLOG("halGetHostByName name[%s] hostinfo[0x%p]", name, hostinfo);
    if (NULL == hostinfo) {
        return -2;
    }
    memset(&tmp, 0, sizeof(struct sockaddr_in));
    memcpy(&tmp.sin_addr.s_addr, hostinfo->h_addr, hostinfo->h_length);
    strcpy(ip[0], (const char *)inet_ntoa(tmp.sin_addr));
    APPLOG("halGetHostByName [%s]", ip[0]);
    return 0;
}
// #include <fcntl.h>
// #include <errno.h>
// #include <sys/ioctl.h>
// #include <net/if.h>

// #define  MAXINTERFACES  5

//     // specified the iterface
//     char ifName[] = "eth0";
//     struct ifconf ifc;
//     struct ifreq buf[MAXINTERFACES];
//     char cIP[32] = {0};
//     int tmpSock = 0;
//     ifc.ifc_len = sizeof(buf);
//     ifc.ifc_len = sizeof(buf);
//     ifc.ifc_buf = (caddr_t)buf;
//     if ((tmpSock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP))<0)
//     {
//         APPLOGE("socket ERR");
//         return 0;
//     }

//     if (!ioctl(tmpSock, SIOCGIFCONF, (char *)&ifc)) //ioctl.h
//     {
//         int intrface = ifc.ifc_len / sizeof (struct ifreq);
//         while (intrface-- > 0)
//         {
//             if (strcmp(buf[intrface].ifr_name, ifName) == 0)
//             {
//                 if (!(ioctl(tmpSock, SIOCGIFADDR, (char *)&buf[intrface])))
//                 {
//                     char* ip = inet_ntoa(((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr);
//                     strcpy(cIP, ip);
//                     break;
//                 }
//             }
//         }
//     }

//     close(tmpSock);

//     int cLen = strlen(cIP);
//     if (lenIP < cLen)
//         return 0;

//     memset(ip, 0, lenIP);
//     strncpy(ip, cIP, cLen);
//     return 1;
// }


