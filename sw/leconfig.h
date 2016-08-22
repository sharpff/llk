#ifndef __LECONFIG_H__
#define __LECONFIG_H__


#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __LE_SDK__
#include "header.h"
#else
#include "halHeader.h"
#endif /* __LE_SDK__ */

#if defined(WIN32) || defined(EWM3081)
#define LELINK_ALIGNED
#define LELINK_PACK
#else
#define LELINK_ALIGNED __attribute__((packed))
#endif

// osal
#define MUTEX_LOCK halLock()
#define MUTEX_UNLOCK halUnlock()

#define AES_LEN (128/8)
#define RSA_LEN (1024/8)
#define RSA_RAW_LEN ((1024/8) - 11)
#define MAX_RSA_PUBKEY (RSA_LEN + RSA_LEN/2)
#define MD5_LEN (16)
#define SHA1_LEN (16)
#define MAX_UUID (32)
#if (MD5_LEN > AES_LEN)
    #pragma error "MD5_LEN > AES_LEN"
#endif
#if (SHA1_LEN > AES_LEN)
    #pragma error "SHA1_LEN > AES_LEN"
#endif
#define MAX_BUF (1024+256)


#define USED(a) ((void)a)


#define REMOTE_BAK_IP "115.182.94.173"
#define REMOTE_BAK_PORT 5546


// halIO
void *halUartOpen(void *dev);
int halUartClose(void *dev);
int halUartRead(void *dev, uint8_t *buf, uint32_t len);
int halUartWrite(void *dev, const uint8_t *buf, uint32_t len);

void *halGPIOInit(void);
int halGPIOClose(void *dev);
int halGPIOOpen(void *dev);
int halGPIORead(void *dev, int *val);
int halGPIOWrite(void *dev, const int val);

void* halPWMInit(int clock);
int halPWMOpen(void *dev);
int halPWMClose(void *dev);
void halPWMWrite(void *dev, uint32_t percent);
void halPWMRead(void *dev, uint32_t *percent);
void halPWMSetFrequency(void *dev);

void halCommonInit(void* dev);

int halFlashInit(void);
int halFlashDeinit(void);
void *halFlashOpen(void);
int halFlashClose(void *dev);
int halFlashErase(void *dev, uint32_t startAddr, uint32_t size);
int halFlashWrite(void *dev, const uint8_t *data, int len, uint32_t startAddr, int32_t offsetToBegin);
int halFlashRead(void *dev, uint8_t *data, int len, uint32_t startAddr, int32_t offsetToBegin);
int halGetMac(uint8_t *mac, int len);
void *halPipeOpen(char *name);
int halPipeClose(void *dev);
int halPipeRead(void *dev, uint8_t *buf, uint32_t len);
int halPipeWrite(void *dev, const uint8_t *buf, uint32_t len);

// halOS
int halLockInit(void);
void halDeLockInit(void);
int halLock(void);
int halUnlock(void);
unsigned int halGetTimeStamp(void);
unsigned int halGetUTC(void);
int halReboot();
uint32_t halGetCurrentTaskId(void);

void *halMallocEx(size_t size, char* filename, uint32_t line);
void *halCallocEx(size_t n, size_t size, char* filename, uint32_t line);
void *halReallocEx(void *ptr, size_t size, char* filename, uint32_t line);
void halFreeEx(void *ptr, char* filename, uint32_t line);

#if 0
void *_halMalloc(size_t size);
void *_halCalloc(size_t n, size_t size);
void *_halRealloc(void *ptr, size_t size);
void _halFree(void *ptr);
#endif

#define halMalloc(size)        halMallocEx(size, __FILE__, __LINE__)
#define halCalloc(n, size)     halCallocEx(n, size, __FILE__, __LINE__)
#define halRealloc(ptr, size)  halReallocEx(ptr, size, __FILE__, __LINE__)
#define halFree(ptr)           halFreeEx(ptr, __FILE__, __LINE__)

#ifdef __cplusplus
}
#endif


#endif /* __LECONFIG_H__ */
