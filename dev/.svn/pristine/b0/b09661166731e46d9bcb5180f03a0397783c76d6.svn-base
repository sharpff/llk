#ifdef __LE_SDK__
#undef __LE_SDK__
#endif
#include "leconfig.h"
// #include "halHeader.h"
static pthread_mutex_t ginMutex = PTHREAD_MUTEX_INITIALIZER;

int halLockInit(void *ptr, const char *file, int line) {
	return 0;
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
