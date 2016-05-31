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
