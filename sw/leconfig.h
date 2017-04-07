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


#if defined(PF_VAL) && (PF_VAL == 7) // for EMW3081(iar compiler)
#define LELINK_WEAK __weak
#else 
#define LELINK_WEAK __attribute__((weak))
#endif

#if defined(WIN32) || defined(EMW3081)
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
    uint8_t id;
    uint8_t dir;
    uint8_t mode;
    uint8_t type; // 0-normal gpio; 1-reset for lelink
    uint8_t state;
    uint8_t oldState;
    /*
     * gpiostate will be filled in lelink while lelink to contorl(write) gpio. 
     * this val should be checked in halGPIOWrite.
     * 1/0 is normal gpio state, just do write(1/0). 
     * 2-quick blink and 3-slow blink should be implemented if nesassary.
     * "longTime":xxx and "shortTime":yyy in sw script has been abolished.
     */
    uint8_t gpiostate;
    void* handler;
    uint32_t reserved1;
    uint32_t reserved2;
} gpioHandler_t;

typedef struct {
    uint8_t id;
    uint8_t type;
    uint8_t mode;
    uint8_t clock;
    uint32_t state;
    uint32_t oldState;
    uint32_t frequency;
    uint32_t percent;
    uint32_t duty;
    void* handler;
    uint32_t reserved1;
    uint32_t reserved2;
} pwmHandler_t;

typedef struct {
    int16_t id;          // support 1, 2, 3 
    uint32_t baud;
    uint8_t dataBits;
    uint8_t stopBits;
    uint8_t flowCtrl;
    uint8_t parity;
    void* handler;
    uint32_t reserved1;
    uint32_t reserved2;
} uartHandler_t;

typedef struct {
    uint8_t id;
    uint8_t gid;
    uint8_t mode;
    uint8_t trigger;
    uint8_t type;
    uint8_t state;
    uint8_t oldState;
    uint8_t longPress;
    uint32_t debounce;
    uint32_t timeout;
    uint32_t count;
    uint32_t timeStamp;
    uint32_t longPressStart;
    uint32_t longPressEnd;
    void* handler;
    uint32_t reserved1;
    uint32_t reserved2;
} eintHandler_t;

typedef struct {
    uint32_t reserved1;
    uint32_t reserved2;
} userHandler_t;

#define COMMON_MAX_ID     (10)

typedef struct {
    uint16_t id; 
    uint16_t mux;
} commonConfig_t;

typedef struct {
    uint32_t num;
    commonConfig_t table[COMMON_MAX_ID + 1];
} commonManager_t;

typedef struct {
    void *ud;
    uint32_t sBasicSize;
    uint8_t sType;
} SContext;

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

int halEINTClose(eintHandler_t *handler);
int halEINTOpen(eintHandler_t *handler);
int halEINTRead(eintHandler_t* handler, int *val);

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
void halDelayms(int ms);
unsigned int halGetUTC(void);
int halReboot();
uint32_t halGetCurrentTaskId(void);

void *halMallocEx(size_t size, const char *filename, uint32_t line);
void *halCallocEx(size_t n, size_t size, const char *filename, uint32_t line);
void *halReallocEx(void *ptr, size_t size, const char *filename, uint32_t line);
void halFreeEx(void *ptr, const char *filename, uint32_t line);

int halGetBroadCastAddr(char *broadcastAddr, int len);
int halCastProbing(const char *mcastIP, const char *bcastIP, int port);
int halAES(uint8_t *key, uint32_t keyLen, uint8_t *iv, uint8_t *data, uint32_t *len, uint32_t maxLen, int isPKCS5, int type);
int halRsaInit();
int halRsaExit();
int halRsaEncrypt(const uint8_t *pubkey, int pubkeyLen, const uint8_t* input, int inputLen, uint8_t *out, int outLen);
int halRsaDecrypt(const uint8_t *prikey, int prikeyLen, const uint8_t* input, int inputLen, uint8_t *out, int outLen);
int halRsaVerify(const uint8_t* pubkey, int pubkeyLen, 
    const uint8_t *raw, int rawLen, const uint8_t *sig, int sigLen);

int halSha1Start();
int halSha1Update(const uint8_t *input, size_t ilen);
int halSha1End(uint8_t output[20]);

int halGetSelfAddr(char *ip, int size, int *port);

int printOut(const char *fmt, ...);
void halPrint(const char *log);
int halNwUDPSendto(int sock, const char *ip, int port, const uint8_t *buf, int len);
int halNwNew(int selfPort, int block, int *sock, int *broadcastEnable);
int halNwUDPRecvfrom(int sock, uint8_t *buf, int len, char *ip, int sizeIP, uint16_t *port);
int halNwDelete(int sock);

int halAESInit(void);
void halDeAESInit(void);
int halAES(uint8_t *key, uint32_t keyLen, uint8_t *iv, uint8_t *data, uint32_t *len, uint32_t maxLen, int isPKCS5, int type);

int halDoConfig(void *ptr, int ptrLen);
int halStopConfig(void);
int halDoConfiguring(void *ptr, int ptrLen);
int halDoApConnect(void *ptr, int ptrLen);
int halDoApConnecting(void *ptr, int ptrLen);
int halSoftApStart(const char *ssid, char *wpa2_passphrase, uint8_t *aesKey, int aesKeyLen);
int halSoftApStop(int success);


int halGetHostByName(const char *name, char ip[4][32], int len);
uint16_t halRand();

int softApDoConfig(const char *ssid, const char *passwd, unsigned int timeout, const char *aesKey);
extern unsigned long halLogTimeStamp(void);
int halWatchDogInit(void);
int halWatchDogFeed(void);
size_t halGetSReservedHeap();


#define applog(_mod_name_, _fmt_, ...) \
    { \
        const char * p = (const char *)strrchr(__FILE__, '/'); \
        printOut("[%u][%s] "_fmt_" @%s:%d\r\n", halLogTimeStamp(), _mod_name_, ##__VA_ARGS__, p ? (p + 1) : __FILE__, __LINE__); \
    }

#define APPLOG(...) \
    applog("LEAPP", ##__VA_ARGS__)

#define APPLOGW(...) \
    applog("LEAPP[W]", ##__VA_ARGS__)

#define APPLOGE(...) \
    applog("LEAPP[E]", ##__VA_ARGS__)

#define APPPRINTF(...) \
    printOut(__VA_ARGS__)

#define APPASSERT(x) \
    { \
        if (!(x))  { \
            const char * p = (const char *)strrchr(__FILE__, '/'); \
            while (1) { \
            printOut("********LEAPP[ASSERT] in file:%s line:%d\r\n", p ? (p + 1) : __FILE__, __LINE__); } \
        } \
    }

#define halMalloc(size)        halMallocEx(size, __FILE__, __LINE__)
#define halCalloc(n, size)     halCallocEx(n, size, __FILE__, __LINE__)
#define halRealloc(ptr, size)  halReallocEx(ptr, size, __FILE__, __LINE__)
#define halFree(ptr)           halFreeEx(ptr, __FILE__, __LINE__)

#define CACHE_NODE_HEADER \
    uint16_t flag; \
    uint16_t nodeReserved;

#define LELINK_OTA_VERIFICATION

#define MONITOR_CONFIG4

#ifdef __cplusplus
}
#endif


#endif /* __LECONFIG_H__ */
