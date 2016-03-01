#include "leconfig.h"
#include "ota.h"
int halHttpOpen(OTAInfo_t *info, const char *url) {
    return 0;
}

void halHttpClose(OTAInfo_t *info) {
}


int halUpdateFirmware(OTAInfo_t *info) {
    return 0;
}


int halUpdateScript(OTAInfo_t *info, ScriptCfg *scriptCfg) {
    return 0;
}