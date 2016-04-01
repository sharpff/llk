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


uint32_t halUpdate(OTAInfo_t *info, uint8_t *buf, uint32_t bufLen) {
    return 0;
}