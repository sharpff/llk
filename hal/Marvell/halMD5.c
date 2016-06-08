#include "halHeader.h"
#include "md5.h"
#if defined(__MRVL_SDK3_3__)
#include "mbedtls-2.2.0_crypto/include/mbedtls/md.h"
#else
#include <lelink/sw/mbedtls-2.2.0_crypto/include/mbedtls/md.h>
#endif

void halMD5(unsigned char *input, unsigned int inputlen, unsigned char output[16])
{
#if 1
    Md5 md5;

    InitMd5(&md5);
    Md5Update(&md5, (uint8_t *)input, inputlen);
    Md5Final(&md5, output);
#else
    // mbedtls_md( mbedtls_md_info_from_type( MBEDTLS_MD_MD5 ), 
    //     input, inputlen, 
    //     output );
#endif
}

