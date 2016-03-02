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
    return os_get_timestamp() / 1000000UL;
}

unsigned int halGetUTC(void) {
    return 1234;
}

int halReboot(void) {
    app_reboot(REASON_USER_REBOOT);
}