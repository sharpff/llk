#ifndef __IO_H__
#define __IO_H__

// EXPORT

#ifdef __cplusplus
extern "C"
{
#endif
#include "leconfig.h"


#define MAX_STR_LEN 36
#define MAX_REMOTE 64
#define MAX_IA 4
#define MAX_RULE_NAME 64
#define GET_PAGE_SIZE(currSize, SecSize) ((SecSize)*(((currSize)-1)/(SecSize) + 1)) // min erase size

typedef enum {
    IO_TYPE_UART = 0x1,
    IO_TYPE_GPIO = 0x2,
    IO_TYPE_PIPE = 0x4,
    IO_TYPE_SOCKET = 0x8,
    IO_TYPE_PWM = 0x10,
}IO_TYPE;

#ifdef LELINK_PACK
#pragma pack(1)
#endif
/*
 * private info
 */
typedef struct
{
    uint8_t ssid[MAX_STR_LEN];
    uint8_t psk[MAX_STR_LEN];
    uint32_t ipaddr;
    uint32_t mask;
    uint32_t gateway;
    uint32_t dns1;
    uint32_t dns2;
    uint8_t ssid_len;
    uint8_t psk_len;
    uint8_t type;
    uint8_t channel;
}LELINK_ALIGNED WifiNetwork;

typedef enum {
    DEV_FLAG_RESET = 0x01, // reboot by reset key
} DEV_FLAG_t;

/* Read/Write info */
typedef struct {
    uint8_t flag; // DEV_FLAG_t
    uint8_t locked;
    uint8_t reserved[37];
}LELINK_ALIGNED DevCfg;

typedef struct
{
    int configStatus; // 1. wifi has been configed. 2. hello has been sent
    WifiNetwork config;
}LELINK_ALIGNED NwCfg;

typedef struct
{
    int num;
    int arrIA[MAX_IA];
    char arrIAName[MAX_IA][MAX_RULE_NAME];
}LELINK_ALIGNED IACfg;

typedef struct
{
    DevCfg  devCfg;
    NwCfg nwCfg;
    IACfg iaCfg;
}LELINK_ALIGNED PrivateData;

typedef struct
{
    PrivateData data;
    uint8_t csum;
}LELINK_ALIGNED PrivateCfg;


/*
 * -------this size should be sync with lelinkTool.py START------
 */
#define SDEV_MAX_INFO 320
#define SDEV_MAX_MAC 24
#define SDEV_MAX_EPT 8
#define SDEV_MAX_CUST 8
/*
 * -------this size should be sync with lelinkTool.py END------
 */
#define MAX_SDEV_NUM 16

// char sdevInfo[SDEV_MAX_INFO]; // as json object "sDev".
// char mac[SDEV_MAX_MAC]; // long mac address
// uint8_t sdevEpt[SDEV_MAX_EPT]; // the element - 1 is the real ept
// uint8_t sdevMan[SDEV_MAX_CUST]; // for the manufacturer
// uint8_t idx[SDEV_MAX_MAC/4]; // short mac or index



#define CACHE_NODE_NBASE \
    CACHE_NODE_HEADER; \
    char sdevInfo[SDEV_MAX_INFO]; \
    char mac[SDEV_MAX_MAC]; \
    uint8_t idx[SDEV_MAX_MAC]; \
    uint8_t sdevEpt[SDEV_MAX_EPT]; \
    uint8_t sdevMan[SDEV_MAX_CUST]; \
    uint8_t reserved[4];

typedef struct {
    CACHE_NODE_NBASE;
}LELINK_ALIGNED SDevNBase;

typedef struct
{
    SDevNBase data;
    uint8_t csum;
}LELINK_ALIGNED SDevInfoCfg;

/*
 * auth info
 */
typedef struct {
    uint8_t uuid[MAX_UUID];
    int pubkeyLen;
    int signatureLen;
    uint8_t signature[RSA_LEN];
    uint8_t pubkey[MAX_RSA_PUBKEY];
    char remote[MAX_REMOTE];
    uint16_t port;
    uint16_t reserved;
}LELINK_ALIGNED AuthData;

typedef struct
{
    AuthData data;
    uint8_t csum;
}LELINK_ALIGNED AuthCfg;
#ifdef LELINK_PACK
#pragma pack()
#endif

typedef enum {
    E_FLASH_TYPE_AUTH, 
    E_FLASH_TYPE_PROFILE_DEV, // XXX
    E_FLASH_TYPE_PROFILE_PROD, // XXX
    E_FLASH_TYPE_SCRIPT, 
    E_FLASH_TYPE_PRIVATE,
    E_FLASH_TYPE_SCRIPT2, 
    E_FLASH_TYPE_SDEV_INFO, // XXX
    E_FLASH_TYPE_MAX,
}E_FLASH_TYPE;

typedef struct {
    E_FLASH_TYPE type;
    uint32_t addr;
    uint32_t size;
}FlashRegion;

int lelinkStorageInit(uint32_t startAddr, uint32_t totalSize, uint32_t minSize);

int lelinkStorageWritePrivateCfg(const PrivateCfg *privateCfg);
int lelinkStorageReadPrivateCfg(PrivateCfg *privateCfg);

int lelinkStorageWriteAuthCfg(const AuthCfg *authCfg);
int lelinkStorageReadAuthCfg(AuthCfg *authCfg);

/*
 * type: 0-fw script, 1-ia script
 */
int lelinkStorageWriteScriptCfg(const void *scriptCfg, int flashType, int idx);
int lelinkStorageReadScriptCfg(void* scriptCfg, int flashType, int idx);

int lelinkStorageWriteSDevInfoCfg(const SDevInfoCfg *sdevInfoCfg, int idx);
int lelinkStorageReadSDevInfoCfg(SDevInfoCfg *sdevInfoCfg, int idx);

void lelinkStorageDeinit(void);


/*
 * UART is 0x1
 * GIPO is 0x2
 */
// #define MAX_IO_HDL 4
typedef struct {
    int ioType;
    void *hdl;
}IOHDL;

void *ioInit(int ioType, const char *json, int jsonLen);
void **ioGetHdl(int *ioType);
IOHDL *ioGetHdlExt();
int ioGetHdlCounts();
int ioWrite(int ioType, void *hdl, const uint8_t *data, int dataLen);
int ioRead(int ioType, void *hdl, uint8_t *data, int dataLen);
void ioDeinit(int ioType, void *hdl);

typedef enum {
    GPIO_DIR_INPUT = 0,
    GPIO_DIR_OUTPUT,
} GPIO_DIR_t;

typedef enum {
    GPIO_MODE_DEFAULT = 0,
    GPIO_MODE_PULLUP,
    GPIO_MODE_PULLDOWN,
    GPIO_MODE_NOPULL,
    GPIO_MODE_RISTATE,
} GPIO_MODE_t;

typedef enum {
    GPIO_STATE_LOW = 0,
    GPIO_STATE_HIGH,
    GPIO_STATE_BLINK,
} GPIO_STATE_t;

typedef enum {
    GPIO_TYPE_INPUT_RESET = 1
} GPIO_TYPE_INPUT_t;

typedef enum {
    GPIO_TYPE_OUTPUT_RESET = 1
} GPIO_TYPE_OUTPUT_t;

typedef enum {
   PWM_TYPE_OUTPUT_RESET = 1
} PWM_TYPE_OUTPUT_t;

typedef enum {
    PWM_STATE_LOW = 0,
    PWM_STATE_HIGH
} PWM_STATE_t;

#define GPIO_MAX_ID     (3)

typedef struct {
    uint32_t num;
    gpioHandler_t table[GPIO_MAX_ID + 1];
} gpioManager_t;

#define PWM_MAX_ID     (4)

typedef struct {
    uint32_t num;
    pwmHandler_t table[PWM_MAX_ID + 1];
} pwmManager_t;

typedef enum {
    RLED_STATE_IGNORE = -1, // 忽略, 设置的时候返回当前状态
    RLED_STATE_FREE, // 空闲状态(正常运行)
    RLED_STATE_WIFI, // wifi配置中
    RLED_STATE_CONNECTING, // connecting
    RLED_STATE_RUNNING // normally
} RLED_STATE_t;
RLED_STATE_t setResetLed(RLED_STATE_t st);

#ifdef __cplusplus
}
#endif

#endif
