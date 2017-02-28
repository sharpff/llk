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

#if 0
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
#else
void *halMallocEx(size_t size, const char *filename, uint32_t line) {
    void *ptr = malloc(size);
    return ptr;
}

void *halCallocEx(int n, size_t size, const char *filename, uint32_t line) {
    void *ptr = malloc(n*size);
    if (ptr) {
        memset(ptr, 0x00, n*size);
    }
    return ptr;
}

void *halReallocEx(void *ptr, size_t size, const char *filename, uint32_t line) {
    void *ptr1 = realloc(ptr, size);
    return ptr1;
}

void halFreeEx(void *ptr, const char *filename, uint32_t line) {
    if (ptr)
        free(ptr);
}

#endif

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
