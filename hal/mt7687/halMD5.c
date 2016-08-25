#include "halHeader.h"

#include "hal_md5.h"
#include "hal_log.h"
#include "os.h"
#include "hal_sha.h"
#include "mbedtls/sha1.h"

#define HW_SHA1

void md5_result_dump(uint8_t *result, uint8_t length) {
    uint8_t i;

    for (i = 0; i < length; i++) {
        if (i % 16 == 0) {
            log_hal_info("\r\n");
        }

        log_hal_info(" %02x ", result[i]);
    }
    log_hal_info("\r\n");

}

void halMD5(uint8_t *input, uint32_t inputlen, uint8_t output[16]) {
    uint8_t digest[HAL_MD5_DIGEST_SIZE] = {0};

    hal_md5_context_t context = {{0}};
    hal_md5_init(&context);
    hal_md5_append(&context, input, inputlen);
    hal_md5_end(&context, digest);
	os_memcpy(output, digest, HAL_MD5_DIGEST_SIZE);
}

#ifdef HW_SHA1

static hal_sha1_context_t ginSha1Ctx;

int halSha1Start() {
    hal_sha1_init( &ginSha1Ctx );
    return 0;
}

int halSha1Update(const uint8_t *input, size_t ilen) {
    hal_sha1_append( &ginSha1Ctx, input, ilen );
    return 0;
}

int halSha1End(uint8_t output[20]) {
    hal_sha1_end( &ginSha1Ctx, output );
    return 0;
}

#else

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

#endif
