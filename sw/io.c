#include "leconfig.h"
#include "io.h"
#include "sengine.h"
#include "ota.h"

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

static FlashRegion ginRegion[E_FLASH_TYPE_MAX];
static uint32_t ginStartAddr;
static uint32_t ginTotalSize;
static uint32_t ginMinSize;
static gpioManager_t ginGpioManager;
static IOHDL ginIOHdl[] = {
    {IO_TYPE_UART, 0x0},
    {IO_TYPE_GPIO, 0x0},
    {IO_TYPE_PIPE, 0x0},
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
            ret = GET_PAGE_SIZE(sizeof(ScriptCfg), minSize);
        } break;
        case E_FLASH_TYPE_TEST: {
            ret = GET_PAGE_SIZE(0x400, minSize);
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
    uint32_t tmpTotal = 0, tmpSize = 0;
    // uint32_t tmpStartAddr = startAddr;
    LELOG("lelinkStorageInit -s\r\n");
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
        // LELOG("[%d] [%d]", i, ginRegion[i].size);
        ginRegion[i].addr = ginStartAddr + tmpSize;
        tmpSize += ginRegion[i].size;
        LELOG("idx[%d] addr[0x%x] size[0x%x] type[%d]", i, ginRegion[i].addr, ginRegion[i].size, ginRegion[i].type);
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
    ret = halFlashWrite(hdl, data, size, fr.addr + (idx*fr.size));
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

    ret = halFlashRead(hdl, data, size, fr.addr + (idx*fr.size));
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

    ret = storageWrite(flashType, scriptCfg, sizeof(ScriptCfg), idx);


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
        ret = storageRead(E_FLASH_TYPE_SCRIPT2, scriptCfg, sizeof(ScriptCfg), idx);
    } else {
        LELOGW("lelinkStorageReadScriptCfg not supported flashType[%d]", flashType);
        return -1;
    }

    return ret;
}

int lelinkStorageWriteScriptCfg2(const ScriptCfg *scriptCfg) {
    int i = 0, ret = 0, lenSelfRuleName = 0, whereToPut = -1, isNew = 0;
    PrivateCfg privCfg;
    char strSelfRuleName[MAX_RULE_NAME] = {0};
    LELOG("lelinkStorageWriteScriptCfg2 -s ");

    lenSelfRuleName = sengineCall((const char *)scriptCfg->data.script, scriptCfg->data.size, S2_GET_SELFNAME,
        NULL, 0, (uint8_t *)&strSelfRuleName, sizeof(strSelfRuleName));
    if (0 > lenSelfRuleName) {
        LELOGW("lelinkStorageWriteScriptCfg2 sengineCall("S2_GET_SELFNAME") [%d]", lenSelfRuleName);
        return -1;
    }

    ret = lelinkStorageReadPrivateCfg(&privCfg);
    if (privCfg.csum != crc8((const uint8_t *)&(privCfg.data), sizeof(privCfg.data))) {
        LELOGW("lelinkStorageWriteScriptCfg2 csum FAILED");
        return -2;
    }

    for (i = MAX_IA - 1; i > -1; i--) {
        if (0 < privCfg.data.iaCfg.arrIA[i]) {
            if (0 == memcmp(strSelfRuleName, privCfg.data.iaCfg.arrIAName[i], lenSelfRuleName)) {
                whereToPut = i;
                isNew = 0;
                break;
            }
        } else {
            whereToPut = i;
            isNew = 1;
        }
    }

    if (-1 == i && !isNew) {
        LELOGW("lelinkStorageWriteScriptCfg2 IA(s) are FULL");
        return -3;
    }

    // comming a new ia item
    if (isNew) {
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
        if (isNew) {
            privCfg.data.iaCfg.arrIA[whereToPut] = -1;
            privCfg.data.iaCfg.num--;
            lelinkStorageWritePrivateCfg(&privCfg);
        }
        LELOGW("lelinkStorageWriteScriptCfg2 lelinkStorageWriteScriptCfg FAILED [%d]", ret);
        return -5;
    }
    LELOG("lelinkStorageWriteScriptCfg2 isNew[%d] where[%d/%d]-e ", isNew, whereToPut, privCfg.data.iaCfg.num);
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
    gpioHand_t *table;

    if(!mgr && !mgr->table) {
        return;
    }
    table = mgr->table;
    memset(table, 0, sizeof(*table) * GPIO_MAX_ID);
    for( i = 0; i < GPIO_MAX_ID; i++ ) {
        table[i].id = 0;
        table[i].num = -1;
        table[i].dir = GPIO_DIR_INPUT; 
        table[i].mode = GPIO_MODE_DEFAULT;
        table[i].state = GPIO_STATE_LOW;
        table[i].type = 0;
        table[i].gpiostate = 0;
        table[i].freestate = GPIO_STATE_LOW;
        table[i].blink = 0;
        table[i].longTime = 0;
        table[i].shortTime = 0;
        table[i].keepLowTimes = 0;
        table[i].keepHighTimes = 0;
        table[i].reserved = 0;
    }
}

static void gpioInitState(gpioManager_t *mgr);
void *ioInit(int ioType, const char *json, int jsonLen) {
    int ret = 0;
    void *ioHdl = NULL;

    switch (ioType) {
        case IO_TYPE_UART: {
            int baud = 0, dataBits = 0, stopBits = 0, flowCtrl = 0, PARITY = 0;
            char parity = 0;
            ret = getUartInfo(json, jsonLen, &baud, &dataBits, &stopBits, &parity, &flowCtrl);
            if (0 > ret) {
                LELOGW("ioInit getUartInfo ret[%d]", ret);
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
                LELOGW("ioInit halUartInit halUartOpen[%p]", ioHdl);
                return NULL;
            }
            return ioHdl;
        }break;
        case IO_TYPE_GPIO: {
            int i;
            gpioHand_t *table;
            gpioSetDefault(&ginGpioManager);
            ret = getGPIOInfo(json, jsonLen, ginGpioManager.table, GPIO_MAX_ID);
            if (0 >= ret) {
                LELOGW("ioInit getGPIOInfo ret[%d]", ret);
                return NULL;
            }
            ginGpioManager.num = ret;
            if(!(ginGpioManager.handle = (void *)halGPIOInit())) {
                LELOGW("ioInit halGPIOInit");
                return NULL;
            }
            for(i = 0, table = ginGpioManager.table; i < ginGpioManager.num; i++, table++) {
                table->num =  halGPIOOpen(table->id, table->dir, table->mode);
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
    }
    // halUartInit()
    return NULL;
}

static void gpioInitState(gpioManager_t *mgr)
{
    int i;
    gpioHand_t *table = mgr->table;

    for(i = 0; i < mgr->num; i++, table++) {
        if(table->num < 0) {
            continue;
        }
        LELOGE("IO id = %d, num = %d, dir = %d, mode = %d, state = %d, type = %d, blink = %d", 
                table->id, table->num, table->dir, table->mode, table->state, table->type, table->blink);
        table->freestate = table->state;
        halGPIOWrite(mgr->handle, table->num, !!table->state);
    }
}

void **ioGetHdl(int *ioType) {
    static void *ioHdl = NULL;
    char json[2560] = {0};
    int ret = 0;
    static int whatCvtType = -1;
    if (-1 == whatCvtType) {
        ret = sengineGetTerminalProfileCvtType(json, sizeof(json));
        if (0 >= ret) {
            LELOGW("ioGetHdl sengineGetTerminalProfileCvtType ret[%d]", ret);
            return NULL;
        }
        whatCvtType = getWhatCvtType(json, ret);
        if (0 > whatCvtType) {
            return NULL;
        }
    }
    ioType ? *(ioType) = whatCvtType : 0;
    switch (whatCvtType) {
        case IO_TYPE_UART: {
            if (NULL == ioHdl) {
                LELOG("ioGetHdl IO_TYPE_UART ioInit [%d]", whatCvtType);
                ioHdl = ioInit(whatCvtType, json, ret);
            }
            return &ioHdl;
        }break;
        case IO_TYPE_GPIO: {
            if (NULL == ioHdl) {
                LELOG("ioGetHdl IO_TYPE_GPIO ioInit 2 [%d]", whatCvtType);
                ioHdl = ioInit(whatCvtType, json, ret);
            }
            return &ioHdl;
        }break;
        case IO_TYPE_PIPE: {
            if (NULL == ioHdl) {
                LELOG("ioGetHdl IO_TYPE_PIPE ioInit [%d]", whatCvtType);
                ioHdl = ioInit(whatCvtType, json, ret);
            }
            return &ioHdl;
        }break;
        case IO_TYPE_SOCKET: {

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
            LELOGW("ioGetHdl sengineGetTerminalProfileCvtType ret[%d]", ret);
            return NULL;
        }
        whatCvtType = getWhatCvtType(json, ret);
        if (0 >= whatCvtType) {
            LELOGW("ioGetHdl getWhatCvtType[%d] NO VALID TYPE !!!!!!!!! ", whatCvtType);
            return NULL;
        }
    } else {
        return ginIOHdl;
    }
    // LELOGW("ioGetHdlExt => whatCvtType[%d]", whatCvtType);

    IO_INIT_START;
    IO_INIT_ITEM(IO_TYPE_UART, whatCvtType, json, ret);
    IO_INIT_ITEM(IO_TYPE_GPIO, whatCvtType, json, ret);
    IO_INIT_ITEM(IO_TYPE_PIPE, whatCvtType, json, ret);
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

    switch (ioType) {
        case IO_TYPE_UART: {
            return halUartWrite(hdl, data, dataLen);
        }break;
        case IO_TYPE_GPIO: {
            gpioHand_t *p;
            int i, j, id, val;
            gpioManager_t *mgr = ((gpioManager_t *)hdl);
            for( i = 0; i < dataLen; i++ ) {
                id = (data[i] >> 4) & 0xF;
                val = (data[i] & 0xF);
                for(j = 0, p = mgr->table; j < mgr->num; j++, p++) {
                    if(id == p->id && p->dir == GPIO_DIR_OUTPUT) {
                        p->state = val;
                        break;
                    }
                }
            }
            return dataLen;
        }break;
        case IO_TYPE_PIPE: {
            return halPipeWrite(hdl, data, dataLen);
        }break;
        case IO_TYPE_SOCKET: {

        }break;
    }    
    return 0;
}

static void gpioCheckState(gpioManager_t *mgr, int index);
int ioRead(int ioType, void *hdl, uint8_t *data, int dataLen) {
    switch (ioType) {
        case IO_TYPE_UART: {
            return halUartRead(hdl, data, dataLen);
        }break;
        case IO_TYPE_GPIO: {
            int val, i, j;
            gpioHand_t *p;
            uint8_t *pTimes;
            gpioManager_t *mgr = ((gpioManager_t *)hdl);
            for(i = j = 0, p = mgr->table; j < mgr->num && i < dataLen; j++, p++) {
                if(p->num < 0) {
                    continue;
                }
                halGPIORead(mgr->handle, p->num, &val);
                pTimes = (val == GPIO_STATE_LOW) ? &p->keepLowTimes : &p->keepHighTimes;
                if(p->gpiostate != val) {
                    *pTimes = 0;
                } else if(*pTimes < 0xFF) {
                    (*pTimes)++;
                }
                p->gpiostate = val;
                if(p->state != GPIO_STATE_LOW && p->state != GPIO_STATE_HIGH) {
                    val = p->state;
                }
                data[i] = (p->id << 4) & 0xF0;
                data[i] |= val & 0x0F;
                gpioCheckState(mgr, j);
                i++;
            }
            return i;
        } break;
        case IO_TYPE_PIPE: {
            return halPipeRead(hdl, data, dataLen);
        }break;
        case IO_TYPE_SOCKET: {

        }break;
    }    
    return 0;
}

static uint8_t s_resetLevel = 0;
static void gpioCheckState(gpioManager_t *mgr, int index)
{
    gpioHand_t *p = &mgr->table[index];

    if(p->type) {
        if(p->dir == GPIO_DIR_INPUT) {
            if(p->type == GPIO_TYPE_INPUT_RESET) {
                if(s_resetLevel < 2 && p->gpiostate == GPIO_STATE_HIGH && p->keepHighTimes > p->shortTime) {
                    s_resetLevel = (p->keepHighTimes > p->longTime) ? 2 : 1;
                }
            }
        } else {
            if(p->type == GPIO_TYPE_OUTPUT_RESET) {
                p->state = s_resetLevel ? GPIO_STATE_BLINK : p->freestate;
            }
        }
    } 
    if(p->dir == GPIO_DIR_OUTPUT) {
        if(p->state == GPIO_STATE_LOW || p->state == GPIO_STATE_HIGH) {
            if(p->state != p->gpiostate) {
                halGPIOWrite(mgr->handle, p->num, p->state);
            }
        } else if(p->state == GPIO_STATE_BLINK) {
            uint8_t kt = p->blink;
            uint8_t times = (p->gpiostate == GPIO_STATE_LOW) ? p->keepLowTimes : p->keepHighTimes;

            if(p->type == GPIO_TYPE_OUTPUT_RESET && s_resetLevel > 0) {
                kt = (s_resetLevel == 1) ? p->longTime : p->shortTime;
            }
            if(kt > 0 && times > kt) {
                halGPIOWrite(mgr->handle, p->num, !p->gpiostate);
            }
        }
    }
}

void ioDeinit(int ioType, void *hdl) {
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
            halGPIOClose(((gpioManager_t *)hdl)->handle);
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
    }    
    // return 0;
}
