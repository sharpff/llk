#ifndef __RSAN_H__
#define __RSAN_H__

#include "leconfig.h"


int rsaEncrypt(const uint8_t *pubkey, int pubkeyLen, const uint8_t* input, int inputLen, uint8_t *out, int outLen);
int rsaDecrypt(const uint8_t *prikey, int prikeyLen, const uint8_t* input, int inputLen, uint8_t *out, int outLen);
int rsaVerify(const uint8_t* pubkey, int pubkeyLen, const uint8_t *raw, int rawLen, const uint8_t *sig, int sigLen);

#endif