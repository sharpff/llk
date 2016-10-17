#include "halHeader.h"
static pthread_mutex_t ginMutex = PTHREAD_MUTEX_INITIALIZER;

int halLockInit(void *ptr, const char *file, int line) {
	return 0;
}

void halDeLockInit(void) {
	
}

int halLock(void *ptr, const char *file, int line) {
    return pthread_mutex_lock(&ginMutex);
}

int halUnlock(void *ptr, const char *file, int line) {
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
void *halCalloc(int n, size_t size) {
    void *ptr = malloc(n*size);
    if (ptr) {
        memset(ptr, 0x00, n*size);
    }
    return ptr;
}

void *halMalloc(size_t size) {
    void *ptr;
    return ptr;
}

void *halRealloc(void *ptr, size_t size) {
    void *ptr1;
    return ptr1;
}

void halFree(void *ptr) {
    if (ptr)
        free(ptr);
}
#else
void *halMallocEx(size_t size, const char *filename, uint32_t line) {
    void *ptr;
    return ptr;
}

void *halCallocEx(int n, size_t size, const char *filename, uint32_t line) {
    void *ptr = malloc(n*size);
    if(ptr==NULL) {
        APPLOG("halCallocEx:%d",  n*size);
    }
    if (ptr) {
        memset(ptr, 0x00, n*size);
    }
    return ptr;
}

void *halReallocEx(void *ptr, size_t size, const char *filename, uint32_t line) {
    void *ptr1;
    return ptr1;
}

void halFreeEx(void *ptr, const char *filename, uint32_t line) {
    //APPLOG("halFreeEx:[0x%x][%d][%s]", ptr, line, filename);
    if (ptr)
        free(ptr);
}
#endif

int halReboot() {
    return 0;
}

uint16_t halRand() {
    return 0;
}

void halDelayms(int ms) {
    usleep(ms*1000);
}
