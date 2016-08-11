#include "halHeader.h"
#include <stdlib.h>
static pthread_mutex_t ginMutex = PTHREAD_MUTEX_INITIALIZER;

int halLockInit(void) {
	return 0;
}

void halDeLockInit(void) {
	
}

int halLock(void) {
    return pthread_mutex_lock(&ginMutex);
}

int halUnlock(void) {
    return pthread_mutex_unlock(&ginMutex);
}

unsigned int halGetTimeStamp(void)
{
	return (unsigned int)time(NULL);
}

unsigned int halGetUTC(void)
{
    return 1234;
}

void *halMalloc(size_t size) {
    void *ptr = malloc(size);
    return ptr;
}

void *halCalloc(int n, size_t size) {
    void *ptr = malloc(n*size);
    if (ptr) {
        memset(ptr, 0x00, n*size);
    }
    return ptr;
}

void halFree(void *ptr) {
    if (ptr)
        free(ptr);
}

int halReboot() {
    return 0;
}

uint16_t halRand() {
    static uint8_t flag = 0;
    uint16_t val = 0;
    if (!flag) {
        srand((int)time(0));
        flag = 1;
    }
    val = 0xFFFF & rand();
    return val;
}

void halDelayms(int ms) {
    usleep(ms*1000);
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