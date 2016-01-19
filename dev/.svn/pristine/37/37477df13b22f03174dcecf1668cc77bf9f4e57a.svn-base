#ifndef __IO_H__
#define __IO_H__

// EXPORT

#ifdef __cplusplus
extern "C"
{
#endif
#include "leconfig.h"


#define MAX_STR_LEN 36



// #ifdef ALINGED 
// #pragma message("ALIGNED defined");
// #else
// #pragma message("ALIGNED NOT DEFINED")
// #endif



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
    uint8_t new;
    uint8_t bind;
    uint8_t lock;
}ALIGNED DevCfg;

typedef struct
{
    int configStatus; // 1. wifi has been configed. 2. hello has been sent
    WifiNetwork config;
}ALIGNED NwCfg;

typedef struct
{
    DevCfg  devCfg;
    NwCfg nwCfg;
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
}ALIGNED AuthData;

typedef struct
{
    AuthData data;
    uint8_t csum;
}ALIGNED AuthCfg;

int lelinkStorageInit(uint32_t startAddr, uint32_t totalSize, uint32_t pageSize);

int lelinkStorageWritePrivateCfg(const PrivateCfg *privateCfg);
int lelinkStorageReadPrivateCfg(PrivateCfg *privateCfg);

int lelinkStorageWriteAuthCfg(const AuthCfg *authCfg);
int lelinkStorageReadAuthCfg(AuthCfg *authCfg);

void lelinkStorageDeinit(void);


int ioWrite(int type, const uint8_t *data, int dataLen);
int ioRead(int type, uint8_t *data, int dataLen);

#ifdef __cplusplus
}
#endif

#endif