#include "halHeader.h"

#include "hal_md5.h"
#include "hal_log.h"
#include "os.h"
#include "mbedtls/sha1.h"

//MTK MT7687 platform;
/* Display the data in the format of 16 bytes each line */
void md5_result_dump(uint8_t *result, uint8_t length)
{
    uint8_t i;

    for (i = 0; i < length; i++) {
        if (i % 16 == 0) {
            log_hal_info("\r\n");
        }

        log_hal_info(" %02x ", result[i]);
    }
    log_hal_info("\r\n");

}

void halMD5(uint8_t *input, uint32_t inputlen, uint8_t output[16]) 
{
    //uint8_t *data = (uint8_t *)"abcdefghijklmnopqrstwxyz1234567";
    //uint32_t size = (uint32_t)strlen((char *)data);
    uint8_t digest[HAL_MD5_DIGEST_SIZE] = {0};

    log_hal_info("Origin input data: %s\r\n", input);

    hal_md5_context_t context = {{0}};
    hal_md5_init(&context);
    hal_md5_append(&context, input, inputlen);
    hal_md5_end(&context, digest);

    log_hal_info("MD5 result:");
    md5_result_dump(digest, sizeof(digest));

	os_memcpy(output, digest, HAL_MD5_DIGEST_SIZE);
}

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
