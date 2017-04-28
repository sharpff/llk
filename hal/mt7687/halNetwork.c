#include "leconfig.h"
#include "halHeader.h"
#include "sockets.h"
#include "wifi_api.h"
#include "os.h"
#include "netdb.h"
#include "ctype.h"

extern uint8_t g_ipv4_addr[4];

int halNwNew(int selfPort, int block, int *sock, int *broadcastEnable) {

    int ret = 0/*, mode = 1*/;
    struct sockaddr_in local_addr;
    *sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    uint16_t port = (uint16_t)selfPort;
    if (port > 0) {
	  
	    os_memset(&local_addr, 0x00, sizeof(local_addr));
	  
        local_addr.sin_family = AF_INET;
        local_addr.sin_port = htons(port);
        if((ret = bind(*sock, (struct sockaddr *)&local_addr, sizeof(local_addr))) != 0) {
          #ifdef BIND_DEBUG
            while (ret) {
                port += 1;
                local_addr.sin_port = htons(port);
                if((ret = bind(*sock, (struct sockaddr *)&local_addr, sizeof(local_addr))) != 0) {
					APPLOG("rebinding... udp port[%d] fail: %d.", port, ret);
                }
            }
          #else
		  
		    APPLOG("Bind udp port[%d] fail: %d.", port, ret);
            close(*sock);
            return -1;
          #endif/*BIND_DEBUG*/
        }
	 
	    APPLOG("bind: sock[%d] port[%d]", *sock, port);
    }

	//fcntl(*sock, F_SETFL, O_NONBLOCK, 1);
	fcntl( *sock, F_SETFL, fcntl( *sock, F_GETFL, 0 ) | O_NONBLOCK );//set non_block;
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
    
    os_memset((char *)&to_addr, 0, sizeof(to_addr));

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
	  
	    int size = os_strlen(p) + 1;
        if (sizeIP < size) {
            size = sizeIP;
        }
        *port = htons(from_addr.sin_port);
	 
	    os_memcpy(ip, p, size - 1);
	 
        ip[size - 1] = 0;
    }
    return ret;
}


int halGetSelfAddr(char *ip, int size, int *port) {

    char *p;
    ip[0] = '\0';
    if(g_ipv4_addr[0] == 0 && g_ipv4_addr[1] == 0 && g_ipv4_addr[2] == 0 && g_ipv4_addr[3] == 0) {
       //APPLOG("halGetSelfAddr(), the current is not get ip addr.");
       return 0;
    }
    p = inet_ntoa(g_ipv4_addr);
    strncpy(ip, p, size);
    return os_strlen(ip);
}

int halGetBroadCastAddr(char *broadcastAddr, int len) {
    const char *ip = "255.255.255.255";
    strcpy(broadcastAddr, ip);
    return strlen(ip);
}
#define NAME_SIZE 32
static char ginIP[NAME_SIZE];
static int  ginIPdone = 0;
static TaskHandle_t hostByName = NULL;

static void hal_hostbyname_proc(void *args) {
    struct hostent* hostinfo;
    struct sockaddr_in tmp;
    char* name = (char *)args;
    os_memset(ginIP, 0, NAME_SIZE);
    while (1) {
        hostinfo = lwip_gethostbyname(name);
        APPLOG("halGetHostByName name[%s] hostinfo[%p]", name, hostinfo);
        if (NULL != hostinfo) {
            break;
        }
        halDelayms(500);
    }
    ginIPdone = 1;
    os_memset(&tmp, 0, sizeof(struct sockaddr_in));
    os_memcpy(&tmp.sin_addr.s_addr, hostinfo->h_addr, hostinfo->h_length);
    strcpy(ginIP, (const char *)inet_ntoa(tmp.sin_addr));
    APPLOG("halGetHostByName [%s]", ginIP);
    vTaskDelete(NULL);
}

int halGetHostByNameNB(const char *name, char ip[4][32], int len) {
    int nameLen = strlen(name);
    static char ginName[NAME_SIZE] = {0};
    if ((nameLen + 1) > NAME_SIZE) {
        LOG_E(common, "url is too long!");
        return -1;
    }
    os_memset(ginName, 0, sizeof(ginName));
    os_memcpy(ginName, name, nameLen);
    if (!isalpha((uint8_t)ginName[0])) {
        LOG_E(common, "not a url!");
        return -2;
    }
    APPLOG("halGetHostByNameNB [%s]", ginName);
    if (ginIPdone) {
        os_memcpy(ip[0], ginIP, NAME_SIZE);
        return 0;
    }
    ginIPdone = 0;
    if (hostByName == NULL) {
        if (pdPASS != xTaskCreate(hal_hostbyname_proc,
                              "hal_hostbyname_proc",
                              1024,
                              ginName,
                              1,
                              &hostByName)) {
            LOG_E(common, "create host name task fail");
            return -3;
        }
    }
    return -4;
}

int halGetHostByName(const char *name, char ip[4][32], int len) { 
    struct hostent* hostinfo;
    struct sockaddr_in tmp;
    if (!isalpha((uint8_t)name[0])) {
        return -1;
    }
    hostinfo = lwip_gethostbyname(name);
    APPLOG("halGetHostByName name[%s] hostinfo[0x%p]", name, hostinfo);
    if (NULL == hostinfo) {
        return -2;
    }
    os_memset(&tmp, 0, sizeof(struct sockaddr_in));
    os_memcpy(&tmp.sin_addr.s_addr, hostinfo->h_addr, hostinfo->h_length);
    strcpy(ip[0], (const char *)inet_ntoa(tmp.sin_addr));
    APPLOG("halGetHostByName [%s]", ip[0]);
    return 0;
}
