#include "leconfig.h"
#include "io.h"

// #define SECTOR_SIZE ginPageSize // 0x800 // 2KB
// #define BLOCK_SIZE 0x8000 // 32KB
#define GET_PAGE_SIZE(currSize, SecSize) ((SecSize)*(((currSize)-1)/(SecSize) + 1)) // min erase size

// // 0x1c2000
// #define STORAGE_ADDR_PRIVATE_CFG (ginStartAddr) // 0x1c2000 for Marvell
// #define STORAGE_SIZE_PRIVATE_CFG (GET_ERASE_SIZE(sizeof(PrivateCfg))) // 0X1000

// #define STORAGE_ADDR_AUTH_CFG (ginStartAddr + STORAGE_SIZE_PRIVATE_CFG)
// #define STORAGE_SIZE_AUTH_CFG (GET_ERASE_SIZE(sizeof(AuthCfg))) // 0X1000

// #define STORAGE_ADDR_TEST_CFG (ginStartAddr + STORAGE_SIZE_PRIVATE_CFG + STORAGE_ADDR_AUTH_CFG)
// #define STORAGE_SIZE_TEST_CFG (GET_ERASE_SIZE(26)) // 0X1000


extern void *halFlashOpen(void);

typedef enum {
    E_FLASH_TYPE_AUTH,
    E_FLASH_TYPE_PROFILE_DEV,
    E_FLASH_TYPE_PROFILE_PROD,
    E_FLASH_TYPE_PRIVATE,
    E_FLASH_TYPE_TEST,
    E_FLASH_TYPE_MAX,
}E_FLASH_TYPE;

typedef struct {
    E_FLASH_TYPE type;
    uint32_t addr;
    uint32_t size;
}FlashRegion;

static FlashRegion ginRegion[E_FLASH_TYPE_MAX];
static uint32_t ginStartAddr;
static uint32_t ginTotalSize;
static uint32_t ginPageSize;

/*
 * modify here, in order to change the ocuppied size.
 */
static uint32_t getSize(E_FLASH_TYPE type, uint32_t pageSize) {
    uint32_t ret = 0;
    switch (type) {
        case E_FLASH_TYPE_AUTH: {
            ret = GET_PAGE_SIZE(sizeof(AuthCfg), pageSize);
        } break;
        case E_FLASH_TYPE_PRIVATE: {
            ret = GET_PAGE_SIZE(sizeof(PrivateCfg), pageSize);
        } break;
        case E_FLASH_TYPE_TEST: {
            ret = GET_PAGE_SIZE(26, pageSize);
        } break;
        default:
            break;
    }
    return ret;
}

static int getRegion(E_FLASH_TYPE type, FlashRegion *region) {
    if (!ginTotalSize || !ginPageSize) {
        return -1;
    }

    if (NULL == region) {
        return -2;
    }

    if (type != ginRegion[type].type) {
        return -3;
    }

    memcpy(region, &ginRegion[type], sizeof(FlashRegion));

    return type;
}




int lelinkStorageInit(uint32_t startAddr, uint32_t totalSize, uint32_t pageSize) {
    int i = 0;
    uint32_t tmpTotal = 0, tmpSize = 0;
    // uint32_t tmpStartAddr = startAddr;
    for (i = 0; i < E_FLASH_TYPE_MAX; i++) {
        tmpTotal += getSize(i, pageSize);
    }

    if (tmpTotal > totalSize) {
        return -1;
    }

    ginStartAddr = startAddr;
    ginTotalSize = totalSize;
    ginPageSize = pageSize;

    for (i = 0; i < E_FLASH_TYPE_MAX; i++) {
        ginRegion[i].type = i;
        ginRegion[i].size = getSize(i, pageSize);
        // LELOG("[%d] [%d]\r\n", i, ginRegion[i].size);
        ginRegion[i].addr = ginStartAddr + tmpSize;
        tmpSize += ginRegion[i].size;
        LELOG("idx[%d] addr[0x%x] size[0x%x]\r\n", i, ginRegion[i].addr, ginRegion[i].size);
    }

    return 0;
}

void lelinkStorageDeinit(void) {
    // int i = 0;
    ginStartAddr = 0;
    ginTotalSize = 0;
    ginPageSize = 0;
    memset(ginRegion, 0, sizeof(ginRegion));
}

static int storageWrite(E_FLASH_TYPE type, const void *data, int size) {
    int ret;
    void *hdl;
    FlashRegion fr;

    ret = getRegion(type, &fr);
    if (0 > ret) {
        return -1;
    }

    hdl = (void *)halFlashOpen();
    if (NULL == hdl) {
        return -2;
    }

    ret = halFlashErase(hdl, fr.addr, fr.size);
    if (0 > ret) {
        return -3;
    }
    // LELOG("flashWritePrivateCfg halFlashErase [0x%x] [0x%x][0x%x]\r\n", hdl, fr.addr, fr.size);

    *((uint8_t *)data + (size - 1)) = crc8(data, size - 1);
    ret = halFlashWrite(hdl, data, size, fr.addr);
    if (0 > ret) {
        return -4;
    }
    
    // LELOG("flashWritePrivateCfg halFlashWrite [0x%x] [0x%x][0x%x]\r\n", hdl, fr.addr, fr.size);
    halFlashClose(hdl);
    return 0; 
}

static int storageRead(E_FLASH_TYPE type, void *data, int size) {
    int ret = 0;
    void *hdl;
    FlashRegion fr;

    ret = getRegion(type, &fr);
    if (0 > ret) {
        return -1;
    }

    hdl = (void *)halFlashOpen();
    if (NULL == hdl) {
        return -2;
    }

    ret = halFlashRead(hdl, data, size, fr.addr);
    if (0 > ret) {
        return -3;
    }

    // LELOG("flashReadPrivateCfg [0x%x] [0x%x][0x%x]\r\n", hdl, STORAGE_SIZE_PRIVATE_CFG, fr.addr);
    halFlashClose(hdl);

    return 0;
}

// static uint8_t ginIsPrivateCfgChanged = 1;
// static PrivateCfg ginPrivateCfg;
int lelinkStorageWritePrivateCfg(const PrivateCfg *privateCfg) {
    int ret = 0;

    ret = storageWrite(E_FLASH_TYPE_PRIVATE, privateCfg, sizeof(PrivateCfg));

    return ret;
}

int lelinkStorageReadPrivateCfg(PrivateCfg *privateCfg) {
    int ret = 0;

    ret = storageRead(E_FLASH_TYPE_PRIVATE, privateCfg, sizeof(PrivateCfg));

    return ret;
}

int lelinkStorageWriteAuthCfg(const AuthCfg *authCfg) {
    int ret = 0;

    ret = storageWrite(E_FLASH_TYPE_AUTH, authCfg, sizeof(AuthCfg));

    return ret;
}

int lelinkStorageReadAuthCfg(AuthCfg *authCfg) {
    int ret = 0;

    ret = storageRead(E_FLASH_TYPE_AUTH, authCfg, sizeof(AuthCfg));

    return ret;
}

int preGenStableInfo2Flash(void) {
    int ret = 0;
    AuthCfg authCfg;
    uint8_t mac[6] = {0};
    char macStr[13] = {0};
    const char beVerifiedSDK1[] = "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCCJHkF6iOBX8ObCiS1tmyM6tgrU2QqR0ByyfMXjvp1x2nKiSNABXtjLAiEUPHBMypThBUSADlw6zM7XOANHmaWSoDKWOjDZKeK82uZDh/4C2k806zLb8wiTrh+e+qeKaBiNnY8PMMoBCYrNvO/gnM8aU4GIyhQnGfTkgcpSm2aqwIDAQAB";
    // const char beVerifiedDEV3[] = "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCf2eu1g68UFbocZLROH90/3rGWpnJkOkRWSO4C3QUjMJ00b4nJDqbTwwkr9w1sLIJd5VsQ0UHwl80+62E6PcUV1ST9KgLfPyvqNbhN3NmpqPOS5wCZGsFp8zGkS9NYdtc3KmClF2K5OlSTaxg7EgdYwytRa1IxZdRNc1MJiKgEtwIDAQAB";

    static uint8_t pubkeySDK1Der[] =
    {
    		0x30, 0x81, 0x9f, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01,
    		0x05, 0x00, 0x03, 0x81, 0x8d, 0x00, 0x30, 0x81, 0x89, 0x02, 0x81, 0x81, 0x00, 0xab, 0x4f, 0x1a,
    		0x55, 0x24, 0x7c, 0xdb, 0x2e, 0x8c, 0x52, 0x93, 0xb0, 0xb3, 0x05, 0xa7, 0xb2, 0x83, 0x80, 0xb3,
    		0xcf, 0xba, 0xc0, 0x35, 0xfa, 0x99, 0x52, 0xee, 0xdb, 0x0f, 0x9e, 0xc2, 0xe1, 0xf1, 0x9b, 0x99,
    		0xf7, 0x5a, 0x30, 0x41, 0xb8, 0x55, 0xfb, 0x8c, 0xc5, 0x66, 0x30, 0xd3, 0xc3, 0x8b, 0x5b, 0xf7,
    		0xfb, 0xdf, 0x91, 0x70, 0x56, 0x93, 0xfc, 0xf5, 0x83, 0xce, 0x72, 0x3a, 0xd1, 0x5b, 0xfa, 0x3f,
    		0xa7, 0xbe, 0x94, 0x7c, 0x84, 0xbc, 0xe0, 0xa6, 0x42, 0x90, 0xc9, 0x05, 0x8b, 0xfd, 0x88, 0x9d,
    		0xf3, 0x53, 0x9b, 0xe7, 0xa9, 0xda, 0xdb, 0x8e, 0x94, 0x2a, 0x6a, 0xec, 0x2e, 0x93, 0x4d, 0xd5,
    		0xf4, 0xe0, 0xc2, 0x6c, 0x4e, 0x97, 0xdb, 0x11, 0x1a, 0x8a, 0x9d, 0xfb, 0x92, 0x05, 0x16, 0xe0,
    		0xd7, 0x63, 0x84, 0xb6, 0x99, 0x91, 0x99, 0xc4, 0xe6, 0x62, 0xa8, 0xe8, 0xb1, 0x02, 0x03, 0x01,
    		0x00, 0x01
    };
    static uint8_t signatureSDK1[] = {
    		0x85, 0x8f, 0x99, 0x01, 0xd5, 0xe8, 0x14, 0x15, 0x4d, 0xa3, 0x03, 0x2e, 0xa8, 0x05, 0xa3, 0x86,
    		0x85, 0x2e, 0x6d, 0x0b, 0x9c, 0x76, 0x10, 0xd5, 0xa5, 0xaf, 0x09, 0xcc, 0x01, 0x43, 0xe8, 0x74,
    		0xd1, 0xec, 0x99, 0x33, 0xd5, 0xc5, 0x2f, 0xae, 0xf6, 0x8c, 0x87, 0xf3, 0x85, 0x8b, 0xc2, 0xe0,
    		0x4c, 0x18, 0xa7, 0x1c, 0x36, 0x87, 0xa6, 0x76, 0x41, 0xbd, 0xf0, 0xe5, 0xf7, 0x94, 0x85, 0xc9,
    		0xe2, 0x41, 0xa4, 0xb1, 0x4c, 0xd6, 0x9f, 0x46, 0xd2, 0x6e, 0x3a, 0x65, 0x51, 0x12, 0x91, 0xe1,
    		0x8f, 0x8d, 0x02, 0x64, 0xc5, 0xde, 0x69, 0x8d, 0x81, 0x61, 0xcd, 0xba, 0x43, 0x4a, 0x50, 0x73,
    		0x31, 0x61, 0x9f, 0x60, 0xf9, 0x56, 0xae, 0x98, 0x19, 0x34, 0x67, 0x96, 0x27, 0x98, 0x55, 0x73,
    		0xc9, 0x6e, 0xfe, 0x55, 0xb6, 0x39, 0x23, 0x2b, 0x23, 0x46, 0x79, 0xc4, 0x00, 0xcc, 0xab, 0x74
    };

    // const uint8_t pubkeyDEV3Der[] =
    // {
    //     0x30, 0x81, 0x9F, 0x30, 0x0D, 0x06, 0x09, 0x2A, 0x86, 0x48, 
    //     0x86, 0xF7, 0x0D, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x81, 
    //     0x8D, 0x00, 0x30, 0x81, 0x89, 0x02, 0x81, 0x81, 0x00, 0xBA, 
    //     0x33, 0xE2, 0xCC, 0xBE, 0x10, 0x2F, 0xCC, 0x89, 0xCA, 0x9D, 
    //     0x64, 0x63, 0x69, 0x09, 0x7B, 0x3E, 0xAA, 0xE9, 0x56, 0x03, 
    //     0xE0, 0x6F, 0x3B, 0xD9, 0x93, 0x07, 0x7E, 0x1B, 0x04, 0xF0, 
    //     0x81, 0xA3, 0xBE, 0x99, 0x5F, 0x64, 0x1C, 0x52, 0xFA, 0x5E, 
    //     0x1A, 0x2B, 0xE3, 0x73, 0x20, 0x89, 0x2D, 0xD0, 0xE3, 0x5E, 
    //     0x93, 0xD8, 0x9B, 0x69, 0x9C, 0x73, 0x1D, 0x29, 0x13, 0x37, 
    //     0x25, 0x30, 0x48, 0x2A, 0x45, 0x05, 0x24, 0xF2, 0x81, 0x24, 
    //     0xEE, 0x01, 0x62, 0x75, 0x01, 0xA6, 0xC5, 0x6D, 0xCE, 0xEC, 
    //     0xED, 0xDE, 0xFC, 0x4E, 0x6A, 0x4F, 0x2B, 0x0B, 0x81, 0x86, 
    //     0xF7, 0x9B, 0xDB, 0xEE, 0xBD, 0x20, 0x85, 0x6A, 0xBA, 0x9C, 
    //     0x80, 0xAB, 0xE7, 0x88, 0x46, 0x96, 0x48, 0x75, 0xF6, 0x8F, 
    //     0xEE, 0xEA, 0xEA, 0x21, 0x4C, 0x1B, 0x19, 0x8B, 0xCE, 0xCD, 
    //     0x67, 0xFE, 0xF9, 0x03, 0x2D, 0xF6, 0x93, 0x02, 0x03, 0x01, 
    //     0x00, 0x01
    // };

    // const uint8_t signatureDEV3[] = {
    //     0x66, 0xeb, 0xf6, 0xde, 0x82, 0x4c, 0xff, 0xce, 0xa1, 0x16, 0xac, 0x30, 0x5e, 0xfd, 0xae, 0x32, 
    //     0x47, 0x3a, 0xd2, 0x9e, 0x4e, 0xc1, 0x6c, 0xb1, 0x60, 0xc4, 0xce, 0x1f, 0x22, 0x2e, 0xf9, 0x28, 
    //     0x46, 0xdc, 0x89, 0x26, 0x58, 0xa5, 0xcd, 0x08, 0xd3, 0x45, 0x60, 0x6f, 0x55, 0x91, 0xcd, 0xc0, 
    //     0xa6, 0x05, 0xae, 0x97, 0x45, 0xa9, 0x15, 0x06, 0x98, 0x4f, 0xdf, 0x65, 0x62, 0x16, 0xd4, 0x2b, 
    //     0xc1, 0x92, 0xea, 0x1d, 0x63, 0xca, 0xa8, 0x8f, 0x6e, 0x3e, 0xde, 0x08, 0x78, 0x59, 0x71, 0xb5, 
    //     0x4d, 0xb1, 0xa4, 0x00, 0xf9, 0x49, 0xb0, 0xfd, 0x2d, 0xfe, 0x16, 0x3e, 0xc7, 0x84, 0x10, 0x49, 
    //     0xff, 0x63, 0x30, 0xc0, 0x88, 0xb8, 0x1a, 0x07, 0xd3, 0xeb, 0x90, 0x58, 0x5b, 0xde, 0xb2, 0x4f, 
    //     0xda, 0x74, 0x58, 0x38, 0x66, 0x58, 0x4c, 0x8e, 0x9e, 0x61, 0xad, 0x6f, 0x7a, 0xc4, 0x1c, 0x1a
    // };

    // verify 
//    ret = rsaVerify(pubkeySDK1Der, sizeof(pubkeySDK1Der), beVerifiedSDK1, strlen(beVerifiedSDK1), signatureSDK1, sizeof(signatureSDK1));
//    LELOG("rsaVerify [%d]\r\n", ret);
//    if (0 > ret) {
//        return -1;
//    }

    ret = lelinkStorageInit(0x12C000, 0x3E000, 256);
    if (0 > ret) {
        return -3;
    }

    // uuid 
    memcpy(authCfg.data.uuid, "10000100101000710007", 20);
    if (0 == halGetMac(mac, sizeof(mac))) {
        bytes2hexStr(mac, sizeof(mac), macStr, sizeof(macStr));
        memcpy(authCfg.data.uuid + 20, macStr, 12);
    }
    // pubkey
    authCfg.data.pubkeyLen = sizeof(pubkeySDK1Der);
    memcpy(authCfg.data.pubkey, pubkeySDK1Der, authCfg.data.pubkeyLen);

    // signature
    authCfg.data.signatureLen = sizeof(signatureSDK1);
    memcpy(authCfg.data.signature, signatureSDK1, RSA_LEN);
    ret = lelinkStorageWriteAuthCfg(&authCfg);
    if (0 > ret) {
        lelinkStorageDeinit();
        return -3;
    }
    
    if (0)
    {
        AuthCfg authCfg2;
        ret = lelinkStorageReadAuthCfg(&authCfg2);
        if (authCfg2.csum != crc8(&(authCfg2.data), sizeof(authCfg2.data))) {
            LELOG("not matched\n");
        } else {
            LELOG("OK\n");
        }
    }

    lelinkStorageDeinit();
    return 0;
}

// int flashWritePrivateCfg(const PrivateCfg *privateCfg) {
//     int ret = 0;
//     uint8_t buf[GET_SIZE(sizeof(PrivateCfg))] = {0};
//     void *hdl = (void *)halFlashOpen();
//     if (NULL == hdl) {
//         return -1;
//     }

//     ret = halFlashErase(hdl, STORAGE_ADDR_PRIVATE_CFG, STORAGE_SIZE_PRIVATE_CFG);
//     if (0 > ret) {
//         return -2;
//     }
//     LELOG("flashWritePrivateCfg halFlashErase [0x%x] [0x%x][0x%x]\r\n", hdl, STORAGE_ADDR_PRIVATE_CFG, STORAGE_SIZE_PRIVATE_CFG);

//     memcpy(buf, privateCfg, sizeof(PrivateCfg));
//     ret = halFlashWrite(hdl, buf, sizeof(buf), STORAGE_ADDR_PRIVATE_CFG);
//     if (0 > ret) {
//         return -3;
//     }
//     LELOG("flashWritePrivateCfg halFlashWrite [0x%x] [0x%x][0x%x]\r\n", hdl, STORAGE_ADDR_PRIVATE_CFG, STORAGE_SIZE_PRIVATE_CFG);
//     halFlashClose(hdl);
//     return 0;
// }

// int flashReadPrivateCfg(PrivateCfg *privateCfg) {
//     int ret = 0;
//     uint8_t buf[GET_SIZE(sizeof(PrivateCfg))] = {0};
//     void *hdl = (void *)halFlashOpen();
//     if (NULL == hdl) {
//         return -1;
//     }

//     ret = halFlashRead(hdl, buf, sizeof(buf), STORAGE_ADDR_PRIVATE_CFG);
//     if (0 > ret) {
//         return -2;
//     }
//     memcpy(privateCfg, buf, sizeof(PrivateCfg));
//     LELOG("flashReadPrivateCfg [0x%x] [0x%x][0x%x]\r\n", hdl, STORAGE_SIZE_PRIVATE_CFG, STORAGE_ADDR_PRIVATE_CFG);
//     halFlashClose(hdl);

//     return 0;
// }




int ioWrite(int type, const uint8_t *data, int dataLen) {
    return 0;
}

int ioRead(int type, uint8_t *data, int dataLen) {
    return 0;
}
