#include "leconfig.h"
#include "io.h"
#include "sengine.h"
#include "ota.h"
#include "utility.h"
#include "misc.h"
#include "data.h"

#ifndef LOG_IO
#ifdef LELOG
#undef LELOG
#define LELOG(...)
#endif

#ifdef LELOGW
#undef LELOGW
#define LELOGW(...)
#endif

// #ifdef LELOGE
// #undef LELOGE
// #define LELOGE(...)
// #endif

#ifdef LEPRINTF
#undef LEPRINTF
#define LEPRINTF(...)
#endif
#endif
// #define SECTOR_SIZE ginMinSize // 0x800 // 2KB
// #define BLOCK_SIZE 0x8000 // 32KB


// // 0x1c2000
// #define STORAGE_ADDR_PRIVATE_CFG (ginStartAddr) // 0x1c2000 for Marvell
// #define STORAGE_SIZE_PRIVATE_CFG (GET_ERASE_SIZE(sizeof(PrivateCfg))) // 0X1000

// #define STORAGE_ADDR_AUTH_CFG (ginStartAddr + STORAGE_SIZE_PRIVATE_CFG)
// #define STORAGE_SIZE_AUTH_CFG (GET_ERASE_SIZE(sizeof(AuthCfg))) // 0X1000

// #define STORAGE_ADDR_TEST_CFG (ginStartAddr + STORAGE_SIZE_PRIVATE_CFG + STORAGE_ADDR_AUTH_CFG)
// #define STORAGE_SIZE_TEST_CFG (GET_ERASE_SIZE(26)) // 0X1000


extern void *halFlashOpen(void);
extern int findPosForIAName(PrivateCfg *privCfg, const char *strSelfRuleName, int lenSelfRuleName, int *whereToPut);
extern int resetConfigData(void);
static FlashRegion ginRegion[E_FLASH_TYPE_MAX];
static uint32_t ginStartAddr;
static uint32_t ginTotalSize;
static uint32_t ginMinSize;
static gpioManager_t ginGpioManager;
static pwmManager_t ginPWMManager;
static commonManager_t ginCommonManager;
static uartHandler_t uartHandler;

static void gpioCheckInput(gpioHandler_t *ptr);
static void gpioCheckState(gpioHandler_t *ptr);
static void pwmCheckState(pwmHandler_t *ptr);
static void gpioInitState(gpioManager_t *mgr);

static IOHDL ginIOHdl[] = {
    {IO_TYPE_UART, 0x0},
    {IO_TYPE_GPIO, 0x0},
    {IO_TYPE_PIPE, 0x0},
    {IO_TYPE_PWM, 0x0},
};

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
        case E_FLASH_TYPE_SCRIPT2: {
            ret = GET_PAGE_SIZE(sizeof(ScriptCfg2), minSize);
        } break;
        case E_FLASH_TYPE_SDEV_INFO: {
            ret = GET_PAGE_SIZE(sizeof(SDevInfoCfg), minSize);
        } break;
        default:
            break;
    }
    return ret;
}

int getRegion(E_FLASH_TYPE type, FlashRegion *region) {
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
    uint32_t tmpTotal = 0, singleSize = 0, tmpSize = 0;
    // uint32_t tmpStartAddr = startAddr;
    LELOG("lelinkStorageInit -s\r\n");
    for (i = 0; i < E_FLASH_TYPE_MAX; i++) {
        tmpTotal += getSize((E_FLASH_TYPE)i, minSize);
    }

    if (tmpTotal > totalSize) {
        return -1;
    }

    ginStartAddr = startAddr;
    ginTotalSize = totalSize;
    ginMinSize = minSize;

    for (i = 0; i < E_FLASH_TYPE_MAX; i++) {
        ginRegion[i].type = i;
        singleSize = getSize(i, minSize);
        ginRegion[i].type = (E_FLASH_TYPE)i;
        ginRegion[i].size = getSize((E_FLASH_TYPE)i, minSize);
        LELOG("[%d] [%d]", i, ginRegion[i].size);
        ginRegion[i].addr = ginStartAddr + tmpSize;
        ginRegion[i].size = singleSize;
        if (E_FLASH_TYPE_SCRIPT2 == i) {
            tmpTotal = singleSize*MAX_IA;
        } else {
            tmpTotal = singleSize;
        }
        tmpSize += tmpTotal;
        LELOG("idx[%d] addr[0x%x] size[0x%x*%d=0x%x] type[%d]", i, ginRegion[i].addr, ginRegion[i].size, ginRegion[i].size ? tmpTotal/ginRegion[i].size : 0, tmpTotal, ginRegion[i].type);
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

int getFlashMinSize() {
    return ginMinSize;
}

static int storageWrite(E_FLASH_TYPE type, const void *data, int size, int idx) {
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

    ret = halFlashErase(hdl, fr.addr + (idx*fr.size), fr.size);
    if (0 > ret) {
        return -3;
    }
    // LELOG("flashWritePrivateCfg halFlashErase [0x%x] [0x%x][0x%x]", hdl, fr.addr, fr.size);

    *((uint8_t *)data + (size - 1)) = crc8(data, size - 1);
    ret = halFlashWrite(hdl, data, size, fr.addr + (idx*fr.size), 0);
    if (0 > ret) {
        return -4;
    }
    
    // LELOG("flashWritePrivateCfg halFlashWrite [0x%x] [0x%x][0x%x]", hdl, fr.addr, fr.size);
    halFlashClose(hdl);
    return 0; 
}

static int storageRead(E_FLASH_TYPE type, void *data, int size, int idx) {
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

    ret = halFlashRead(hdl, data, size, fr.addr + (idx*fr.size), 0);
    if (0 > ret) {
        return -3;
    }

    // LELOG("flashReadPrivateCfg [0x%x] [0x%x][0x%x]", hdl, STORAGE_SIZE_PRIVATE_CFG, fr.addr);
    halFlashClose(hdl);

    return 0;
}


int lelinkStorageWriteAuthCfg(const AuthCfg *authCfg) {
    int ret = 0;

    ret = storageWrite(E_FLASH_TYPE_AUTH, authCfg, sizeof(AuthCfg), 0);

    return ret;
}

int lelinkStorageReadAuthCfg(AuthCfg *authCfg) {
    int ret = 0;

    ret = storageRead(E_FLASH_TYPE_AUTH, authCfg, sizeof(AuthCfg), 0);

    return ret;
}

int lelinkStorageWriteScriptCfg(const void *scriptCfg, int flashType, int idx) {
    int ret = 0;
    // char strSelfRuleName[MAX_RULE_NAME] = {0};
    // ScriptCfg *tmpScriptCfg = (ScriptCfg *)scriptCfg;
    int tmpSize = 0;
    if (E_FLASH_TYPE_SCRIPT == flashType) {
        tmpSize = sizeof(ScriptCfg);
    } else if (E_FLASH_TYPE_SCRIPT2 == flashType) {
        tmpSize = sizeof(ScriptCfg2);
    } else 
        return -1;
    ret = storageWrite(flashType, scriptCfg, tmpSize, idx);


    // if (E_FLASH_TYPE_SCRIPT == flashType) {
    //     ret = storageWrite(E_FLASH_TYPE_SCRIPT, scriptCfg, sizeof(ScriptCfg), idx);
    // } else if (E_FLASH_TYPE_SCRIPT2 == flashType) {
    //     PrivateCfg privCfg;
    //     // write fw script
    //     ret = storageWrite(E_FLASH_TYPE_SCRIPT2, scriptCfg, sizeof(ScriptCfg), idx);

    //     // update private
    //     ret = lelinkStorageReadPrivateCfg(&privCfg);
    //     if (privCfg.csum != crc8((const uint8_t *)&(privCfg.data), sizeof(privCfg.data))) {
    //         LELOGW("lelinkStorageReadPrivateCfg csum failed");
    //         return -1;
    //     }

    //     ret = sengineCall((const char *)tmpScriptCfg->data.script, tmpScriptCfg->data.size, S2_GET_SELFNAME,
    //         NULL, 0, (uint8_t *)&strSelfRuleName, sizeof(strSelfRuleName));
    //     if (0 > ret) {
    //         LELOGW("senginePollingRules sengineCall("S2_GET_SELFNAME") [%d]", ret);
    //         return -2;
    //     }


    //     privCfg.data.iaCfg.arrIA[idx] = 1;
    //     memcpy(&(privCfg.data.iaCfg.arrIAName[idx]), strSelfRuleName, ret);
    //     for (i = 0; i < MAX_IA; i++) {
    //         if (0 < privCfg.data.iaCfg.arrIA[i]) {
    //             tmpNum++;
    //         }
    //     }
    //     privCfg.data.iaCfg.num = tmpNum;
    //     lelinkStorageWritePrivateCfg(&privCfg);
    // }

    return ret;
}

int lelinkStorageReadScriptCfg(void *scriptCfg, int flashType, int idx){
    int ret = 0;

    if (E_FLASH_TYPE_SCRIPT == flashType) {
        ret = storageRead(E_FLASH_TYPE_SCRIPT, scriptCfg, sizeof(ScriptCfg), idx);
    } else if (E_FLASH_TYPE_SCRIPT2 == flashType) {
        ret = storageRead(E_FLASH_TYPE_SCRIPT2, scriptCfg, sizeof(ScriptCfg2), idx);
    } else {
        LELOGW("lelinkStorageReadScriptCfg not supported flashType[%d]", flashType);
        return -1;
    }

    return ret;
}

int lelinkStorageWriteScriptCfg2(const void *scriptCfg) {
    int ret = 0, lenSelfRuleName = 0, whereToPut = -1, found = 0;
    PrivateCfg privCfg;
    char strSelfRuleName[MAX_RULE_NAME] = {0};
    LELOG("lelinkStorageWriteScriptCfg2 -s ");

    lenSelfRuleName = sengineCall((const char *)((ScriptCfg2 *)scriptCfg)->data.script, ((ScriptCfg2 *)scriptCfg)->data.size, S2_GET_SELFNAME,
        NULL, 0, (uint8_t *)&strSelfRuleName, sizeof(strSelfRuleName));
    if (0 > lenSelfRuleName) {
        LELOGW("lelinkStorageWriteScriptCfg2 sengineCall("S2_GET_SELFNAME") [%d]", lenSelfRuleName);
        return -1;
    }

    ret = lelinkStorageReadPrivateCfg(&privCfg);
    if (0 > ret || privCfg.csum != crc8((const uint8_t *)&(privCfg.data), sizeof(privCfg.data))) {
        LELOGW("lelinkStorageWriteScriptCfg2 csum FAILED");
        return -2;
    }


    found = findPosForIAName(&privCfg, strSelfRuleName, lenSelfRuleName, &whereToPut);

    if (!found && 0 > whereToPut) {
        LELOGW("lelinkStorageWriteScriptCfg2 IA(s) are FULL");
        return -3;
    }

    // comming a new ia item
    if (!found && 0 <= whereToPut) {
        privCfg.data.iaCfg.arrIA[whereToPut] = 1;
        if (0 > privCfg.data.iaCfg.num)
            privCfg.data.iaCfg.num = 1;
        else
            privCfg.data.iaCfg.num++;
    }
    memcpy(privCfg.data.iaCfg.arrIAName[whereToPut], strSelfRuleName, lenSelfRuleName);
    ret = lelinkStorageWritePrivateCfg(&privCfg);
    if (0 > ret) {
        LELOGW("lelinkStorageWriteScriptCfg2 lelinkStorageWritePrivateCfg FAILED [%d]", ret);
        return -4;
    }

    ret = lelinkStorageWriteScriptCfg(scriptCfg, E_FLASH_TYPE_SCRIPT2, whereToPut);
    if (0 > ret) {
        // a new ia item
        if (!found && 0 <= whereToPut) {
            privCfg.data.iaCfg.arrIA[whereToPut] = -1;
            privCfg.data.iaCfg.num--;
            lelinkStorageWritePrivateCfg(&privCfg);
        }
        LELOGW("lelinkStorageWriteScriptCfg2 lelinkStorageWriteScriptCfg FAILED [%d]", ret);
        return -5;
    }
    LELOG("lelinkStorageWriteScriptCfg2 found[%d] where[%d/%d]-e ", found, whereToPut, privCfg.data.iaCfg.num);
    return 0;
}

// static uint8_t ginIsPrivateCfgChanged = 1;
// static PrivateCfg ginPrivateCfg;
int lelinkStorageWritePrivateCfg(const PrivateCfg *privateCfg) {
    int ret = 0;

    ret = storageWrite(E_FLASH_TYPE_PRIVATE, privateCfg, sizeof(PrivateCfg), 0);

    return ret;
}

int lelinkStorageReadPrivateCfg(PrivateCfg *privateCfg) {
    int ret = 0;

    ret = storageRead(E_FLASH_TYPE_PRIVATE, privateCfg, sizeof(PrivateCfg), 0);

    return ret;
}

int lelinkStorageWriteSDevInfoCfg(const void *sdevArr) {
    int ret = 0, written = 0, i = 0;
    FlashRegion fr;
    uint8_t csum = 0;
    const SDevNode *arr = (SDevNode *)sdevArr; 
    if (NULL == arr) {
        return -1;
    }

    ret = getRegion(E_FLASH_TYPE_SDEV_INFO, &fr);
    if (0 > ret) {
        return -2;
    }

    void *hdl = (void *)halFlashOpen();
    if (NULL == hdl) {
        return -3;
    }

    ret = halFlashErase(hdl, fr.addr, fr.size);
    if (0 > ret) {
        return -4;
    }

    for (i = 0; i < MAX_SDEV_NUM; i++) {
        SDevNBase *base = (SDevNBase *)&(arr[i]);
        ret = halFlashWrite(hdl, (const uint8_t *)base, sizeof(SDevNBase), fr.addr, i*sizeof(SDevNBase));
        if (0 > ret) {
            halFlashClose(hdl);
            return -5;
        }
        csum += crc8((const uint8_t *)base, sizeof(SDevNBase));
        written += sizeof(SDevNBase);
    }
    ret = halFlashWrite(hdl, (const uint8_t *)&csum, 1, fr.addr, i*sizeof(SDevNBase));
    if (0 > ret) {
        halFlashClose(hdl);
        return -6;
    }
    LELOG("lelinkStorageWriteSDevInfoCfg [0x%x] [0x%x][0x%x]", csum, fr.addr, fr.size);
    halFlashClose(hdl);

    return written;
}

int lelinkStorageReadSDevInfoCfg(void *sdevArr) {
    int ret = 0, read = 0, i = 0;
    FlashRegion fr;
    uint8_t csum = 0, tmp = 0;
    void * hdl;
    SDevNode *arr = (SDevNode *)sdevArr; 
    if (NULL == arr) {
        return -1;
    }

    ret = getRegion(E_FLASH_TYPE_SDEV_INFO, &fr);
    if (0 > ret) {
        return -2;
    }

    hdl = (void *)halFlashOpen();
    if (NULL == hdl) {
        return -3;
    }

    for (i = 0; i < MAX_SDEV_NUM; i++) {
        SDevNBase *base = (SDevNBase *)&(arr[i]);
        ret = halFlashRead(hdl, (uint8_t *)base, sizeof(SDevNBase), fr.addr, i*sizeof(SDevNBase));
        if (0 > ret) {
            halFlashClose(hdl);
            return -4;
        }
        csum += crc8((const uint8_t *)base, sizeof(SDevNBase));
        read += sizeof(SDevNBase);
    }
    ret = halFlashRead(hdl, (uint8_t *)&tmp, 1, fr.addr, i*sizeof(SDevNBase));
    if (0 > ret) {
        halFlashClose(hdl);
        return -5;
    }
    if (csum != tmp) {
        halFlashClose(hdl);
        for (i = 0; i < MAX_SDEV_NUM; i++) {
            SDevNBase *base = (SDevNBase *)&(arr[i]);
            memset(base, 0, sizeof(SDevNBase));
        }
        return -6;
    }

    LELOG("lelinkStorageReadSDevInfoCfg [0x%x] [0x%x][0x%x]", csum, fr.addr, fr.size);
    halFlashClose(hdl);


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
//     LELOG("flashWritePrivateCfg halFlashErase [0x%x] [0x%x][0x%x]", hdl, STORAGE_ADDR_PRIVATE_CFG, STORAGE_SIZE_PRIVATE_CFG);

//     memcpy(buf, privateCfg, sizeof(PrivateCfg));
//     ret = halFlashWrite(hdl, buf, sizeof(buf), STORAGE_ADDR_PRIVATE_CFG);
//     if (0 > ret) {
//         return -3;
//     }
//     LELOG("flashWritePrivateCfg halFlashWrite [0x%x] [0x%x][0x%x]", hdl, STORAGE_ADDR_PRIVATE_CFG, STORAGE_SIZE_PRIVATE_CFG);
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
//     LELOG("flashReadPrivateCfg [0x%x] [0x%x][0x%x]", hdl, STORAGE_SIZE_PRIVATE_CFG, STORAGE_ADDR_PRIVATE_CFG);
//     halFlashClose(hdl);

//     return 0;
// }

int processStdMessage() {
    // 1. parse json
    // 2. std2pri
    // 3. writeUart
    return 0;
}

static void gpioSetDefault(gpioManager_t *mgr)
{
    int i;
    gpioHandler_t *table;

    if(!mgr && !mgr->table) {
        return;
    }
    table = mgr->table;
    memset(table, 0, sizeof(*table) * GPIO_MAX_ID);
    for( i = 0; i < GPIO_MAX_ID; i++ ) {
        table[i].id = 0;
        table[i].dir = GPIO_DIR_INPUT; 
        table[i].mode = GPIO_MODE_DEFAULT;
        table[i].state = GPIO_STATE_LOW;
        table[i].oldState = 0;
        table[i].type = 0;
        table[i].gpiostate = 0;
        table[i].freestate = GPIO_STATE_LOW;
        table[i].blink = 0;
        table[i].longTime = 5;
        table[i].shortTime = 0;
        table[i].keepLowTimes = 0;
        table[i].keepHighTimes = 0;
        table[i].handler = NULL;
        table[i].reserved1 = 0;
        table[i].reserved2 = 0;
    }
}

static void pwmSetDefault(pwmManager_t *mgr) {
    int i;
    pwmHandler_t *table;

    if(!mgr && !mgr->table) {
        return;
    }
    table = mgr->table;
    memset(table, 0, sizeof(*table) * PWM_MAX_ID);
    for( i = 0; i < PWM_MAX_ID; i++ ) {
        table[i].id = 0;
        table[i].type = 0;
        table[i].state = 0;
        table[i].oldState = 0;
        table[i].longTime = 0;
        table[i].shortTime = 0;
        table[i].handler = NULL;
        table[i].reserved1 = 0;
        table[i].reserved2 = 0;
    }
}

void *ioInit(int ioType, const char *json, int jsonLen) {
    int ret = 0;
    void *ioHdl = NULL;

    LELOG("ioInt type[%d]\r\n", ioType);

    switch (ioType) {
        case IO_TYPE_UART: {
            ret = getUartInfo(json, jsonLen, &uartHandler);
            if (0 > ret) {
                LELOGW("ioInit getUartInfo ret[%d]", ret);
                return NULL;
            }
            if(uartHandler.parity == 'N') // None
                uartHandler.parity = 0;
            else if(uartHandler.parity == 'O') // Odd
                uartHandler.parity = 1;
            else // Even
                uartHandler.parity = 2;
            ioHdl = halUartOpen(&uartHandler);
            return ioHdl;
        }break;
        case IO_TYPE_GPIO: {
            int i;
            gpioHandler_t *table;
            gpioSetDefault(&ginGpioManager);
            ret = getGPIOInfo(json, jsonLen, ginGpioManager.table, GPIO_MAX_ID);
            if (0 >= ret) {
                LELOGW("ioInit getGPIOInfo ret[%d]", ret);
                return NULL;
            }
            ginGpioManager.num = ret;
            LELOGW("ioInit ginGpioManager.num ret[%d]", ginGpioManager.num);
            halGPIOInit();
            for(i = 0, table = ginGpioManager.table; i < ginGpioManager.num; i++, table++) {
                halGPIOOpen(table);
            }
            ioHdl = &ginGpioManager;
            gpioInitState(&ginGpioManager);
            return ioHdl;
        }break;
        case IO_TYPE_PIPE: {
            char name[64];
            ret = getPipeInfo(json, jsonLen, name, sizeof(name));
            if (0 > ret) {
                LELOGW("ioInit getPipeInfo ret[%d]", ret);
                return NULL;
            }
            ioHdl = (void *)halPipeOpen(name);
            if (NULL == ioHdl) {
                LELOGW("ioInit halPipeOpen[%p]", ioHdl);
                return NULL;
            }
            return ioHdl;
        }break;
        case IO_TYPE_SOCKET: {

        }break;
        case IO_TYPE_PWM: {
            int i;
            pwmHandler_t *table;
            pwmSetDefault(&ginPWMManager);
            ret = getPWMInfo(json, jsonLen, ginPWMManager.table, PWM_MAX_ID);
            if (0 >= ret) {
                LELOGW("ioInit getPWMInfo ret[%d]", ret);
                return NULL;
            }
            ginPWMManager.num = ret;
            halPWMInit(ginPWMManager.table->clock);
            for(i = 0, table = ginPWMManager.table; i < ginPWMManager.num; i++, table++) {
                halPWMSetFrequency(table);
                halPWMOpen(table);
                halPWMWrite(table, table->state);
            }
            ioHdl = &ginPWMManager;
            return ioHdl;
        }break;
    }
    // halUartInit()
    return NULL;
}

static void gpioInitState(gpioManager_t *mgr)
{
    int i;
    gpioHandler_t *table = mgr->table;

    for(i = 0; i < mgr->num; i++, table++) {
        LELOG("IO id = %d, dir = %d, mode = %d, state = %d, type = %d, blink = %d", 
                table->id, table->dir, table->mode, table->state, table->type, table->blink);
        table->freestate = table->state;
        halGPIOWrite(table, !!table->state);
    }
}

void **ioGetHdl(int *ioType) {
    static void *ioHdl = NULL;
    char json[2560] = {0};
    int ret = 0;
    static int whatCvtType = -1;
    LELOGW("ioGetHdl => ioGetHdl[%d]", whatCvtType);
    if (-1 == whatCvtType) {
        ret = sengineGetTerminalProfileCvtType(json, sizeof(json));
        if (0 >= ret) {
            LELOGW("ioGetHdl sengineGetTerminalProfileCvtType ret[%d]", ret);
            return NULL;
        }
        whatCvtType = getWhatCvtType(json, ret);
        LELOG("getWhatCvtType [%d] [%d]", ret, whatCvtType);
        if (0 > whatCvtType) {
            return NULL;
        }
    }
    memset(&ginCommonManager, 0, sizeof(commonManager_t));
    getCommonInfo(json, sizeof(json), &ginCommonManager, COMMON_MAX_ID);
    if(ginCommonManager.num > 0) {
        LELOG("halCommonInit ioInit [%d]", ginCommonManager.num);
        halCommonInit(&ginCommonManager);
    }

    ioType ? *(ioType) = whatCvtType : 0;
    switch (whatCvtType) {
        case IO_TYPE_UART: {
            if (NULL == ioHdl) {
                ioHdl = ioInit(whatCvtType, json, ret);
            }
            return &ioHdl;
        }break;
        case IO_TYPE_GPIO: {
            if (NULL == ioHdl) {
                ioHdl = ioInit(whatCvtType, json, ret);
            }
            return &ioHdl;
        }break;
        case IO_TYPE_PIPE: {
            if (NULL == ioHdl) {
                ioHdl = ioInit(whatCvtType, json, ret);
            }
            return &ioHdl;
        }break;
        case IO_TYPE_SOCKET: {

        }break;
        case IO_TYPE_PWM: {
            if (NULL == ioHdl) {
                ioHdl = ioInit(whatCvtType, json, ret);
            }
            return &ioHdl;
        }break;
    }
    // halUartInit()
    return NULL;    
}

#define IO_INIT_START { \
    int x = 0;

#define IO_INIT_ITEM(i, w, j, jl) \
    if (i == (i & w)) { \
        ginIOHdl[x].hdl = ioInit((i & w), j, jl); \
        ginIOHdl[x].ioType = (i & w); \
    } \
    x++;

#define IO_INIT_END \
    }

IOHDL *ioGetHdlExt() {
    // IOHDL *ioHdl = NULL;
    char json[MAX_BUF] = {0};
    int ret = 0;
    // int x = 0;
    static uint8_t whatCvtType = 0x00;
    if (0x00 == whatCvtType) {
        ret = sengineGetTerminalProfileCvtType(json, sizeof(json));
        if (0 >= ret) {
            LELOG("ioGetHdl sengineGetTerminalProfileCvtType ret[%d]", ret);
            return NULL;
        }
        whatCvtType = getWhatCvtType(json, ret);
        if (0 >= whatCvtType) {
            // LELOGW("ioGetHdl getWhatCvtType[%d] NO VALID TYPE !!!!!!!!! ", whatCvtType);
            return NULL;
        }
    } else {
        return ginIOHdl;
    }
    LELOGW("ioGetHdlExt => whatCvtType[%d] [%d]", ret, whatCvtType);
    memset(&ginCommonManager, 0, sizeof(commonManager_t));
    getCommonInfo(json, ret, &ginCommonManager, COMMON_MAX_ID);
    if(ginCommonManager.num > 0) {
        LELOG("halCommonInit ioInit [%d]", ginCommonManager.num);
        halCommonInit(&ginCommonManager);
    }

    IO_INIT_START;
    IO_INIT_ITEM(IO_TYPE_UART, whatCvtType, json, ret);
    IO_INIT_ITEM(IO_TYPE_GPIO, whatCvtType, json, ret);
    IO_INIT_ITEM(IO_TYPE_PIPE, whatCvtType, json, ret);
    IO_INIT_ITEM(IO_TYPE_PWM, whatCvtType, json, ret);
    IO_INIT_END;

    // {
    //     int i = 0;
    //     for (i = 0; i < ioGetHdlCounts(); i++) {
    //         LELOGE("ioType[%d], hdl[%p]", ginIOHdl[i].ioType, ginIOHdl[i].hdl);
    //     }
    // }

    return ginIOHdl;
}

int ioGetHdlCounts() {
    return sizeof(ginIOHdl)/sizeof(IOHDL);
}

int ioWrite(int ioType, void *hdl, const uint8_t *data, int dataLen) {
    //LELOG("ioWrite ioType[%d] [%d]", ioType, dataLen);
    switch (ioType) {
        case IO_TYPE_UART: {
            return halUartWrite(hdl, data, dataLen);
        }break;
        case IO_TYPE_GPIO: {
            int i,j,ret = 0;
            gpioManager_t *mgr = ((gpioManager_t *)hdl);
            gpioHandler_t *q = NULL;
            //LELOG("ioWrite gpio count [%d][%d]", mgr->num, dataLen);
            for(i = 0; i < dataLen ; i+=2) {
                q = mgr->table;
                for(j = 0; j< mgr->num; j++, q++) {
                    if(data[i] == q->id) {
                        halGPIOWrite(q, data[i+1]);
                        ret = 1;
                    }
                }
            }
            return ret;
        }break;
        case IO_TYPE_PIPE: {
            return halPipeWrite(hdl, data, dataLen);
        }break;
        case IO_TYPE_SOCKET: {

        }break;
        case IO_TYPE_PWM: {
            int i,j,ret = 0, val = 0;
            pwmManager_t *mgr = ((pwmManager_t *)hdl);
            pwmHandler_t *q = NULL;
            //LELOG("ioWrite pwm count [%d][%d]", mgr->num, dataLen);
            for(i = 0; i < dataLen ; i+=3) {
                q = mgr->table;
                for(j = 0; j< mgr->num; j++,q++) {
                    if(data[i] == q->id) {
                        val = data[i+1];
                        val = ((val << 8) | data[i+2]);
                        //LELOG("ioWrite halPWMWrite [%d][%d]", q->id, val);
                        halPWMWrite(q, val);
                        ret = 1;
                    }
                }
            }
            return ret;
        }break;
    }
    return 0;
}

int ioRead(int ioType, void *hdl, uint8_t *data, int dataLen) {
    int ret = 0;
    switch (ioType) {
        case IO_TYPE_UART: {
            return halUartRead(hdl, data, dataLen);
        }break;
        case IO_TYPE_GPIO: {
            int val, i, k = 0;
            gpioManager_t *mgr = ((gpioManager_t *)hdl);
            gpioHandler_t *q = mgr->table;
            for(i = 0; i < mgr->num; i++, q++) {
                halGPIORead(q, &val);
                if(val == q->state) {
                    q->keepHighTimes = 0;
                } else if(q->type == GPIO_TYPE_INPUT_RESET && 
                    (q->dir == GPIO_DIR_INPUT) && (q->oldState == val)) {
                    gpioCheckInput(q);
                }
                data[k++] = q->id;
                data[k++] = val;
                if(q->oldState != val && q->oldState != q->state) {
                    ret = mgr->num*(1 + 1); // id + val
                }
                q->oldState = val;
                if(q->type == GPIO_TYPE_OUTPUT_RESET && (q->dir == GPIO_DIR_OUTPUT)) {
                    gpioCheckState(q);
                }
            }
            return ret;
        } break;
        case IO_TYPE_PIPE: {
            return halPipeRead(hdl, data, dataLen);
        }break;
        case IO_TYPE_SOCKET: {

        }break;
        case IO_TYPE_PWM: {
            int i, k=0, val;
            pwmManager_t *mgr = ((pwmManager_t *)hdl);
            pwmHandler_t *q = mgr->table;
            for(i = 0; i < mgr->num ; i++, q++) {
                halPWMRead(q, (uint32_t*)&val);
                //LELOG("ioWrite ioRead [%d][%d][%d]", q->id, val, q->oldState);
                data[k++] = q->id;
                data[k++] = ((val >> 8) & 0xFF);
                data[k++] = (val & 0xFF);
                if(q->oldState != val) {
                    ret = mgr->num*(1 + 2); // id + percent
                    q->oldState = val;
                }
                if(q->type == PWM_TYPE_OUTPUT_RESET) {
                    pwmCheckState(q);
                }
            }
            //LELOG("ioRead [%d][%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x]", k, data[0], data[1], data[2], 
            //    data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11]);
            return ret;
        }break;
    }    
    return 0;
}

static RLED_STATE_t s_resetLevel = RLED_STATE_FREE;
RLED_STATE_t setResetLed(RLED_STATE_t st)
{
    LELOG("setResetLed state [%d]",st);
    if(st >= RLED_STATE_FREE && st <= RLED_STATE_RUNNING) {
        s_resetLevel = st;
    }
    return s_resetLevel;
}

static void gpioCheckInput(gpioHandler_t *ptr) {
    ptr->keepHighTimes++;
    LELOG("gpioCheckInput [%d] [%d] id[%d]",ptr->keepHighTimes, ptr->longTime, ptr->id);
    if(ptr->keepHighTimes >= ptr->longTime) {
        int ret = resetConfigData();
        LELOG("resetConfigData [%d]", ret);
        if (0 <= ret) {
            setDevFlag(DEV_FLAG_RESET, 1);
            halReboot();
        }
    }
}

static void pwmCheckState(pwmHandler_t *ptr) {
    static uint32_t count = 0;
    uint32_t val = 0;
    if(ptr->type == PWM_TYPE_OUTPUT_RESET) {
        halPWMRead(ptr, &val);
        //LELOG("pwmCheckState duty [%d] [%d] [%d]",ptr->duty, count,ptr->state);
        if(s_resetLevel > RLED_STATE_FREE) {
            count++;
            if(s_resetLevel == RLED_STATE_WIFI) {
                if(count >= ptr->shortTime) {
                    if(val)
                        halPWMWrite(ptr, 0);
                    else
                        halPWMWrite(ptr, ptr->duty);
                    count = 0;
                }    
            } else if(s_resetLevel == RLED_STATE_CONNECTING) {
                if(count >= ptr->longTime) {
                    if(val)
                        halPWMWrite(ptr, 0);
                    else
                        halPWMWrite(ptr, ptr->duty);
                }   
            } else if(s_resetLevel == RLED_STATE_RUNNING) {
                if (count >= ptr->longTime) {
                    halPWMWrite(ptr, ptr->state);
                    count = 0;
                }
            } else {
                count = 0;
                LELOG("pwmCheckState error\n");
            }
        } else {
            halPWMWrite(ptr, ptr->state ? 0 : ptr->duty);
            count = 0;
        }
    }
}

static void gpioCheckState(gpioHandler_t *ptr) {
    static uint32_t count = 0;
    int val = 0;
    halGPIORead(ptr, &val);
    if(s_resetLevel > RLED_STATE_FREE) {
        count++;
        if(s_resetLevel == RLED_STATE_WIFI) {
            if(count >= ptr->shortTime) {
                halGPIOWrite(ptr, !val);
                count = 0;
            }
        } else if(s_resetLevel == RLED_STATE_CONNECTING) {
            if(count >= ptr->longTime) {
                halGPIOWrite(ptr, !val);
                count = 0;
            }
        } else if(s_resetLevel == RLED_STATE_RUNNING) {
            if (val != ptr->state) {
                halGPIOWrite(ptr, ptr->state);
            }
        } else {
            count = 0;
            LELOG("gpioCheckState error\n");
        }
    } else {
        // LELOG("gpioCheckState val[%d] state[%d]", val, ptr->state);
        if (val == !!ptr->state) {
            halGPIOWrite(ptr, !ptr->state);
        }
        count = 0;
    }
}

void ioDeinit(int ioType, void *hdl) {
    int i;
    switch (ioType) {
        case IO_TYPE_UART: {
            void **hdlUart = NULL; 
            hdlUart = ioGetHdl(NULL);
            halUartClose(hdl);
            *hdlUart = NULL;
        }break;
        case IO_TYPE_GPIO: {
            void **hdlGPIO = NULL; 
            hdlGPIO = ioGetHdl(NULL);
            gpioManager_t *mgr = ((gpioManager_t *)hdlGPIO);
            gpioHandler_t *q = mgr->table;
            for(i = 0; i < mgr->num ; i++, q++) {
                halGPIOClose(q);
            }
            *hdlGPIO = NULL;
        }break;
        case IO_TYPE_PIPE: {
            void **hdlPipe = NULL; 
            hdlPipe = ioGetHdl(NULL);
            halPipeClose(hdl);
            *hdlPipe = NULL;
        }break;
        case IO_TYPE_SOCKET: {

        }break;
        case IO_TYPE_PWM: {
            void **hdlPWM = NULL; 
            hdlPWM = ioGetHdl(NULL);
            pwmManager_t *mgr = ((pwmManager_t *)hdlPWM);
            pwmHandler_t *q = mgr->table;
            for(i = 0; i < mgr->num ; i++, q++) {
                halPWMClose(q);
            }
            *hdlPWM = NULL;
        }break;
    }    
    // return 0;
}
