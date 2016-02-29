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
#define MAX_IA 8

typedef enum {
    IO_TYPE_UART,
    IO_TYPE_PIPE,
    IO_TYPE_SOCKET,
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
int lelinkStorageWriteScriptCfg(const void *scriptCfg, int type);
int lelinkStorageReadScriptCfg(void* scriptCfg, int type);

void lelinkStorageDeinit(void);


/*
 * 1. uart
 * 2. pipe
 * . socket
 */
void *ioInit(int ioType, const char *json, int jsonLen);
void **ioGetHdl();
int ioWrite(int ioType, void *hdl, const uint8_t *data, int dataLen);
int ioRead(int ioType, void *hdl, uint8_t *data, int dataLen);
void ioDeinit(int ioType, void *hdl);

#ifdef __cplusplus
}
#endif

#endif