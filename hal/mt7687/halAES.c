#include "halHeader.h"
// #include "protocol.h"
#include "hal_aes.h"
#include "hal_log.h"
#include "os.h"

extern void* pvPortMalloc(size_t xWanteSize);

int halAESInit(void) {
    //not need to do something;
    return 0;//always return success;
}

void halDeAESInit(void) {
    //not need to do something;
    return;
}

/* Display the data in the format of 16 bytes per line */
void aes_result_dump(uint8_t *result, uint8_t length)
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

//AES crypt
int halAES(uint8_t *aes_key, uint32_t keyLen, uint8_t *iv, uint8_t *data, uint32_t *len, uint32_t maxLen, int isPKCS5, int type)
{
    int ret;
	
	/*uint8_t hardware_id[16] = {
		0x4d, 0x54, 0x4b, 0x30, 0x30, 0x30, 0x30, 0x30,
		0x32, 0x30, 0x31, 0x34, 0x30, 0x38, 0x31, 0x35
	};
	uint8_t aes_cbc_iv[HAL_AES_CBC_IV_LENGTH] = {
		0x61, 0x33, 0x46, 0x68, 0x55, 0x38, 0x31, 0x43,
		0x77, 0x68, 0x36, 0x33, 0x50, 0x76, 0x33, 0x46
	};
	uint8_t plain[] = {
		0, 11, 22, 33, 44, 55, 66, 77, 88, 99, 0, 11, 22, 33, 44, 55,
		66, 77, 88, 99, 0, 11, 22, 33, 44, 55, 66, 77, 88, 99
	};*/

	//uint8_t encrypted_buffer[32] = {0};
	//uint8_t decrypted_buffer[32] = {0};
		
	hal_aes_buffer_t key = {
		.buffer = aes_key,
		.length = keyLen
	};

	log_hal_info("aes_cbc_iv: iv= ");
	aes_result_dump(iv, HAL_AES_CBC_IV_LENGTH);
	log_hal_info("Key:");
	aes_result_dump(key.buffer, key.length);

    if (!type)
    {
        //decrypt case;
        uint8_t * decrypted_buffer = (uint8_t *)pvPortMalloc(*len);

        hal_aes_buffer_t encrypted_text = {
		.buffer = data,
		.length = *len
		};
		
		hal_aes_buffer_t decrypted_text = {
			.buffer = decrypted_buffer,
			.length = sizeof(decrypted_buffer)
		};
		ret = hal_aes_cbc_decrypt(&decrypted_text, &encrypted_text, &key, iv);
		
		log_hal_info("Decrypted data(AES CBC):");
		aes_result_dump(decrypted_text.buffer, decrypted_text.length);

		os_memset(data, 0x00, (*len));
		*len = decrypted_text.length;
		os_memcpy(data, decrypted_text.buffer, decrypted_text.length);
    }	
	else
	{
	    //encrypt case;
	    uint8_t* encrypted_buffer = (uint8_t *)pvPortMalloc(*len);
		
	    hal_aes_buffer_t plain_text = {
		.buffer = data,
		.length = *len
		};

		hal_aes_buffer_t encrypted_text = {
		.buffer = encrypted_buffer,
		.length = sizeof(encrypted_buffer)
		};

		log_hal_info("Origin data:");
		aes_result_dump(plain_text.buffer, plain_text.length);
		
		ret = hal_aes_cbc_encrypt(&encrypted_text, &plain_text, &key, iv);

		log_hal_info("Encrypted data(AES CBC):");
		aes_result_dump(encrypted_text.buffer, encrypted_text.length);

		
		os_memset(data, 0x00, (*len));
		*len = encrypted_text.length;
		os_memcpy(data, encrypted_text.buffer, encrypted_text.length);
	}
	
 #if 0
	hal_aes_ecb_encrypt(&encrypted_text, &plain_text, &key);
	log_hal_info("Encrypted data(AES ECB):");
	aes_result_dump(encrypted_text.buffer, encrypted_text.length);

	hal_aes_ecb_decrypt(&decrypted_text, &encrypted_text, &key);
	log_hal_info("Decrypted data(AES ECB):");
	aes_result_dump(decrypted_text.buffer, decrypted_text.length);
 #endif//0

	return ret;
}
