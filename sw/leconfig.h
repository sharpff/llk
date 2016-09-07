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

typedef struct {
    uint8_t id;          // support 1, 2, 3 
    //int8_t num;         // gpio num
    uint16_t dir:1;     // 0 - input; 1 - output
    uint16_t mode:3;    // 0 - default; 1 - pullup; 2 - pulldown; 3 - nopull; 4 - tristate
    uint16_t state:3;   // 0 - low; 1 - high; 2 - blink
    uint16_t type:3;    // 0 - stdio; input: 1 - reset; output: 1 - reset
    uint16_t gpiostate:1;   // only : 0 - low; 1 - high
    uint16_t freestate:1;   // only output reset: 0 - low; 1 - high
    uint8_t blink;          // only output. ticks, blink frequency
    uint8_t oldState; 
    // for input/output type reset
    uint8_t longTime;
    uint8_t shortTime;
    // for output type reset
    // TODO: only for internal
    uint8_t keepLowTimes;   // ticks, gpiostat keep low times
    uint8_t keepHighTimes;  // ticks, gpiostat keep high times
    void* handler;
    uint32_t reserved1;
    uint32_t reserved2;
} gpioHandler_t;

typedef struct {
    uint8_t id;            // support 1, 2, 3, 4
    uint8_t type;
    uint8_t blink;
    uint8_t state;
    uint8_t oldState;
    uint8_t longTime;
    uint8_t shortTime;
    uint8_t clock;
    uint32_t frequency;
    uint32_t percent;
    uint32_t duty;
    void* handler;
    uint32_t reserved1;
    uint32_t reserved2;
} pwmHandler_t;

typedef struct {
    int16_t id;          // support 1, 2, 3 
    int32_t baud;
    int8_t dataBits;
    int8_t stopBits;
    int8_t flowCtrl;
    int8_t parity;
    void* handler;
    uint32_t reserved1;
    uint32_t reserved2;
} uartHandler_t;

#define COMMON_MAX_ID     (10)

typedef struct {
    uint16_t id; 
    uint16_t mux;
} commonConfig_t;

typedef struct {
    uint32_t num;
    commonConfig_t table[COMMON_MAX_ID + 1];
} commonManager_t;

// halIO
void *halUartOpen(uartHandler_t* handler);
int halUartClose(uartHandler_t* handler);
int halUartRead(uartHandler_t* handler, uint8_t *buf, uint32_t len);
int halUartWrite(uartHandler_t* handler, const uint8_t *buf, uint32_t len);

void *halGPIOInit(void);
int halGPIOClose(gpioHandler_t* handler);
int halGPIOOpen(gpioHandler_t* handler);
int halGPIORead(gpioHandler_t* handler, int *val);
int halGPIOWrite(gpioHandler_t* handler, const int val);

void* halPWMInit(int clock);
int halPWMOpen(pwmHandler_t* handler);
int halPWMClose(pwmHandler_t* handler);
void halPWMWrite(pwmHandler_t* handler, uint32_t percent);
void halPWMRead(pwmHandler_t* handler, uint32_t *percent);
void halPWMSetFrequency(pwmHandler_t* handler);

void halCommonInit(commonManager_t* dev);

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
