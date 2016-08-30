#include "halHeader.h"
#include "protocol.h"
#include <stdio.h>
#include <string.h>
#include "hal_aes.h"
#include "debug.h"
#include "semphr.h"
#include "task.h"
#include "os.h"

uint8_t g_data_buffer[MAX_BUF] = {0};

#ifdef HW_AES

uint8_t g_aes_key[16] = {0};
uint32_t g_key_len =0;
uint8_t g_iv[16] = {0};
uint8_t g_data[1280] = {0};
uint32_t g_len = 0;
int g_type = 0;

SemaphoreHandle_t g_m_mutex_for_aes = NULL;
xQueueHandle aes_rx_queue = NULL;
TaskHandle_t g_current_task_id = NULL;

#define AES_RX_QUEUE_SIZE        4

int halAESInit(void) {
    return 0;
}

void halDeAESInit(void) {

}

int halAES(uint8_t *aes_key, uint32_t keyLen, uint8_t *iv, uint8_t *data, uint32_t *len, uint32_t maxLen, int isPKCS5, int type) {
    int is_aes = 1;
    xSemaphoreTake(g_m_mutex_for_aes, portMAX_DELAY);
	
	g_current_task_id = xTaskGetCurrentTaskHandle(); 

    os_memcpy(g_aes_key, aes_key, keyLen);
	g_key_len = keyLen;
    os_memcpy(g_iv, iv, 16);
	os_memcpy(g_data, data, *len);
	g_len = *len;
	g_type = type;

    if (xQueueSendToBack(aes_rx_queue, (void *)&is_aes, (TickType_t)5) != pdPASS) {   
        printf("can't add a job to aes rx queue. \n");
        return -1;
    }

	vTaskSuspend(g_current_task_id);

	os_memcpy(data, g_data, g_len);
	*len = g_len;

    os_memset(g_aes_key, 0x00, g_key_len);
	os_memset(g_iv, 0x00, 16);
	os_memset(g_data, 0x00, g_len);
	xSemaphoreGive(g_m_mutex_for_aes);
	return 0;
}

void ads_do_operate(void) {
    int ret = 0;
    hal_aes_buffer_t key = {
        .buffer = g_aes_key,
        .length = g_key_len/8
    };
  
    if (!g_type) {
        hal_aes_buffer_t encrypted_text = {
            .buffer = g_data,
            .length = g_len
        };
        
        hal_aes_buffer_t decrypted_text = {
            .buffer = g_data_buffer,
            .length = sizeof(g_data_buffer)//*len
        };

        ret = hal_aes_cbc_decrypt(&decrypted_text, &encrypted_text, &key, g_iv);

        if(ret != HAL_AES_STATUS_OK) {
            printf(" \n =====> halAES decrypt data error <===== \n");
            goto end;
        }

        if(decrypted_text.length > 0) {
            os_memset(g_data, 0x00, 1280);
            g_len = decrypted_text.length;
            os_memcpy(g_data, decrypted_text.buffer, decrypted_text.length);
        } else {
            printf("decrypted_text.length error. \n");
        }
    } else {
        uint8_t padding_size = 16 - g_len%16;
        uint32_t encrypted_len = g_len+padding_size;
        hal_aes_buffer_t plain_text = {
            .buffer = g_data,
            .length = g_len
        };

        hal_aes_buffer_t encrypted_text = {
            .buffer = g_data_buffer,
            .length = encrypted_len
        };

        ret = hal_aes_cbc_encrypt(&encrypted_text, &plain_text, &key, g_iv);
        if(ret != HAL_AES_STATUS_OK) {
            printf(" \n =====> halAES encrypt data error <===== \n");
            goto end;
        }

        os_memset(g_data, 0x00, encrypted_text.length);
        g_len = encrypted_text.length;
        os_memcpy(g_data, encrypted_text.buffer, encrypted_text.length);
    }

end:
    if(g_current_task_id != NULL) {
        vTaskResume(g_current_task_id);
    }
}

void aes_handle_task_func(void *args) {
    int is_aes_handle;
	while (1) {
		if (xQueueReceive(aes_rx_queue, (void *)&is_aes_handle, portMAX_DELAY) == pdPASS) {
			if(is_aes_handle == 1) {
				ads_do_operate();
			}
		}
	}
}

void aes_task_init(void) {
    if (aes_rx_queue == NULL) {
            aes_rx_queue = xQueueCreate(AES_RX_QUEUE_SIZE, sizeof(int));
            if (aes_rx_queue == NULL) 
            {
                printf("aes_rx_queue create failed. \n");
                return;
            }
            configASSERT(aes_rx_queue);
            vQueueAddToRegistry(aes_rx_queue, "aes handle");
    }
    
    if (pdPASS != xTaskCreate(aes_handle_task_func,"aes_test",1024,NULL,1,NULL)) {
        LOG_E(common, "create user task fail");
        return;
    }
    g_m_mutex_for_aes = xSemaphoreCreateMutex(); 
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
    os_memset(g_data_buffer, 0, MAX_BUF);
    if (!type) {
        //APPLOG("dec *len is [%d] ret[%d] START", *len, ret);
        mbedtls_aes_setkey_dec(&ginAESCtx, key, keyLen);
        ret = mbedtls_aes_crypt_cbc(&ginAESCtx, MBEDTLS_AES_DECRYPT, *len, iv, data, g_data_buffer);
        // for (i = 0; i < *len; i++) {
        //     APPPRINTF("%02x ", out[i]);
        //     if (0 == (i + 1)%AES_LEN) {
        //         APPPRINTF("\r\n");
        //     }
        // }
        // APPPRINTF("\r\n");
        if (0 == ret) {
            *len -= g_data_buffer[*len - 1];
            APPLOG("dec *len is [%d]", *len);
            if(*len <= MAX_BUF) {
                os_memcpy(data, g_data_buffer, *len);
            } else {
                ret = -1;
                APPLOG("dec error");
            }
        }
        APPLOG("dec *len is [%d] ret[%d] END", *len, ret);

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
        ret = mbedtls_aes_crypt_cbc(&ginAESCtx, MBEDTLS_AES_ENCRYPT, *len, iv, data, g_data_buffer);
        if (0 == ret) {
            os_memcpy(data, g_data_buffer, *len);
        }
        APPLOG("enc *len is [%d] ret[%d] END", *len, ret);
    }
    

    return ret;
}
#endif
