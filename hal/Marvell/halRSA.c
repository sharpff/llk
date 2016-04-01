#include "halHeader.h"

#if !defined(MBEDTLS_CONFIG_FILE)
#include <lelink/sw/mbedtls-2.2.0_crypto/include/mbedtls/config.h>
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_PLATFORM_C)
#include <lelink/sw/mbedtls-2.2.0_crypto/include/mbedtls/platform.h>
#include <lelink/sw/mbedtls-2.2.0_crypto/include/mbedtls/ctr_drbg.h>
#include <lelink/sw/mbedtls-2.2.0_crypto/include/mbedtls/pk.h>
#include <lelink/sw/mbedtls-2.2.0_crypto/include/mbedtls/entropy.h>
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

// mbedtls 2.2
#if 1

static mbedtls_pk_context ginPK;
static mbedtls_entropy_context ginEntropy;
static mbedtls_ctr_drbg_context ginCtrDrbg;
int halRsaInit() {

    mbedtls_ctr_drbg_init(&ginCtrDrbg);
    mbedtls_entropy_init(&ginEntropy);
    mbedtls_pk_init( &ginPK );
    return 0;
}

int halRsaExit() {

    mbedtls_ctr_drbg_free(&ginCtrDrbg);
    mbedtls_entropy_free(&ginEntropy);
    mbedtls_pk_free(&ginPK);
    return 0;
}

int halRsaEncrypt(const uint8_t *pubkey, int pubkeyLen, const uint8_t* input, int inputLen, uint8_t *out, int outLen) {

    int ret;
    size_t olen = 0;
    const char *pers = "mbedtls_pk_encrypt";

    if(0 != (ret = mbedtls_ctr_drbg_seed(&ginCtrDrbg, mbedtls_entropy_func, &ginEntropy,
                               (const uint8_t *) pers,
                               strlen(pers)))) {
        APPLOGE( " mbedtls_ctr_drbg_seed returned -0x%04x\n", -ret );
        return ret;
    }

    if( ( ret = mbedtls_pk_parse_public_key( &ginPK, pubkey, pubkeyLen ) ) != 0 ) {
        APPLOGE( " mbedtls_pk_parse_public_key!! returned -0x%04x\n", -ret );
        return ret;
    }

    if( ( ret = mbedtls_pk_encrypt( &ginPK, input, inputLen,
                            out, &olen, outLen,
                            mbedtls_ctr_drbg_random, &ginCtrDrbg ) ) != 0 ) {
        APPLOGE( " mbedtls_pk_encrypt returned -0x%04x\n", -ret );
        return ret;
    }
    
    return olen;
}

int halRsaDecrypt(const uint8_t *prikey, int prikeyLen, const uint8_t* input, int inputLen, uint8_t *out, int outLen) {
    int ret;
    size_t olen = 0;
    const char *pers = "mbedtls_pk_decrypt";

    if(0 != (ret = mbedtls_ctr_drbg_seed(&ginCtrDrbg, mbedtls_entropy_func, &ginEntropy,
                               (const uint8_t *) pers,
                               strlen( pers )))) {
        APPLOGE( " mbedtls_ctr_drbg_seed returned -0x%04x\n", -ret );
        return ret;
    }

    if(0 != (ret = mbedtls_pk_parse_key( &ginPK, prikey, prikeyLen, NULL, 0))) {
        APPLOGE( " mbedtls_pk_parse_key returned -0x%04x\n", -ret );
        return ret;
    }

    if (0 != (ret = mbedtls_pk_decrypt(&ginPK, input, inputLen, out, &olen, outLen,
                            mbedtls_ctr_drbg_random, &ginCtrDrbg))) {
        APPLOGE( " mbedtls_pk_decrypt returned -0x%04x\n", -ret );
        return ret;
    }

    return olen;
}

int halRsaVerify(const uint8_t* pubkey, int pubkeyLen, 
    const uint8_t *raw, int rawLen, const uint8_t *sig, int sigLen)
{
    int ret;
    unsigned char hash[32] = {0};

    if( ( ret = mbedtls_pk_parse_public_key( &ginPK, pubkey, pubkeyLen ) ) != 0 ) {
        APPLOGE( " mbedtls_pk_parse_public_key!! returned -0x%04x\n", -ret );
        return ret;
    }

    if (0 != (ret = mbedtls_md( 
                    mbedtls_md_info_from_type( MBEDTLS_MD_MD5 ),
                    raw, rawLen, hash ))) {
        APPLOGE( " mbedtls_md returned -0x%04x\n", -ret );
        return ret;
    }

    if (0 != (ret = mbedtls_pk_verify( &ginPK, MBEDTLS_MD_MD5, hash, 0,
                           sig, sigLen)))
    {
        APPLOGE( " mbedtls_pk_verify returned -0x%04x\n", -ret );
        return ret;
    }

    return 0;
}

// Marvell native
#else

#include <wm-tls.h>
#include <rsa.h>

int halRsaInit() {
    ctaocrypt_lib_init();
    return 0;
}

int halRsaExit() {

    return 0;
}

int halRsaEncrypt(const uint8_t *pubkey, int pubkeyLen, const uint8_t* input, int inputLen, uint8_t *out, int outLen) {

    RsaKey key;
    word32 idx = 0, ret;
    RNG    rng;

    APPLOG("rsaDecrypt start");

    InitRsaKey(&key, 0);
    
    ret = RsaPublicKeyDecode(pubkey, &idx, &key, pubkeyLen);
    if (ret != 0) {
        APPLOGE("rsaDecrypt RsaPublicKeyDecode %d", ret);
        return -1;
    }
    
    ret = InitRng(&rng);
    if (ret != 0) {
        APPLOGE("RSA Random number generator error: %d", ret);
        return -2;
    }
    ret = RsaPublicEncrypt(input, inputLen, out, outLen, &key, &rng);
    if (0 > ret)
    {
        APPLOGE("rsaDecrypt RsaPublicDecrypt failed[%d]", ret);
        return -3;
    }
    FreeRsaKey(&key);

    return ret;

}

int halRsaDecrypt(const uint8_t *prikey, int prikeyLen, const uint8_t* input, int inputLen, uint8_t *out, int outLen) {
    int ret = 0;
    RsaKey key;
    word32 idx = 0;
    
    APPLOG("rsaDecrypt start");

    InitRsaKey(&key, 0);
    
    ret = RsaPrivateKeyDecode(prikey, &idx, &key, prikeyLen);
    if (ret != 0) {
        APPLOGE("rsaDecrypt RsaPrivateKeyDecode %d", ret);
        return -1;
    }
    
    ret = RsaPrivateDecrypt(input, inputLen, out, outLen, &key);
    if (0 > ret)
    {
        APPLOGE("rsaDecrypt RsaPrivateDecrypt failed[%d]", ret);
        return -2;
    }
    FreeRsaKey(&key);

    return ret;
}

int halRsaVerify(const uint8_t* pubkey, int pubkeyLen, 
    const uint8_t *raw, int rawLen, const uint8_t *sig, int sigLen)
{

    RsaKey key;
    int ret = 0;
    word32 idx = 0;
    const int size = 2048 > rawLen ? rawLen : 2048;
    static uint8_t hashDecrypt[2048] = { 0 };
    
    memset(hashDecrypt, 0, sizeof(hashDecrypt));
    InitRsaKey(&key, 0);
    APPLOG("rsaVerify start");
    //hex2bin(pubkey, pubkey, sizeof(pubkey));
    ret = RsaPublicKeyDecode(pubkey, &idx, &key, pubkeyLen);
    if (ret < 0) {
        APPLOGE("RsaPublicKeyDecode : %d", ret);
        return -1;
    }
    ret = RsaSSL_Verify(sig, sigLen, hashDecrypt, size, &key);
    
    if (memcmp(raw, hashDecrypt, size) || ret < 0) {
        FreeRsaKey(&key);
        APPLOGE("RSA Verify error : %d", ret);
        return -2;
    }

    FreeRsaKey(&key);
    APPLOG("rsaVerify OK");

    return ret;
}

#endif