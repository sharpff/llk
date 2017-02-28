// #include "halHeader.h"
// #include "leconfig.h"
//static pthread_mutex_t ginMutex = PTHREAD_MUTEX_INITIALIZER;
#include "debug.h"
#include "MICORTOS.h"
#include "MICOPLATFORM.h"
#include "mico.h"


static mico_mutex_t os_mutex;
int halLockInit(void) {
    mico_rtos_init_mutex(&os_mutex);
    return 0;
}

void halDeLockInit(void) {
    
}

int halLock(void) {
    mico_rtos_lock_mutex(&os_mutex);
    return 0;
}

int halUnlock(void) {
    mico_rtos_unlock_mutex(&os_mutex);
    return 0;
}

/**
 * ?????ͳ????????
 */
unsigned int halGetTimeStamp(void) {
    return mico_get_time()/1000;
}

unsigned int halGetUTC(void) {
    //problem
    return 1234;
}
////////////////////////////////////////////////////////////////////////////////
#if 0
void *halCalloc(int n, size_t size) {
    void *ptr = calloc(n,size);
    if(NULL==ptr){
        APPLOG("halCalloc(%d,%d) = %p",n,size,ptr);
    }
    return ptr;
}

void *halMalloc(size_t size) {
    void* ret = malloc(size);
    if(NULL==ret){
        APPLOG("halMalloc(%d) = %p",size,ret);
    }
    return ret;
}

void *halRealloc(void *ptr, size_t size) {
    void* ret = NULL;
    ret = realloc(ptr,size);
    if(NULL==ret){
        APPLOG("halRealloc(%p,%d) = %p",ptr,size,ret);
    }
    return ret;
}
#endif
void *halMallocEx(size_t size, const char *filename, uint32_t line) {
    void *ptr = malloc(size);
    //APPLOG("malloc:[%d][0x%x][%d][%s]", size, ptr, line, filename);
    if(ptr==NULL) {
        // APPLOG("halMalloc(%d) = %p",size,ptr);
    }
    return ptr;
}

void *halCallocEx(size_t n, size_t size, const char *filename, uint32_t line) {
    void *ptr = calloc(n, size);
    //APPLOG("calloc:[%d][%d][0x%x][%d][%s]", n*size,xPortGetFreeHeapSize(), ptr, line, filename);
    if(ptr==NULL) {
        // APPLOG("halCallocEx:%d, %d",  n*size, xPortGetFreeHeapSize());
    }
    return ptr;
}

void *halReallocEx(void *ptr, size_t size, const char *filename, uint32_t line) {
    void *ptr1 = realloc(ptr, size);
    //APPLOG("realloc:[%d][0x%x][%d][%s]", size, ptr1, line, filename);
    if (ptr1==NULL) {
        // APPLOG("halRealloc(%p,%d) = %p",ptr,size,ptr1);
    }
    return ptr1;
}

void halFreeEx(void *ptr, const char *filename, uint32_t line) {
    //APPLOG("halFreeEx:[0x%x][%d][%s]", ptr, line, filename);
    if (ptr)
        free(ptr);
}
////////////////////////////////////////////////////////////////////////////////
int halReboot() {
    // APPLOG("halReboot");
    MicoSystemReboot();
    return 0;
}

uint16_t halRand() {
    u16 ret;
    MicoRandomNumberRead(&ret, 2);
    return ret;
}

void halDelayms(int ms) {
    mico_thread_msleep(ms);
}
