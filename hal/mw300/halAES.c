#include "halHeader.h"
#include <mdev_aes.h>
#if defined(__MRVL_SDK3_3__)
#include "protocol.h"
#else
#include <lelink/sw/protocol.h>
#endif
#if 0

static int lelinkPadding(uint8_t *data, uint32_t len, uint32_t maxLen, int isPKCS5)
{
    if (isPKCS5)
    {
        int i;
        uint8_t padding = 16;
        
        if ( len % 16 )
            padding = 16 - ( len % 16 );
        
        if ( ( len + padding ) > maxLen )
            return LELINK_ERR_ENCINFO_ERR;
        
        for ( i=0; i<padding; i++ )
            data[len + i] = padding;
        return len + padding;
    }
    else
    {
        len = (len + 15) & 0xfffffff0;
        if (len > maxLen)
            return LELINK_ERR_ENCINFO_ERR;
        return len;
    }
}

static int lelinkUnPadding(uint8_t *data, uint32_t len, int isPKCS5)
{
    if ( len % 16 )
        return LELINK_ERR_ENCINFO_ERR;
    
    if (isPKCS5)
    {
        int i;
        uint8_t padding;
        
        padding = data[len - 1];
        for (i=0; i<padding; i++)
        {
            if (data[len - 1 - i] != padding)
                return LELINK_ERR_RECV_DATA_ERR;
        }
        
        return len - padding;
    }
    else
    {
        return len;
    }
}



static int __lelinkAesEncryt(uint8_t *input, uint8_t *output, const uint8_t *key, const uint8_t *iv, int len)
{
    int ret = 0;
    aes_t enc_aes;
    mdev_t *aes_dev = aes_drv_open(MDEV_AES_0);

    aes_drv_setkey(aes_dev, &enc_aes, key, 16,
        iv, AES_ENCRYPTION, HW_AES_MODE_CBC);
    ret = aes_drv_encrypt(aes_dev, &enc_aes, input, output, len);
    aes_drv_close(aes_dev);
    return ret;
}

/*Raw interface to decryt*/
int __lelinkAesDecryt(uint8_t *input, uint8_t *output, const uint8_t *key, const uint8_t *iv, int len)
{
    int ret = 0;
    aes_t enc_aes;
    mdev_t *aes_dev = aes_drv_open(MDEV_AES_0);


    aes_drv_setkey(aes_dev, &enc_aes, key, 16,
        iv, AES_DECRYPTION, HW_AES_MODE_CBC);
    ret = aes_drv_decrypt(aes_dev, &enc_aes, input, output, len);
    aes_drv_close(aes_dev);
    return ret;

}

/*Encryt the input data according the key*/
int lelinkAesEncryt(uint8_t *input, int input_len, const uint8_t *key, 
    const uint8_t *iv, uint8_t *output)
{
    int ret = 0, len;
    //int i;
    len = (input_len + 15)&0xfff0;

/*    wmprintf("En before:");
    for(i = 0; i < len; i++)
        wmprintf("0x%02x, ", input[i]);
    wmprintf("\r\n");
*/
    ret = __lelinkAesEncryt(input, output, key, iv, len);

  /*  wmprintf("En after:");
    for(i = 0; i < len; i++)
        wmprintf("0x%02x, ", output[i]);
    wmprintf("\r\n");
*/
    if(ret == 0)
        ret = len;
  
    return  ret;
    
}   

/*decryt the input data according the key*/
int lelinkAesDecryt(uint8_t *input, int input_len, const uint8_t *key, 
    const uint8_t *iv, uint8_t *output)
{
    //int i;
    int ret = 0;
    /*input len check*/
    if((input_len&0xf) != 0)
        return -1; //Len error.

    /*wmprintf("de before:");
    for(i = 0; i < input_len; i++)
        wmprintf("0x%02x, ", input[i]);
    wmprintf("\r\n");*/

    ret =  __lelinkAesDecryt(input, output, key, iv, input_len);

    /*wmprintf("de after:");
    for(i = 0; i < input_len; i++)
        wmprintf("0x%02x, ", output[i]);
    wmprintf("\r\n");*/

    return ret;
}

/*Initialize the aes driver*/
int halAESInit(void)
{
    int ret = 0;

    ret =  aes_drv_init();

    return ret;
}

//AES crypt
int halAES(uint8_t *key, uint32_t keyLen, uint8_t *iv, uint8_t *data, uint32_t *len, uint32_t maxLen, int isPKCS5, int type)
{
    int dl;
    int ret = -1;
    
    if (!type)
    {
        ret = lelinkAesDecryt(data, *len, key, iv, data);
        dl = lelinkUnPadding(data, (uint32_t)(*len), isPKCS5);
        if (dl < 0)
            return dl;
    }
    else
    {
        dl = lelinkPadding(data, *len, maxLen, isPKCS5);
        if (dl < 0)
            return dl;
        ret = lelinkAesEncryt(data, dl, key, iv, data);
    }
    
    *len = dl;
    
    return ret;
}

#else

#if !defined(MBEDTLS_CONFIG_FILE)
#if defined(__MRVL_SDK3_3__)
#include "mbedtls-2.2.0_crypto/include/mbedtls/config.h"
#else
#include <lelink/sw/mbedtls-2.2.0_crypto/include/mbedtls/config.h>
#endif
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_PLATFORM_C)
#if defined(__MRVL_SDK3_3__)
#include "mbedtls-2.2.0_crypto/include/mbedtls/platform.h"
#include "mbedtls-2.2.0_crypto/include/mbedtls/ctr_drbg.h"
#include "mbedtls-2.2.0_crypto/include/mbedtls/pk.h"
#include "mbedtls-2.2.0_crypto/include/mbedtls/entropy.h"
#else
#include <lelink/sw/mbedtls-2.2.0_crypto/include/mbedtls/platform.h>
#include <lelink/sw/mbedtls-2.2.0_crypto/include/mbedtls/ctr_drbg.h>
#include <lelink/sw/mbedtls-2.2.0_crypto/include/mbedtls/pk.h>
#include <lelink/sw/mbedtls-2.2.0_crypto/include/mbedtls/entropy.h>
#endif
#else
#include <stdio.h>
#define mbedtls_printf     printf
#endif

#if defined(MBEDTLS_BIGNUM_C) && defined(MBEDTLS_PK_PARSE_C) && \
    defined(MBEDTLS_FS_IO) && defined(MBEDTLS_ENTROPY_C) && \
    defined(MBEDTLS_CTR_DRBG_C)
#include "mbedtls/error.h"
#include "mbedtls/pk.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"

#include <stdio.h>
#include <string.h>
#endif

static mbedtls_aes_context ginAESCtx;
int halAESInit(void) {
    mbedtls_aes_init( &ginAESCtx );
    return 0;
}

void halDeAESInit(void) {
    mbedtls_aes_free( &ginAESCtx );
}

//AES crypt
int halAES(uint8_t *key, uint32_t keyLen, uint8_t *iv, uint8_t *data, uint32_t *len, uint32_t maxLen, int isPKCS5, int type)
{
    int ret = 0;
    uint8_t out[MAX_BUF] = {0};
    if (!type) {
        APPLOG("dec *len is [%d] ret[%d] START", *len, ret);
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

#endif