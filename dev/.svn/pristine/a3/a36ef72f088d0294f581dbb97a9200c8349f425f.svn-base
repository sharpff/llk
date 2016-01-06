#ifndef __AESN_H__
#define __AESN_H__

#include "leconfig.h"

enum {
    ENC_AES_ENCRYPT,
    ENC_AES_DECRYPT,
};

/* aes 128, CBC PKCS#5 */
int aes(uint8_t iv[AES_LEN],
    uint8_t key[AES_LEN],
    uint8_t *data,
    uint32_t *len,
    uint32_t maxLen,
    int type);

#endif