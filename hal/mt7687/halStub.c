#include "leconfig.h"


/*
 * These functions is called in hal of Lelink, but depends the current App(FW). 
 * Diff App has diff implementation. So, these should not be defined in halXXX.c, but in halStub.c.
 */
LELINK_WEAK int haalIsRepeater(void) {
    return 0;
}

LELINK_WEAK void haalCoOTASetFlag(uint32_t flag) {
    return;
}

LELINK_WEAK void haalCoOTAProcessing(void) {
    return;
}