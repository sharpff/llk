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
}IO_TYPE;

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
}ALIGNED WifiNetwork;

/* Read/Write info */
typedef struct {
    uint8_t name[MAX_STR_LEN];
    uint8_t newOne;
    uint8_t bind;
    uint8_t locked;
}ALIGNED DevCfg;

typedef struct
{
    int configStatus; // 1. wifi has been configed. 2. hello has been sent
    WifiNetwork config;
}ALIGNED NwCfg;

typedef struct
{
    int num;
    int arrIA[MAX_IA];
    char arrIAName[MAX_IA][MAX_RULE_NAME];
}ALIGNED IACfg;

typedef struct
{
    DevCfg  devCfg;
    NwCfg nwCfg;
    IACfg iaCfg;
}ALIGNED PrivateData;

typedef struct
{
    PrivateData data;
    uint8_t csum;
}ALIGNED PrivateCfg;


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
}ALIGNED AuthData;

typedef struct
{
    AuthData data;
    uint8_t csum;
}ALIGNED AuthCfg;

typedef enum {
    E_FLASH_TYPE_AUTH, 
    E_FLASH_TYPE_PROFILE_DEV, // XXX
    E_FLASH_TYPE_PROFILE_PROD, // XXX
    E_FLASH_TYPE_SCRIPT, 
    E_FLASH_TYPE_PRIVATE,
    E_FLASH_TYPE_SCRIPT2, 
    E_FLASH_TYPE_TEST, // XXX
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

#define GPIO_MAX_ID     (3)
typedef struct {
    int8_t id;          // support 1, 2, 3 
    int8_t num;         // gpio num
    uint16_t dir:1;     // 0 - input; 1 - output
    uint16_t mode:3;    // 0 - default; 1 - pullup; 2 - pulldown; 3 - nopull; 4 - tristate
    uint16_t state:3;   // 0 - low; 1 - high; 2 - blink
    uint16_t type:3;    // 0 - stdio; input: 1 - reset; output: 1 - reset
    uint16_t gpiostate:1;   // only : 0 - low; 1 - high
    uint16_t freestate:1;   // only output reset: 0 - low; 1 - high
    uint8_t blink;          // only output. ticks, blink frequency
    // for input/output type reset
    uint8_t longTime;
    uint8_t shortTime;
    // for output type reset
    // TODO: only for internal
    uint8_t keepLowTimes;   // ticks, gpiostat keep low times
    uint8_t keepHighTimes;  // ticks, gpiostat keep high times
    uint8_t reserved;   
} gpioHand_t;

typedef struct {
    void *handle;
    uint32_t num;
    gpioHand_t table[GPIO_MAX_ID + 1];
} gpioManager_t;

#ifdef __cplusplus
}
#endif

#endif
