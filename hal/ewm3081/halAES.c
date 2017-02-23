#include "halHeader.h"
#include "protocol.h"
#include <stdio.h>
#include <string.h>

uint8_t g_data_buffer[MAX_BUF] = {0};

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

#include "mbedtls/error.h"
#include "mbedtls/pk.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"

#include <stdio.h>
#include <string.h>

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
    int ret = 0, i;
    memset(g_data_buffer, 0, MAX_BUF);
    if (!type) {
        //APPLOG("dec *len is [%d] ret[%d] START", *len, ret);
        mbedtls_aes_setkey_dec(&ginAESCtx, key, keyLen);
        ret = mbedtls_aes_crypt_cbc(&ginAESCtx, MBEDTLS_AES_DECRYPT, *len, iv, data, g_data_buffer);
        //for (i = 0; i < *len; i++) {
        //     APPPRINTF("%02x ", g_data_buffer[i]);
        //     if (0 == (i + 1)%AES_LEN) {
        //         APPPRINTF("\r\n");
        //     }
        //}
        //APPPRINTF("\r\n");
        if (0 == ret) {
            *len -= g_data_buffer[*len - 1];
            //APPLOG("dec *len is [%d]", *len);
            if(*len <= MAX_BUF) {
                memcpy(data, g_data_buffer, *len);
            } else {
                ret = -1;
                APPLOG("dec error");
            }
        }
        //APPLOG("dec *len is [%d] ret[%d] END", *len, ret);

    } else {
        int blocks = 0, padSize = 0, i = 0;
        //APPLOG("enc *len is [%d] ret[%d] START", *len, ret);
        blocks = (*len/AES_LEN) + 1;
        padSize = AES_LEN - *len%AES_LEN;
        for (i = *len; i < *len + padSize; i++) {
            data[i] = padSize;
        }
        // APPPRINTF("*len is [%d]\r\n", *len);
        *len = AES_LEN*blocks;
        //for (i = 0; i < *len; i++) {
        //     APPPRINTF("%02x ", data[i]);
        //     if (0 == (i + 1)%AES_LEN) {
        //         APPPRINTF("\r\n");
        //     }
        //}
        //APPPRINTF("\r\n");
        mbedtls_aes_setkey_enc(&ginAESCtx, key, keyLen);
        ret = mbedtls_aes_crypt_cbc(&ginAESCtx, MBEDTLS_AES_ENCRYPT, *len, iv, data, g_data_buffer);
        if (0 == ret) {
            memcpy(data, g_data_buffer, *len);
        }
        //APPLOG("enc *len is [%d] ret[%d] END", *len, ret);
    }
    return ret;
}
