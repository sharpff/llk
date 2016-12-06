#include "leconfig.h"
#include "halHeader.h"

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
