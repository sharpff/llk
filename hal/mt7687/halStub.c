#include "leconfig.h"


/*
 * These functions is called in hal of Lelink, but depends the current App(FW). 
 * Diff App has diff implementation. So, these should not be defined in halXXX.c, but in halStub.c.
 */
__attribute__((weak)) int haalIsRepeater(void) {
    return 0;
}

__attribute__((weak)) void haalCoOTASetFlag(uint32_t flag) {
    return;
}

__attribute__((weak)) void haalCoOTAProcessing(void) {
    return;
}