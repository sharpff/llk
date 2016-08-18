#include "halHeader.h"
#include "sockets.h"
#include "wifi_api.h"
#include "os.h"

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
        if((ret = bind(*sock, (struct sockaddr *)&local_addr, sizeof(local_addr))) != 0) 
        {
          #ifdef BIND_DEBUG
            while (ret) {
                port += 1;
                local_addr.sin_port = htons(port);
                if((ret = bind(*sock, (struct sockaddr *)&local_addr, sizeof(local_addr))) != 0) {
				  #ifndef __MTK_MT7687_PLATFORM__
                    APPLOGW("rebinding... udp port[%d] fail: %d.", port, ret);
				  #else
					printf("rebinding... udp port[%d] fail: %d.", port, ret);
				  #endif/*__MTK_MT7687_PLATFORM__*/
                }
            }
          #else
		  
		    printf("Bind udp port[%d] fail: %d.", port, ret);
            close(*sock);
            return -1;
          #endif/*BIND_DEBUG*/
        }
	 
	    printf("bind: sock[%d] port[%d]", *sock, port);
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


static bool get_local_ip(void* ip_addr)
{
    u8_t *p_ip_addr =(u8_t *)ip_addr;
	
    if(g_ipv4_addr[0] == 0 && g_ipv4_addr[1] == 0 && g_ipv4_addr[2] == 0 && g_ipv4_addr[3] == 0)
    {
       printf("get_local_ip(), the current is not get ip addr. \n");
	   return false;
    }
	
    (*p_ip_addr) =  g_ipv4_addr[0];
	*(p_ip_addr+1) =  g_ipv4_addr[1];
	*(p_ip_addr+2) =  g_ipv4_addr[2];
	*(p_ip_addr+3)=  g_ipv4_addr[3];
    //printf("get_local_ip(), ip_addr = %d.%d.%d.%d \n",ip_addr[0], ip_addr[1], ip_addr[2], ip_addr[3]);
    printf("get_local_ip(), get ip addr success. \n");
	return true;
}

int halGetSelfAddr(char *ip, int size, int *port) {
    get_local_ip(ip);
    //os_strncpy(ip, p, size);
    
    return os_strlen(ip);
}

int halGetBroadCastAddr(char *broadcastAddr, int len) {
    const char *ip = "255.255.255.255";
    strcpy(broadcastAddr, ip);
    return strlen(ip);
}

int halGetHostByName(const char *name, char ip[4][32], int len) { 
    return -1;
}