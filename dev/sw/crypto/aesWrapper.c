#include "aesWrapper.h"

// AES pack/unpack
int aes(uint8_t iv[AES_LEN],
    uint8_t key[AES_LEN],
    uint8_t *data,
    uint32_t *len,
    uint32_t maxLen,
    int type) {
    return halAES(key, AES_LEN*8, iv, data, len, maxLen, 1, type);
}
