#include "leconfig.h"

uint8_t crc8(const uint8_t *ptr, uint16_t len);

int bytes2hexStr(const uint8_t *src, int srcLen, uint8_t *dst, int dstLen);
int hexStr2bytes(const char *hexStr, uint8_t *buf, int bufLen);


uint16_t genRand();
uint16_t genSeqId();
    
