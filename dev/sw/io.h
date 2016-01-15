#ifndef __IO_H__
#define __IO_H__

// EXPORT

#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_STR_LEN 36

#define SECTOR_SIZE 0x800 // 2KB, 4KB
#define BLOCK_SIZE 0x8000 // 32 KB, 64KB
#define GET_SIZE(s) (SECTOR_SIZE*((s-1)/SECTOR_SIZE + 1))

#define FLASH_ADDR_PRIVATE_CFG 0x1c3000 
#define FLASH_SIZE_PRIVATE_CFG (2*SECTOR_SIZE) // 0X1000 

int ioWrite(int type, const uint8_t *data, int dataLen);
int ioRead(int type, uint8_t *data, int dataLen);


/*
 * Flash size: xxxx KB
 * 
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
}WifiNetwork;

/* Read/Write info */
typedef struct DevCfg {
    uint8_t name[MAX_STR_LEN];
    uint8_t new;
    uint8_t bind;
    uint8_t lock;
}DevCfg;

typedef struct NwCfg
{
    int configStatus; // 1. wifi has been configed. 2. hello has been sent
    WifiNetwork config;
}NwCfg;

typedef struct
{
    DevCfg  devCfg;
    NwCfg nwCfg;
}PrivateData;

typedef struct
{
    PrivateData data;
    uint8_t csum;
    int8_t idx;
}PrivateCfg;

int lelinkFlashWritePrivateCfg(const PrivateCfg *privateCfg);
int lelinkFlashReadPrivateCfg(PrivateCfg *privateCfg);


#ifdef __cplusplus
}
#endif

#endif