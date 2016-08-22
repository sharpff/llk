#include "halHeader.h"
#include "protocol.h"
#include <stdio.h>
#include <string.h>
#include "hal_aes.h"

//define HW_AES
#ifdef HW_AES

int halAESInit(void) {
    return 0;
}

void halDeAESInit(void) {

}

uint8_t data_buffer[1280] = {0};

int halAES(uint8_t *aes_key, uint32_t keyLen, uint8_t *iv, uint8_t *data, uint32_t *len, uint32_t maxLen, int isPKCS5, int type)
{
    int ret = 0;
    
    hal_aes_buffer_t key = {
        .buffer = aes_key,
        .length = keyLen/8
    };

    if (!type)
    {
        //decrypt case;
        printf("\n==== decrypt len[%d] key[%d] ==== \n", keyLen/8, *len);
        hal_aes_buffer_t encrypted_text = {
            .buffer = data,
            .length = *len
        };
        
        hal_aes_buffer_t decrypted_text = {
            .buffer = data_buffer,
            .length = *len
        };
 #if 0
        {
            int i = 0;
            printf(" \n =====> halAES decrypt data begin <===== \n");
            for (i = 0; i < *len; i++) {
                printf("%02X", data[i]);
            }
            printf(" \n =====> halAES decrypt data  <===== \n");
        }
#endif
        ret = hal_aes_cbc_decrypt(&decrypted_text, &encrypted_text, &key, iv);
        printf(" \n =====> hal_aes_cbc_decrypt  %d <===== \n",decrypted_text.length);
        if(ret != HAL_AES_STATUS_OK)
        {
            printf(" \n =====> halAES decrypt data error <===== \n");
            goto end;
        }
#if 0
        {
            int i = 0;
            printf(" \n =====> halAES decrypt data  %d <===== \n",decrypted_text.length);
            for (i = 0; i < decrypted_text.length; i++) {
                printf("%02X", decrypted_text.buffer[i]);
            }
            printf(" \n =====> halAES decrypt data end <===== \n");
        }
#endif
        os_memset(data, 0x00, (*len));
        *len = decrypted_text.length;
        os_memcpy(data, decrypted_text.buffer, decrypted_text.length);
    }
    else
    {
        
        uint8_t padding_size = 16 - *len%16;
        uint32_t encrypted_len = *len+padding_size;
        printf("\n==== encrypted begin len[%d] encrypted_len[%d]==== \n", *len, encrypted_len);
        hal_aes_buffer_t plain_text = {
            .buffer = data,
            .length = *len
        };

        hal_aes_buffer_t encrypted_text = {
            .buffer = data_buffer,
            .length = encrypted_len
        };
#if 0
        {
            int i = 0;
            printf(" \n =====> halAES encrypted data begin <===== \n");
            for (i = 0; i < *len; i++) {
                printf("%02X", data[i]);
            }
            printf(" \n =====> halAES encrypted data  <===== \n");
        }
#endif
        ret = hal_aes_cbc_encrypt(&encrypted_text, &plain_text, &key, iv);
        printf(" \n =====> halAES encrypted data <===== %d \n", encrypted_text.length);
        if(ret != HAL_AES_STATUS_OK)
        {
            printf(" \n =====> halAES encrypt data error <===== \n");
            goto end;
        }
#if 0
        {
            int i = 0;
            printf(" \n =====> halAES encrypted data <===== %d \n", encrypted_text.length);
            for (i = 0; i < encrypted_text.length; i++) {
                printf("%02X", encrypted_text.buffer[i]);
            }
            printf(" \n =====> halAES encrypted data  end<===== \n");
        }
#endif
        os_memset(data, 0x00, encrypted_text.length);
        *len = encrypted_text.length;
        os_memcpy(data, encrypted_text.buffer, encrypted_text.length);
        //printf("\n==== encrypted end len[%d]==== \n", *len);
    }
end:
    return ret;
}
#else
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
    int ret = 0;
    uint8_t out[MAX_BUF] = {0};
    if (!type) {
        //APPLOG("dec *len is [%d] ret[%d] START", *len, ret);
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
            memcpy(data, out, *len);
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
        //APPLOG("enc *len is [%d] ret[%d] END", *len, ret);
    }
    

    return ret;
}
#endif
