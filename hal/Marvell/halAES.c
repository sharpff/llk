#include "halHeader.h"
#include <mdev_aes.h>
#if defined(__MRVL_SDK3_3__)
#include "protocol.h"
#else
#include <lelink/sw/protocol.h>
#endif

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
#if 1

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

static mbedtls_aes_context aes_ctx;

/*Initialize the aes driver*/
int lelinkAesInit(void)
{
    int ret = 0;

    mbedtls_aes_init( &aes_ctx );

    return ret;
}

//AES crypt
int halAES(uint8_t *key, uint32_t keyLen, uint8_t *iv, uint8_t *data, uint32_t *len, uint32_t maxLen, int isPKCS5, int type)
{
    int dl;
    int ret = -1;
    
    mbedtls_aes_setkey_enc(&aes_ctx, key, keyLen);

    
    return ret;
}

#endif