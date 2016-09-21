#include "halHeader.h"
#include "protocol.h"

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#define mbedtls_printf     printf
#endif

//#if defined(MBEDTLS_BIGNUM_C) && defined(MBEDTLS_PK_PARSE_C) && \
    //defined(MBEDTLS_FS_IO) && defined(MBEDTLS_ENTROPY_C) && \
    //defined(MBEDTLS_CTR_DRBG_C)
#include "mbedtls/error.h"
#include "mbedtls/pk.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"

#include <stdio.h>
#include <string.h>
//#endif



static mbedtls_aes_context ginAESCtx;
int halAESInit(void) {
    mbedtls_aes_init( &ginAESCtx );
    return 0;
}

void halDeAESInit(void) {
    mbedtls_aes_free( &ginAESCtx );
}

//AES crypt
int halAES(uint8_t *key, uint32_t keyLen, uint8_t *iv, uint8_t *data, uint32_t *len, uint32_t maxLen, int isPKCS5, int type) {
    int ret = 0;
    uint8_t out[MAX_BUF] = {0};
    if (!type) {
        APPLOG("dec *len is [%d] ret[%d] maxLen[%d] START", *len, ret, maxLen);
        mbedtls_aes_setkey_dec(&ginAESCtx, key, keyLen);
        ret = mbedtls_aes_crypt_cbc(&ginAESCtx, MBEDTLS_AES_DECRYPT, *len, iv, data, out);
        // APPPRINTF("dec *len is [%d]\r\n", *len);
        // for (i = 0; i < *len; i++) {
        //     APPPRINTF("%02x ", out[i]);
        //     if (0 == (i + 1)%AES_LEN) {
        //         APPPRINTF("\r\n");
        //     }
        // }
        // APPPRINTF("\r\n");
        if (0 == ret) {
            *len -= out[*len - 1];
            memcpy(data, out, *len > maxLen ? maxLen : *len);
        }
        APPLOG("dec *len is [%d] ret[%d] END", *len, ret);

    } else {
        int blocks = 0, padSize = 0, i = 0;
        APPLOG("enc *len is [%d] ret[%d] START", *len, ret);
        blocks = (*len/AES_LEN) + 1;
        padSize = AES_LEN - *len%AES_LEN;
        for (i = *len; i < *len + padSize; i++) {
            data[i] = padSize;
        }
        // APPPRINTF("*len is [%d]\r\n", *len);
        *len = AES_LEN*blocks;
        // for (i = 0; i < *len; i++) {
        //     APPPRINTF("%02x ", data[i]);
        //     if (0 == (i + 1)%AES_LEN) {
        //         APPPRINTF("\r\n");
        //     }
        // }
        // APPPRINTF("\r\n");
        mbedtls_aes_setkey_enc(&ginAESCtx, key, keyLen);
        ret = mbedtls_aes_crypt_cbc(&ginAESCtx, MBEDTLS_AES_ENCRYPT, *len, iv, data, out);
        if (0 == ret) {
            memcpy(data, out, *len);
        }
        APPLOG("enc *len is [%d] ret[%d] END", *len, ret);
    }
    

    return ret;
}
