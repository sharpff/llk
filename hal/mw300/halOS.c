#include "halHeader.h"
#include "lwip/netdb.h"

int halLockInit(void *ptr, const char *file, int line) {
	return 0;
}

int halLock(void *ptr, const char *file, int line) {
	return 0;
}

int halUnlock(void *ptr, const char *file, int line) {
	return 0;
}

unsigned int halGetTimeStamp(void) {
    // uint64_t tmp = os_get_timestamp();
    // APPLOG("halGetTimeStamp [%lld]", tmp);
    // tmp = tmp / 1000000UL;
    // return (unsigned int)tmp;

    struct tm c_time;
    wmtime_time_get(&c_time);
    c_time.tm_year -= 1900;

    time_t timeUTC = mktime(&c_time);

    // test only
    // int64_t utc = 0;
    // getTerminalUTC(&utc);

    // APPLOG("halGetTimeStamp [%u]", timeUTC);//
    return (unsigned int)timeUTC;
}

unsigned int halGetUTC(void) {
    return 1234;
}
#if 0
void *halMalloc(size_t size) {
    void *ptr = pvPortMalloc(size);
    return ptr;
}

void *halCalloc(int n, size_t size) {
    void *ptr = pvPortMalloc(n*size);
    if (ptr)
        memset(ptr, 0x00, n*size);
    return ptr;
}

void *halRealloc(void *ptr, size_t size) {
    void *ptr1 = pvPortReAlloc(ptr, size);
    return ptr1;
}

void halFree(void *ptr) {
    if (ptr)
        vPortFree(ptr);
}
#endif
void *halMallocEx(size_t size, char* filename, uint32_t line) {
    void *ptr = pvPortMalloc(size);
    //APPLOG("malloc:[%d][0x%x][%d][%s]", size, ptr, line, filename);
    if(ptr==NULL) {
        APPLOG("halMallocEx:%d, %d",  size, xPortGetFreeHeapSize());
    }
    return ptr;
}

void *halCallocEx(int n, size_t size, char* filename, uint32_t line) {
    void *ptr = pvPortMalloc(n*size);
    //APPLOG("calloc:[%d][%d][0x%x][%d][%s]", n*size,xPortGetFreeHeapSize(), ptr, line, filename);
    if(ptr==NULL) {
        APPLOG("halCallocEx:%d, %d",  n*size, xPortGetFreeHeapSize());
    }
    if (ptr) {
        memset(ptr, 0x00, n*size);
    }
    return ptr;
}

void *halReallocEx(void *ptr, size_t size, char* filename, uint32_t line) {
    void *ptr1 = pvPortReAlloc(ptr, size);
    //APPLOG("realloc:[%d][0x%x][%d][%s]", size, ptr1, line, filename);
    //APPLOG("realloc:[%d][0x%x]", size, ptr1);
    if (ptr1==NULL) {
        APPLOG("halReallocEx:%d, %d\n",  size, xPortGetFreeHeapSize());
    }
    return ptr1;
}

void halFreeEx(void *ptr, char* filename, uint32_t line) {
    //APPLOG("halFreeEx:[0x%x][%d][%s]", ptr, line, filename);
    if (ptr)
        vPortFree(ptr);
}

int halReboot(void) {
    app_reboot(REASON_USER_REBOOT);
    return 0;
}

uint16_t halRand() {
    return 0;
}

void halDelayms(int ms) {
    os_thread_sleep(os_msec_to_ticks(ms));
}
