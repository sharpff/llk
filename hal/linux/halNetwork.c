#include "halHeader.h"
#include <errno.h>

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
                    APPLOGW("rebinding... udp port[%d] fail: %d.", port, ret);
                }
            }
            #else
            APPLOGE("Bind udp port[%d] fail: %d.", port, ret);
            close(*sock);
            return -1;
            #endif
        }
        APPLOG("bind: sock[%d] port[%d]", *sock, port);
        // test only
        if (0)
        {
            struct ip_mreq mreq; 
            struct sockaddr_in ia;  
            bzero(&mreq, sizeof (struct ip_mreq)); 
            inet_pton(AF_INET,"239.101.1.1",&ia.sin_addr);
            /* 设置组地址 */ 
            bcopy (&ia.sin_addr.s_addr, &mreq.imr_multiaddr.s_addr, sizeof (struct in_addr)); 
            /* 设置发送组播消息的源主机的地址信息 */ 
            mreq.imr_interface.s_addr = htonl (INADDR_ANY);  
            if (setsockopt(*sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq,sizeof (struct ip_mreq)) == -1)
            {     
                APPLOGE("setsockopt() IP_ADD_MEMBERSHIP failed!");
                return -2;   
            }
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

int halGetSelfAddr(char *ip, int size, int *port) {
    // struct ifaddrs * ifAddrStruct = NULL;
    // void * tmpAddrPtr = NULL;
    size = (strlen(SELF_IP)+1) > size ? size : (strlen(SELF_IP)+1);

    memcpy(ip, SELF_IP, size);// TODO: caution

    // test only
    return strlen(ip);
}

int halGetBroadCastAddr(char *broadcastAddr, int len) {
    const char *ip = "255.255.255.255";
    // const char *ip = "192.168.0.255";
    strcpy(broadcastAddr, ip);
    return strlen(ip);
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

static int intCastProbing(int sock, struct sockaddr_in *address) {
    #define PROB_TIMES 4
    int times = PROB_TIMES, ret = 0, isSupport = 0;
    uint16_t data = genRand(), sendData = 0, recvData = 0;
    while (times--) {
        int retryTimes = 3;
        sendData = data + times;
        ret = sendto(sock, &sendData, sizeof(sendData), 0, (struct sockaddr *)address, sizeof(*address));
        if (0 >= ret) {
            delayMS(10);
            APPLOGE("intCastProbing sendto [%d]", ret);
            continue;
        }
        APPLOG("intCastProbing sendto [%d] sendData[0x%04x]", ret, sendData);    
        while (retryTimes--) {
            char localIP[32] = {0};
            char fromIP[32] = {0};
            int localPort = 0;
            socklen_t lenFrom = sizeof(struct sockaddr_in);
            ret = recvfrom(sock, &recvData, sizeof(recvData), 0, (struct sockaddr *)address, &lenFrom);
            if (0 >= ret) {
                delayMS(10);
                continue;
            }
            APPLOG("intCastProbing recvfrom [%d] recvData[0x%04x]", ret, recvData); 
            ret = halGetSelfAddr(localIP, sizeof(localIP), &localPort);
            if (0 >= ret) {
                APPLOGE("intCastProbing halGetSelfAddr FAILED");
                break;
            }
            inet_ntop(AF_INET, (void *)&(address->sin_addr), fromIP, sizeof(fromIP));
            APPLOG("intCastProbing fromIP[%s], localIP[%s]", fromIP, localIP); 
            if (sendData == recvData && (0 == strncmp(localIP, fromIP, ret))) {
                isSupport++;
                APPLOG("intCastProbing times[%d] done", times);
                break;
            }
        }
        delayMS(10);
    }

    // return 0;
    return isSupport == PROB_TIMES;
}


int halCastProbing(const char *mcastIP, const char *bcastIP, int port) {
    struct sockaddr_in addrLocal;
    struct sockaddr_in addrTmp;  
    struct sockaddr_in address;  
    int sock, ret = 0;
    struct ip_mreq mreq; 
    int isSupportMCast = 0;
    int isSupportBCast = 0;
    int broadcastEnable = 1;

    sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    // enable multicast
    bzero(&mreq, sizeof (struct ip_mreq)); 
    inet_pton(AF_INET, mcastIP, &addrTmp.sin_addr);
    bcopy (&addrTmp.sin_addr.s_addr, &mreq.imr_multiaddr.s_addr, sizeof (struct in_addr)); 
    mreq.imr_interface.s_addr = htonl (INADDR_ANY);  
    if (-1 == setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq,sizeof (struct ip_mreq)))
    {     
        APPLOGE("halCastProbing setsockopt() IP_ADD_MEMBERSHIP failed!");
        close(sock);
        return -1;   
    }

    // enable broadcast
    if (-1 == setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *)&broadcastEnable, sizeof(int))) {
        APPLOGE("halCastProbing setsockopt() SO_BROADCAST failed!");
        close(sock);
        return -2;
    }

    // non block socket
    fcntl(sock, F_SETFL, O_NONBLOCK, 1);

    // bind port
    addrLocal.sin_family = AF_INET;
    addrLocal.sin_port = htons(port);
    addrLocal.sin_addr.s_addr = htonl(INADDR_ANY);
    if((ret = bind(sock, (struct sockaddr *)&addrLocal, sizeof(addrLocal))) != 0) {
        APPLOGE("halCastProbing bind() port[%d] failed errno[%d]!", port, errno);
        close(sock);
        return -3;
    }

    APPLOG("probing for mcast START");
    address.sin_addr.s_addr = inet_addr(mcastIP);
    isSupportMCast = intCastProbing(sock, &address);
    APPLOG("probing for mcast [%d] END", isSupportMCast)
    APPLOG("probing for bcast START");
    address.sin_addr.s_addr = inet_addr(bcastIP);
    isSupportBCast = intCastProbing(sock, &address);
    APPLOG("probing for bcast [%d] END", isSupportBCast);
    close(sock);

    return isSupportMCast + isSupportBCast;
}

int halGetHostByName(const char *name, char ip[4][32], int len) { 
    char **pptr;
    struct hostent *hptr;
    int i = 0;
    struct sockaddr_in addr;

    if (NULL == name || 
        4*32 > len) {
        APPLOGE("param error");
        return -1;
    }

    if (inet_aton (name, &addr.sin_addr)) {
        APPLOGE("not dns %s\n", name);
        return -2;
    }

    /* 调用gethostbyname()。调用结果都存在hptr中 */  
    if ((hptr = gethostbyname(name)) == NULL) {   
        APPLOGE("gethostbyname error for host:%s\n", name);
        return -3; /* 如果调用gethostbyname发生错误，返回1 */  
    }   
    /* 将主机的规范名打出来 */  
    APPLOG("official hostname:%s\n",hptr->h_name);
    /* 主机可能有多个别名，将所有别名分别打出来 */  
    for (pptr = hptr->h_aliases; *pptr != NULL; pptr++)   
        APPLOG(" alias:%s\n",*pptr);
    /* 根据地址类型，将地址打出来 */  
    switch(hptr->h_addrtype) {
        case AF_INET:   
        case AF_INET6:   
            pptr=hptr->h_addr_list; 
        /* 将刚才得到的所有地址都打出来。其中调用了inet_ntop()函数 */  
        for (i = 0; *pptr != NULL && i < 4; pptr++, i++) {
            inet_ntop(hptr->h_addrtype, *pptr, ip[i], sizeof(ip[i]));
            APPLOG(" address:%s\n", ip[i]);
        }
        break;
        default:   
            APPLOG("unknown address type\n");
        break;
    }   
    return 0;
}