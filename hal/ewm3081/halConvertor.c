#include "halHeader.h"
#include <stdint.h>
int halStd2Pri(const char *protocol, int protocolLen, uint8_t *io, int ioLen, int *type, void *userData) {
    return 0;
}

int halPri2Std(const uint8_t *io, int ioLen, char *protocol, int protocolLen, int type, void *userData) {
    return 0;
}

