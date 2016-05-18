#ifdef __LE_SDK__
#undef __LE_SDK__
#endif

#include "leconfig.h"
#include "protocol.h"
#include "io.h"
#include "ota.h"

int main(int argc, char *argv[]) {
    int i, ret = 0;

    ret = lelinkStorageInit(0x1C2000, 0x3E000, 0x1000);
    if (0 > ret) {
        APPLOGE("lelinkStorageInit ret[%d]\r\n", ret);
        return -2;
    }
    ret = lelinkInit();
    if (0 > ret) {
        APPLOGE("lelinkInit failed [%d]\r\n", ret);
        return -3;
    }
    void *ctxR2R = (void *)lelinkNwNew(REMOTE_BAK_IP, REMOTE_BAK_PORT, 0, NULL);
    void *ctxQ2A = (void *)lelinkNwNew(NULL, 0, NW_SELF_PORT, NULL);
    while (1) {
        lelinkPollingState(100, ctxR2R, ctxQ2A);
    }
    lelinkNwDelete(ctxR2R);
    lelinkNwDelete(ctxQ2A);
    return 0;
}

