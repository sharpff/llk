#include "leconfig.h"
#include "io.h"
#include "sengine.h"

// #define SECTOR_SIZE ginMinSize // 0x800 // 2KB
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
    E_FLASH_TYPE_SCRIPT, 
    E_FLASH_TYPE_PRIVATE,
    E_FLASH_TYPE_TEST,
    E_FLASH_TYPE_MAX,
}E_FLASH_TYPE;

typedef struct {
    E_FLASH_TYPE type;
    uint32_t addr;
    uint32_t size;
}FlashRegion;

int lelinkStorageWriteScriptCfg(const ScriptCfg *scriptCfg);
int lelinkStorageReadScriptCfg(ScriptCfg *scriptCfg);


static FlashRegion ginRegion[E_FLASH_TYPE_MAX];
static uint32_t ginStartAddr;
static uint32_t ginTotalSize;
static uint32_t ginMinSize;
const int ginScriptSize;
/*
 * modify here, in order to change the ocuppied size.
 */
static uint32_t getSize(E_FLASH_TYPE type, uint32_t minSize) {
    uint32_t ret = 0;
    switch (type) {
        case E_FLASH_TYPE_AUTH: {
            ret = GET_PAGE_SIZE(sizeof(AuthCfg), minSize);
        } break;
        // case E_FLASH_TYPE_PROFILE_DEV: {
        //     ret = GET_PAGE_SIZE(sizeof(XXX), minSize);
        // } break;
        // case E_FLASH_TYPE_PROFILE_PROD: {
        //     ret = GET_PAGE_SIZE(sizeof(XXX), minSize);
        // } break;
        case E_FLASH_TYPE_SCRIPT: {
            ret = GET_PAGE_SIZE(sizeof(ScriptCfg), minSize);
        } break;
        case E_FLASH_TYPE_PRIVATE: {
            ret = GET_PAGE_SIZE(sizeof(PrivateCfg), minSize);
        } break;
        case E_FLASH_TYPE_TEST: {
            ret = GET_PAGE_SIZE(0x400, minSize);
        } break;
        default:
            break;
    }
    return ret;
}

static int getRegion(E_FLASH_TYPE type, FlashRegion *region) {
    if (!ginTotalSize || !ginMinSize) {
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




int lelinkStorageInit(uint32_t startAddr, uint32_t totalSize, uint32_t minSize) {
    int i = 0;
    uint32_t tmpTotal = 0, tmpSize = 0;
    // uint32_t tmpStartAddr = startAddr;
    for (i = 0; i < E_FLASH_TYPE_MAX; i++) {
        tmpTotal += getSize(i, minSize);
    }

    if (tmpTotal > totalSize) {
        return -1;
    }

    ginStartAddr = startAddr;
    ginTotalSize = totalSize;
    ginMinSize = minSize;

    for (i = 0; i < E_FLASH_TYPE_MAX; i++) {
        ginRegion[i].type = i;
        ginRegion[i].size = getSize(i, minSize);
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
    ginMinSize = 0;
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

int lelinkStorageWriteScriptCfg(const ScriptCfg *scriptCfg) {
    int ret = 0;

    ret = storageWrite(E_FLASH_TYPE_SCRIPT, scriptCfg, sizeof(ScriptCfg));

    return ret;
}
int lelinkStorageReadScriptCfg(ScriptCfg *scriptCfg){
    int ret = 0;

    ret = storageRead(E_FLASH_TYPE_SCRIPT, scriptCfg, sizeof(ScriptCfg));

    return ret;
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

int processStdMessage() {
    // 1. parse json
    // 2. std2pri
    // 3. writeUart
    return 0;
}

void *ioInit(int ioType) {
    void *ioHdl = NULL;
    char json[256] = {0};
    int ret = 0;
    switch (ioType) {
        case IO_TYPE_UART: {
            int baud = 0, dataBits = 0, stopBits = 0, flowCtrl = 0, PARITY = 0;
            char parity = 0;
            ret = sengineGetDevProfile(json, sizeof(json));
            if (0 >= ret) {
                LELOGW("ioInit sengineGetDevProfile ret[%d]\r\n", ret);
                return NULL;
            }
            ret = getUartInfo(json, ret, &baud, &dataBits, &stopBits, &parity, &flowCtrl);
            if (0 > ret) {
                LELOGW("ioInit getUartInfo ret[%d]\r\n", ret);
                return NULL;
            }
            if(parity == 'N') // None
                PARITY = 0;
            else if(parity == 'O') // Odd
                PARITY = 1;
            else // Even
                PARITY = 2;
            ioHdl = (void *)halUartOpen(baud, dataBits, stopBits, PARITY, flowCtrl);
            if (NULL == ioHdl) {
                LELOGW("ioInit halUartInit ioHdl[%p]\r\n", ioHdl);
                return NULL;
            }
            return ioHdl;
        }break;
        case IO_TYPE_PIPE: {

        }break;
        case IO_TYPE_SOCKET: {

        }break;
    }
    // halUartInit()
    return NULL;
}

void **ioGetHdl(int ioType) {
    static void *hdlUart = NULL;
    
    switch (ioType) {
        case IO_TYPE_UART: {
            if (NULL == hdlUart) {
                hdlUart = ioInit(ioType);
            }
            return &hdlUart;
        }break;
        case IO_TYPE_PIPE: {

        }break;
        case IO_TYPE_SOCKET: {

        }break;
    }
    // halUartInit()
    return NULL;    
}

int ioWrite(int ioType, void *hdl, const uint8_t *data, int dataLen) {

    switch (ioType) {
        case IO_TYPE_UART: {
            return halUartWrite(hdl, data, dataLen);
        }break;
        case IO_TYPE_PIPE: {

        }break;
        case IO_TYPE_SOCKET: {

        }break;
    }    
    return 0;
}

int ioRead(int ioType, void *hdl, uint8_t *data, int dataLen) {
    switch (ioType) {
        case IO_TYPE_UART: {
            return halUartRead(hdl, data, dataLen);
        }break;
        case IO_TYPE_PIPE: {

        }break;
        case IO_TYPE_SOCKET: {

        }break;
    }    
    return 0;
}

void ioDeinit(int ioType, void *hdl) {
    switch (ioType) {
        case IO_TYPE_UART: {
            void **hdlUart = NULL; 
            hdlUart = ioGetHdl(ioType);
            halUartClose(hdl);
            *hdlUart = NULL;
        }break;
        case IO_TYPE_PIPE: {

        }break;
        case IO_TYPE_SOCKET: {

        }break;
    }    
    // return 0;
}
