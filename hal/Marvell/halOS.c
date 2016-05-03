#include "halHeader.h"

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
    uint64_t tmp = os_get_timestamp();
    APPLOG("halGetTimeStamp [%lld]", tmp);
    tmp = tmp / 1000000UL;
    return (unsigned int)tmp;
}

unsigned int halGetUTC(void) {
    return 1234;
}

void *halCalloc(int n, size_t size) {
    void *ptr = pvPortMalloc(n*size);
    if (ptr)
        memset(ptr, 0x00, n*size);
    return ptr;
}

void halFree(void *ptr) {
    if (ptr)
        vPortFree(ptr);
}

int halReboot(void) {
    app_reboot(REASON_USER_REBOOT);
    return 0;
}