#include "halHeader.h"
#include "mbedtls/sha1.h"

static mbedtls_sha1_context ginSha1Ctx;

int halSha1Start() {
    mbedtls_sha1_init( &ginSha1Ctx );
    mbedtls_sha1_starts( &ginSha1Ctx );
    return 0;
}

int halSha1Update(const uint8_t *input, size_t ilen) {
    mbedtls_sha1_update( &ginSha1Ctx, input, ilen );
    return 0;
}

int halSha1End(uint8_t output[20]) {
    mbedtls_sha1_finish( &ginSha1Ctx, output );
    mbedtls_sha1_free( &ginSha1Ctx );
    return 0;
}