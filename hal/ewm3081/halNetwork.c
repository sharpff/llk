#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "halHeader.h"
#include "mico.h"
#include "MicoSocket.h"

#define  MAXINTERFACES  5

////////////////////////////////////////////////////////////////////////////////
#define sockaddr_in sockaddr_t
#define PF_INET AF_INET
#define SOCK_DGRAM SOCK_DGRM
////////////////////////////////////////////////////////////////////////////////

//网络配置信息
extern IPStatusTypedef t11_network_info;

const char* stateIdStr[] = {
    "E_STATE_NONE",
    "E_STATE_START",
    "E_STATE_CONFIGURING",
    "E_STATE_SNIFFER_GOT",
    "E_STATE_AP_CONNECTING",
    "E_STATE_AP_CONNECTED",
    "E_STATE_CLOUD_LINKED",
    "E_STATE_CLOUD_AUTHED",
    "E_STATE_MAX"
};

static int port_num =3332;
int halNwNew(int selfPort, int block, int *sock, int *broadcastEnable) {
    
    int ret = 0, mode = 1;
    struct sockaddr_in local_addr;
    *sock = socket(AF_INET, SOCK_DGRM, IPPROTO_UDP);
    if(*sock < 0) {
        APPLOGE("create udp socket failed,ret = %d", *sock);
        return -1;
    }
    
    uint16_t port = (uint16_t) selfPort;
    
    if(port<=0){
        APPLOGE("port = %d",port_num);
        port = port_num++;
    }
    
    if(port > 0) {
        //将udp socket绑定到本地
        memset(&local_addr, 0, sizeof(local_addr));
        local_addr.s_ip = INADDR_ANY;
        local_addr.s_port = port;
        
        if ((ret = bind(*sock, (struct sockaddr *) &local_addr, sizeof(local_addr))) != 0) {
            APPLOGE("udp bind [%d] error，ret = %d", port, ret);
            close(*sock);
            return -1;
        }
        
        APPLOGE("udp[%d] bind success, ret = %d,sock = %d", port, ret, *sock);
    }
    
    int is_block = !block;  //mico中0代表block，1代表non-blocking
    //    setsockopt(*sock, SOL_SOCKET, SO_BLOCKMODE, &is_block, sizeof(block));
    //    setsockopt(*sock, SOL_SOCKET, SO_BROADCAST, (char *) broadcastEnable,sizeof(int));
    APPLOG("socket create success, sock = %d, selfPort = %d, block = %d", *sock, selfPort, block);
    return 0;
}

int halNwDelete(int sock) {
    
    close(sock);
    return 0;
}

int halNwUDPSendto(int sock, const char *ip, int port, const uint8_t *buf, int len) {

    int ret;
    struct sockaddr_in to_addr;
    
    memset((char *) &to_addr, 0, sizeof(to_addr));
    //to_addr.sin_family = AF_INET;
    to_addr.s_port = port;
    to_addr.s_ip = inet_addr(ip);
    ret = sendto(sock, buf, len, 0, (struct sockaddr *) &to_addr, sizeof(to_addr));
    APPLOG("send data done, sock = %d, ip = %s, port = %d, buf = %p, len = %d", sock, ip, port, buf, len);
    //t11_print_mem(buf, len);
    return ret;
}

int halNwUDPRecvfrom(int sock, uint8_t *buf, int len, char *ip, int sizeIP, uint16_t *port) {
    
    
    int ret = 0;
    fd_set readfds;
    struct timeval_t t;
    struct sockaddr_t server_addr = {0};
    socklen_t  server_addrLen = sizeof(server_addr);
    
    uint32_t time_start = mico_get_time();
    
    FD_ZERO(&readfds);
    t.tv_sec = 0;
    t.tv_usec = 100;
    FD_SET(sock, &readfds);
    select(1, &readfds, NULL, NULL, &t);
    
    
    
    if (FD_ISSET(sock, &readfds)) {
        ret = recvfrom(sock, buf, len, 0, &server_addr, &server_addrLen);
        uint8_t *s_ip = (uint8_t*)(&(server_addr.s_ip));
        sprintf(ip,"%d.%d.%d.%d", s_ip[3], s_ip[2], s_ip[1], s_ip[0]);
        *port = server_addr.s_port;
        APPLOG("receive data done, sockfd = %d, len = %d, ret = %d, ip = %s, port = %d, sock_addr:",sock, len,ret, ip, *port);
        
    } else {
        //        APPLOGE("网络数据接收完成，sockfd = %d，len = %d， ret = %d, 用时 %d ms", sock, len, ret,mico_get_time()-time_start);
    }
    if(ret == 0) {
        ret = -1;
    }
    return ret;
}

int halGetSelfAddr(char *ip, int size, int *port) {
    //    
    strncpy(ip, t11_network_info.ip, size);
    return strlen(ip);
}


int halGetMac(uint8_t *mac, int len) {
    
    
    wlan_get_mac_address(mac);
    APPLOG("get mac address:");
    t11_print_mem(mac, len);
    return 0;
}

int halGetBroadCastAddr(char *broadcastAddr, int len) {
    
    strncpy(broadcastAddr, t11_network_info.broadcastip, len);
    APPLOG("broadcast address:%s", broadcastAddr);
    return strlen(broadcastAddr);
}

int halGetHostByName(const char *name, char ip[4][32], int len) { 
    char ipstr[16] = {0};
    int err;
    
    if (NULL == name || 4*32 > len) {
        APPLOGE("param error");
        return -1;
    }
    err = gethostbyname(name, (uint8_t *)ipstr, 16);
    APPLOG("%s server address: %d %s", name, err, ipstr);
    if(err == 0) {
        strcpy(ip[0], ipstr);
        return 0;
    } else {
        return -2;
    }
}

int halGetHostByNameNB(const char *name, char ip[4][32], int len) { 
    return halGetHostByName(name, ip, len);
}