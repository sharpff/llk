#include "leconfig.h"
#include "sengine.h"

#include "io.h"
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "ota.h"
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>

// #define MAX_STATUS 64
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) < (b)) ? (b) : (a))


typedef struct
{
    int param;
    int ret;
}IO;

typedef struct
{
    IO (*lf_impl_input)(lua_State *L, const uint8_t *input, int inputLen);
    int (*lf_impl_func)(lua_State *L, uint8_t *output, int outputLen);
}LF_IMPL;

typedef struct
{
    const char *func_name;
    LF_IMPL lf_impl;
}FUNC_LIST;

typedef struct {
    int beingReservedNum;
    char ruleName[MAX_RULE_NAME];
    char beingReservedStatus[MAX_RSV_NUM][MAX_BUF];
    uint8_t beingReservedUUID[MAX_RSV_NUM][MAX_UUID];
}IA_CACHE_INT;

typedef struct {
    IACfg cfg;
    IA_CACHE_INT cache[MAX_IA];
}IA_CACHE;

ScriptCfg *ginScriptCfg;
ScriptCfg *ginScriptCfg2;
static uint32_t ginDelayMS;
static IA_CACHE ginIACache;

static IO lf_s1GetQueries_input(lua_State *L, const uint8_t *input, int inputLen) {
    // lua_pushlstring(L, (char *)input, inputLen);
    IO io = { 0, 4 };
    return io;
}
static int lf_s1GetQueries(lua_State *L, uint8_t *output, int outputLen) {
    int size = 0;// i, j;
    uint8_t *tmp = 0;
    // uint16_t currLen = 0, appendLen = 0;
    Queries *queries = (Queries *)output;

    if (NULL == queries || sizeof(Queries) > outputLen) {
        return 0;
    }

    queries->queriesCountsLen = lua_tointeger(L, -4);
    size = MIN(queries->queriesCountsLen, outputLen);
    tmp = (uint8_t *)lua_tostring(L, -3);
    if (tmp && 0 < size) {
        memcpy(queries->arrQueriesCounts, tmp, size);
    } else {
        size = 0;
    }

    queries->queriesLen = lua_tointeger(L, -2);
    size = MIN(queries->queriesLen, outputLen);
    tmp = (uint8_t *)lua_tostring(L, -1);
    if (tmp && 0 < size) {
        memcpy(queries->arrQueries, tmp, size);
    } else {
        size = 0;
    }

    /*
    for (i = 0; i < queries->queriesCountsLen; i += 2) {
        LEPRINTF("[SENGINE]_s1GetQueries_[%d]_cmd: ", i/2);
        memcpy(&currLen, &queries->arrQueriesCounts[i], 2);
        for (j = 0; j < currLen; j++) {
            LEPRINTF("%02x ", queries->arrQueries[j + appendLen]);
        }
        appendLen += currLen;
        LEPRINTF("\r\n");
    }
    */
    return sizeof(Queries);
}

static IO lf_s1GetCvtType_input(lua_State *L, const uint8_t *input, int inputLen) {
    // lua_pushlstring(L, (char *)input, inputLen);
    IO io = { 0, 3 };
    return io;
}
static int lf_s1GetCvtType(lua_State *L, uint8_t *output, int outputLen) {
    // int i = 0;
    int sLen = lua_tointeger(L, -3);
    int size = MIN(sLen, outputLen);
    const uint8_t *tmp = (const uint8_t *)lua_tostring(L, -2);
    if (tmp && 0 < size) {
        memcpy(output, tmp, size);
    } else {
        size = 0;
    }
    ginDelayMS = lua_tointeger(L, -1);
    LELOG("[SENGINE] s1GetCvtType: [%d][%s] ginDelayMS[%d]\r\n", size, output, ginDelayMS);
    if (200 < ginDelayMS) {
        ginDelayMS = 200;
    }
    // LEPRINTF("std2pri [%d]: ", size);
    // for (i = 0; i < size; i++) {
    //     LEPRINTF("%02x ", output[i]);
    // }
    // LEPRINTF("\r\n");
    return size;
}

static IO lf_s1GetValidKind_input(lua_State *L, const uint8_t *input, int inputLen) {
    lua_pushlstring(L, (char *)input, inputLen);
    IO io = { 1, 1 };
    return io;
}
static int lf_s1GetValidKind(lua_State *L, uint8_t *output, int outputLen) {
    // int i = 0;
    *((int *)output) = lua_tointeger(L, -1);
    // LEPRINTF("[SENGINE] s1GetValidKind: [%d]\r\n", *((int *)output));
    return sizeof(int);
}

static IO lf_s1GetVer_input(lua_State *L, const uint8_t *input, int inputLen) {
    // lua_pushlstring(L, (char *)input, inputLen);
    IO io = { 0, 2 };
    return io;
}
static int lf_s1GetVer(lua_State *L, uint8_t *output, int outputLen) {
    // int i = 0;
    // LEPRINTF("[SENGINE] s1GetValidKind: [%d]\r\n", *((int *)output));
    int sLen = lua_tointeger(L, -2);
    int size = MIN(sLen, outputLen);
    const uint8_t *tmp = (const uint8_t *)lua_tostring(L, -1);
    if (tmp && 0 < size) {
        memcpy(output, tmp, size);
        LEPRINTF("[SENGINE] s1GetVer: [%d][%s]\r\n", size, output);
    } else {
        size = 0;
    }

    return size;
}

static IO lf_s1CvtStd2Pri_input(lua_State *L, const uint8_t *input, int inputLen) {
    lua_pushlstring(L, (char *)input, inputLen);
    IO io = { 1, 2 };
    return io;
}
static int lf_s1CvtStd2Pri(lua_State *L, uint8_t *output, int outputLen) {
    int i = 0;
    int sLen = lua_tointeger(L, -2);
    int size = MIN(sLen, outputLen);
    const uint8_t *tmp = (const uint8_t *)lua_tostring(L, -1);
    if (tmp && 0 < size) {
        memcpy(output, tmp, size);
        LEPRINTF("[SENGINE]_s1CvtStd2Pri_[%d]_cmd: ", size);
        for (i = 0; i < size; i++) {
            LEPRINTF("%02x ", output[i]);
        }
        LEPRINTF("\r\n");
    } else {
        size = 0;
    }

    return size;
}

static IO lf_s1CvtPri2Std_input(lua_State *L, const uint8_t *input, int inputLen) {
    lua_pushlstring(L, (char *)input, inputLen);
    IO io = { 1, 2 };
    return io;
}
static int lf_s1CvtPri2Std(lua_State *L, uint8_t *output, int outputLen) {
    /* cmd */
    int sLen = lua_tointeger(L, -2);
    int size = MIN(sLen, outputLen);
    const char *tmp = (const char *)lua_tostring(L, -1);
    if (tmp && 0 < size) {
        memcpy(output, tmp, size);
        // LEPRINTF("[SENGINE] s1CvtPri2Std: [%d][%s]\r\n", size, output);
    } else {
        size = 0;
    }

    return size;
}

static IO lf_s2IsValid_input(lua_State *L, const uint8_t *input, int inputLen) {
    lua_pushlstring(L, (char *)input, inputLen);
    IO io = { 1, 1 };
    return io;
}
static int lf_s2IsValid(lua_State *L, uint8_t *output, int outputLen) {
    /* cmd */
    *((int *)output) = lua_tointeger(L, -1);
    LEPRINTF("[SENGINE] s2IsValid: [%d]\r\n", *((int *)output));
    return sizeof(int);
}

// static IO lf_s2IsValidExt_input(lua_State *L, const uint8_t *input, int inputLen) {
//     long long a = 1456129920179;
//     lua_pushlstring(L, (char *)input, inputLen);
//     lua_pushinteger(L, a);
//     IO io = { 2, 1 };
//     return io;
// }

// static int lf_s2IsValidExt(lua_State *L, uint8_t *output, int outputLen) {
//     /* cmd */
//     *((int *)output) = lua_tointeger(L, -1);
//     LEPRINTF("[SENGINE] s2IsValidExt: [%d]\r\n", *((int *)output));
//     return sizeof(int);
// }

static IO lf_s2GetSelfName_input(lua_State *L, const uint8_t *input, int inputLen) {
    IO io = { 0, 2 };
    return io;
}
static int lf_s2GetSelfName(lua_State *L, uint8_t *output, int outputLen) {
    /* cmd */
    int sLen = lua_tointeger(L, -2);
    int size = MIN(sLen, outputLen);
    const char *tmp = (const char *)lua_tostring(L, -1);
    if (tmp && 0 < size) {
        memcpy(output, tmp, size);
        LEPRINTF("[SENGINE] lf_s2GetSelfName: [%d][%s]\r\n", size, output);
    } else {
        size = 0;
    }

    return size;
}

static IO lf_s2GetRuleType_input(lua_State *L, const uint8_t *input, int inputLen) {
    IO io = { 0, 2 };
    return io;
}
static int lf_s2GetRuleType(lua_State *L, uint8_t *output, int outputLen) {
    /* cmd */
    *((int *)output) = lua_tointeger(L, -2);
    *(((int *)output + 1)) = lua_tointeger(L, -1);
    LEPRINTF("[SENGINE] s2GetRuleType: repeat[%d] isAnd[%d]\r\n", *((int *)output), *(((int *)output + 1)));

    return 2*sizeof(int);
}

static IO lf_s2GetBeingReservedInfo_input(lua_State *L, const uint8_t *input, int inputLen) {
    IO io = { 0, 3 };
    return io;
}

static int lf_s2GetBeingReservedInfo(lua_State *L, uint8_t *output, int outputLen) {
    /* cmd */
    int num = 0, tmpLen = 0, sLen = 0, size = 0;
    char *tmp = NULL;
    num = lua_tointeger(L, -3);
    if (0 >= num) {
        return 0;
    }

    *((int *)output) = num;
    tmpLen = sizeof(int);

    sLen = lua_tointeger(L, -2);
    size = MIN(sLen, outputLen - tmpLen);
    tmp = (char *)lua_tostring(L, -1);
    if (tmp && 0 < size) {
        memcpy(output + tmpLen, tmp, size);
        tmpLen += size;
        LEPRINTF("[SENGINE] s2GetBeingReservedInfo: [%d][%s]\r\n", size, output);
    } else {
        size = 0;
    }        

    return tmpLen;
}

static IO lf_s2IsConditionOK_input(lua_State *L, const uint8_t *input, int inputLen) {
    int firstLen = 0;
    int secondLen = 0;

    firstLen = strlen(input);
    if (0 == firstLen) {
        // lua_pushlstring(L, NULL, 0);
    } else {
        lua_pushlstring(L, (char *)input, firstLen);
    }
    LEPRINTF("[SENGINE] lf_s2IsConditionOK_input: firstLen[%d][%d]\r\n", firstLen, inputLen);
    firstLen += 1;
    secondLen = strlen((char *)input + firstLen);
    if (0 == secondLen) {
        // lua_pushlstring(L, NULL, 0);
    } else {
        lua_pushlstring(L, (char *)input + firstLen, secondLen);
    }
    LEPRINTF("[SENGINE] lf_s2IsConditionOK_input: secondLen[%d][%d]\r\n", secondLen, inputLen);
    IO io = { 2, 1 };
    return io;
}
static int lf_s2IsConditionOK(lua_State *L, uint8_t *output, int outputLen) {
    /* cmd */
    *((int *)output) = lua_tointeger(L, -1);
    LEPRINTF("[SENGINE] s2IsConditionOK: [%d]\r\n", *((int *)output));
    return sizeof(int);
}

static IO lf_s2GetSelfCtrlCmd_input(lua_State *L, const uint8_t *input, int inputLen) {
    IO io = { 0, 2 };
    return io;
}
static int lf_s2GetSelfCtrlCmd(lua_State *L, uint8_t *output, int outputLen) {
    /* cmd */
    int sLen = lua_tointeger(L, -2);
    int size = MIN(sLen, outputLen);
    const char *tmp = (const char *)lua_tostring(L, -1);
    if (tmp && 0 < size) {
        memcpy(output, tmp, size);
        LEPRINTF("[SENGINE] s2GetSelfCtrlCmd: [%d][%s]\r\n", size, output);
    } else {
        size = 0;
    }

    return size;
}
static FUNC_LIST func_list[] = {
    { S1_GET_CVTTYPE, { lf_s1GetCvtType_input, lf_s1GetCvtType } },
    { S1_GET_QUERIES, { lf_s1GetQueries_input, lf_s1GetQueries } },
    { S1_STD2PRI, { lf_s1CvtStd2Pri_input, lf_s1CvtStd2Pri } },
    { S1_PRI2STD, { lf_s1CvtPri2Std_input, lf_s1CvtPri2Std } },
    { S1_GET_VALIDKIND, { lf_s1GetValidKind_input, lf_s1GetValidKind } },
    { S1_GET_VER, { lf_s1GetVer_input, lf_s1GetVer } },
    { S2_IS_VALID, { lf_s2IsValid_input, lf_s2IsValid } },
    // { S2_IS_VALID_EXT, { lf_s2IsValidExt_input, lf_s2IsValidExt } },
    { S2_GET_SELFNAME, { lf_s2GetSelfName_input, lf_s2GetSelfName } },
    { S2_GET_RULETYPE, { lf_s2GetRuleType_input, lf_s2GetRuleType } },
    { S2_GET_BERESERVED, { lf_s2GetBeingReservedInfo_input, lf_s2GetBeingReservedInfo } },
    { S2_GET_ISOK, { lf_s2IsConditionOK_input, lf_s2IsConditionOK } },
    { S2_GET_BECMD, { lf_s2GetSelfCtrlCmd_input, lf_s2GetSelfCtrlCmd } },
    { NULL, { 0, 0 } }
};

LF_IMPL get_lf_impl(const char *func_name, int *param_num)
{
    int i = 0;
    LF_IMPL impl = { 0, 0 };
    for (i = 0; i < (sizeof(func_list) / sizeof(FUNC_LIST)-1); i++)
    {
        if (0 == strcmp(func_name, func_list[i].func_name))
        {
            //*param_num = func_list[i].param_num;
            impl.lf_impl_input = func_list[i].lf_impl.lf_impl_input;
            impl.lf_impl_func = func_list[i].lf_impl.lf_impl_func;
            return impl;
        }
    }

    return impl;
}

int sengineInit(void) {
    int ret = 0;
    // #define STATIC_MEMORY_FOR_SCRIPT
    #ifndef STATIC_MEMORY_FOR_SCRIPT
    ginScriptCfg = (ScriptCfg *)malloc(sizeof(ScriptCfg));
    memset(ginScriptCfg, 0, sizeof(ScriptCfg));
    ginScriptCfg2 = (ScriptCfg *)malloc(sizeof(ScriptCfg));
    memset(ginScriptCfg2, 0, sizeof(ScriptCfg));
    #else
    static ScriptCfg inScriptCfg;
    static ScriptCfg inScriptCfg2;
    ginScriptCfg = &inScriptCfg;
    ginScriptCfg2 = &inScriptCfg2;
    #endif
    ret = lelinkStorageReadScriptCfg(ginScriptCfg, OTA_TYPE_FW_SCRIPT, 0);
    
    if (0 > ret) {
        LELOG("lelinkStorageReadScriptCfg 1 read from flash failed\r\n");
        return -1;
    }

    if (ginScriptCfg->csum != crc8((uint8_t *)&(ginScriptCfg->data), sizeof(ginScriptCfg->data))) {
        LELOG("ginScriptCfg crc8 failed\r\n");
        return -2;
    }

    return 0;
}

// int bitshift(lua_State *L);
// int bitshiftL(lua_State *L);
// int bitor(lua_State *L);
// int bitxor(lua_State *L);
// int bitnor(lua_State *L);
// int bitand(lua_State *L);
// int csum(lua_State *L);
int s2apiStoreCurrStatus(lua_State *L);

int sengineCall(const char *script, int scriptSize, const char *funcName, const uint8_t *input, int inputLen, uint8_t *output, int outputLen)
{
    int ret = 0;
    lua_State *L = luaL_newstate();

    // lua_register(L, "bitshift", bitshift);
    // lua_register(L, "bitshiftL", bitshiftL);
    // lua_register(L, "bitand", bitand);
    // lua_register(L, "bitor", bitor);
    // lua_register(L, "bitxor", bitxor);
    // lua_register(L, "bitnor", bitnor);
    lua_register(L, "s2apiStoreCurrStatus", s2apiStoreCurrStatus);
    // lua_register(L, "csum", csum);

    if (script == NULL)
        return -1;

    luaL_openlibs(L);
    if (luaL_loadbuffer(L, script, scriptSize, "lelink") || lua_pcall(L, 0, 0, 0))
    {
        lua_pop(L, 1);
        ret = -1;
        LELOG("[lua engine] lua code syntax error\r\n");
    }
    else
    {
        IO io_ret = { 0, 0 };
        LF_IMPL lf_impl = { 0, 0 };
        lf_impl = get_lf_impl(funcName, 0);
        if (NULL == lf_impl.lf_impl_input)
        {
            return -2;
        }

        lua_getglobal(L, funcName);
        
        // do push param
        io_ret = lf_impl.lf_impl_input(L, input, inputLen);

        if (lua_pcall(L, io_ret.param, io_ret.ret, 0))
        {
            const char *err = lua_tostring(L, -1);
            LELOG("[lua engine] lua error: %s\r\n", err);
            lua_pop(L, 1);
            ret = -3;
        }
        else
        {
            // int param_num = 0;
            if (lf_impl.lf_impl_func)
            {
                ret = lf_impl.lf_impl_func(L, output, outputLen);
                lua_pop(L, io_ret.ret);
            }
        }
    }

    //wmprintf("[config]: config[0x%x] \r\nlua[0x%x] \r\ntimer[0x%x] \r\n", SYS_CONFIG_OFFSET, LUA_STORE_ADDRESS, JOYLINK_TIMER_MEM_ADDR);

    lua_close(L);
    return ret;
}

int s2apiStoreCurrStatus(lua_State *L) {
    int ret = 0;
    int m, n = 0;
    // char strsum[3] = { 0 };
    char *strSelfName = NULL;
    char *strUUID = NULL;
    char *strReservedStatus = NULL;
    int lenSelfName = 0, lenUUID = 0, lenReservedStatus = 0;

    // get self name
    LELOG("s2apiStoreCurrStatus -s\r\n");
    lenSelfName = lua_tointeger(L, 1);
    if (0 >= lenSelfName) {
        LELOGE("s2apiStoreCurrStatus -e1\r\n");
        return -1;
    }
    strSelfName = (char *)lua_tostring(L, 2);
    if (NULL == strSelfName) {
        // lua_pushstring(L, strsum);
        LELOGE("s2apiStoreCurrStatus -e2\r\n");
        return -2;
    }

    // get uuid 
    lenUUID = lua_tointeger(L, 3);
    if (0 >= lenUUID) {
        LELOGE("s2apiStoreCurrStatus -e3\r\n");
        return -3;
    }
    strUUID = (char *)lua_tostring(L, 4);
    if (NULL == strUUID) {
        // lua_pushstring(L, strsum);
        LELOGE("s2apiStoreCurrStatus -e4\r\n");
        return -4;
    }

    // get the current status
    lenReservedStatus = lua_tointeger(L, 5);
    if (0 >= lenReservedStatus) {
        LELOGE("s2apiStoreCurrStatus -e3\r\n");
        return -5;
    }
    strReservedStatus = (char *)lua_tostring(L, 6);
    if (NULL == strReservedStatus) {
        // lua_pushstring(L, strsum);
        LELOGE("s2apiStoreCurrStatus -e4\r\n");
        return -6;
    }

    for (m = 0; m < MAX_IA && 0 == ret; m++) {
        // LELOG("[%d] [%s] <=> [%s]\r\n", m, ginIACache.cache[m].ruleName, strSelfName); 
        if (0 == strcmp(ginIACache.cache[m].ruleName, strSelfName)) {
            for (n = 0; n < MAX_RSV_NUM; n++) {
                // LELOG("[%d] [%s] <=> [%s]\r\n", n, ginIACache.cache[m].beingReservedUUID[n], strUUID); 
                if (0 == memcmp(ginIACache.cache[m].beingReservedUUID[n], strUUID, MAX_UUID)) {
                    memcpy(ginIACache.cache[m].beingReservedStatus[n], strReservedStatus, MIN(lenReservedStatus, MAX_BUF));
                    ginIACache.cache[m].beingReservedStatus[n][MIN(lenReservedStatus, MAX_BUF-1)] = 0;
                    ret = 1;
                    break;
                }
            }            
        }
    }
    LELOG("s2apiStoreCurrStatus[%d] IDX[%d][%d]\r\n", ret, ret ? m - 1 : m, n); 
    LELOG("[%d][%s]\r\n [%d][%s]\r\n [%d][%s]\r\n", 
        lenSelfName, strSelfName, lenUUID, strUUID, lenReservedStatus, strReservedStatus); 

    // LELOG("s2apiStoreCurrStatus IDX[%d][%d] [%d][%s], [%d][%s]\r\n", 
    //     i, j, lenUUID, str1, len2, strReservedStatus);
    LELOG("s2apiStoreCurrStatus -e\r\n");
    return ret;

}

int sengineSetStatus(char *json, int jsonLen) {
    int ret = 0;
    uint8_t bin[512] = {0};
    ret = sengineCall((const char *)ginScriptCfg->data.script, ginScriptCfg->data.size, S1_STD2PRI,
        (uint8_t *)json, jsonLen, bin, sizeof(bin));
    if (ret <= 0) {
        LELOGW("sengineSetStatus sengineCall("S1_STD2PRI") [%d]\r\n", ret);
        return ret;
    }

    ret = ioWrite(IO_TYPE_UART, *((void **)ioGetHdl(IO_TYPE_UART)), bin, ret);
    if (ret <= 0) {
        LELOGW("sengineSetStatus ioWrite [%d]\r\n", ret);
        return ret;
    }

    return ret;
}

int sengineGetTerminalProfileCvtType(char *json, int jsonLen) {
    int ret = 0;
    ret = sengineCall((const char *)ginScriptCfg->data.script, ginScriptCfg->data.size, S1_GET_CVTTYPE,
            NULL, 0, (uint8_t *)json, jsonLen);
    if (ret <= 0) {
        LELOGW("sengineGetStatus sengineCall("S1_GET_CVTTYPE") [%d]\r\n", ret);
        return ret;
    }
    return ret;
}

int sengineQuerySlave(void) 
{
    Queries queries;
    int ret = 0, i = 0;
    uint16_t currLen = 0, appendLen = 0;

    // 0. getQueries from script
    ret = sengineCall((const char *)ginScriptCfg->data.script, ginScriptCfg->data.size, S1_GET_QUERIES,
            NULL, 0, (uint8_t *)&queries, sizeof(queries));
    if (ret <= 0) {
        LELOGE("sengineGetStatus sengineCall("S1_GET_QUERIES") [%d]\r\n", ret);
        return ret;
    }
    for (i = 0; i < queries.queriesCountsLen; i += 2, appendLen += currLen) {
        memcpy(&currLen, &queries.arrQueriesCounts[i], 2);
        ret = ioWrite(IO_TYPE_UART, *((void **)ioGetHdl(IO_TYPE_UART)), &(queries.arrQueries[appendLen]), currLen);
        if (ret <= 0) {
            LELOGE("sengineGetStatus ioWrite [%d]\r\n", ret);
            return ret;
        }
    }
    return 0;
}

int senginePollingSlave(void) {
    char status[256];
    uint8_t bin[128] = {0};
    int whatKind = 0, ret = 0, size;

    ret = ioRead(IO_TYPE_UART, *((void **)ioGetHdl(IO_TYPE_UART)), bin, sizeof(bin));
    if (ret <= 0) {
        LELOGW("sengineGetStatus ioRead [%d]\r\n", ret);
        return ret;
    }
    size = ret;
    ret = sengineCall((const char *)ginScriptCfg->data.script, ginScriptCfg->data.size, S1_GET_VALIDKIND,
            bin, size, (uint8_t *)&whatKind, sizeof(whatKind));
    if (ret <= 0) {
        return -1;
    }
    switch (whatKind) {
        case 1: // wifi reset
            LELOG("Please reset wifi\r\n");
            break;
        case 2: // status
            ret = sengineCall((const char *)ginScriptCfg->data.script, ginScriptCfg->data.size, S1_PRI2STD,
                    bin, size, (uint8_t *)status, sizeof(status));
            if (ret <= 0) {
                LELOGW("sengineGetStatus sengineCall("S1_PRI2STD") [%d]\r\n", ret);
            }
            cacheSetTerminalStatus(status, ret);
            LELOG("Cache status:%s\r\n", status);
            break;
        default:
            LELOGE("Unknow whatKind = %d\r\n", whatKind);
            return -1;
    }

    return whatKind;
}

/*
 * 0. 
 * check if it is valid
 * check if it is a repeat rule
 * check if it is a corresponding info(uuid)
 */ 
// int sengineFindRule(const char *json, int jsonLen) {

//     return isFound;
// }

int sengineS2GetBeingReservedInfo(const ScriptCfg *scriptCfg2, uint8_t strBeingReserved[MAX_RSV_NUM][MAX_UUID]) {
    int ret = 0, count = 0, i;
    uint8_t tmpBeingReserved[sizeof(int) + (MAX_RSV_NUM*MAX_UUID)] = {0};
    char *tmp = NULL;

    if (NULL == strBeingReserved) {
        return -1;
    }

    ret = sengineCall((const char *)scriptCfg2->data.script, scriptCfg2->data.size, S2_GET_BERESERVED,
        NULL, 0, tmpBeingReserved, sizeof(tmpBeingReserved));
    if (0 > ret) {
        LELOGW("sengineS2GetBeingReservedInfo sengineCall("S2_GET_BERESERVED") [%d]\r\n", ret);
        return -2;
    }

    count = MIN(*((int *)tmpBeingReserved), MAX_RSV_NUM);
    LELOGW("sengineS2GetBeingReservedInfo count[%d]\r\n", count);

    // point to the list of being reserved uuidList
    tmp = (char *)tmpBeingReserved + sizeof(int);
    for (i = 0; i < count; i++) {
        memcpy(strBeingReserved[i], tmp, MAX_UUID);
        tmp += MAX_UUID;
        LELOGW("strBeingReserved[%d][%s]\r\n", MAX_UUID, strBeingReserved[i]);
    }

    return count;
}

int sengineS2RuleHandler(const ScriptCfg *scriptCfg2, 
    const char *uuid, int len, 
    const char *json, int jsonLen,
    IA_CACHE_INT *cacheInt) {

    int ret = 0, isRepeat = 0, isAND = 0, is = 0, i, count = 0;
    uint8_t buf[MAX_BUF * 2] = {0};
    // char validList[MAX_IA][MAX_UUID + 1] = {0};
    if (NULL == scriptCfg2) {
        return -1;
    }

    /*
     * 0. 
     * check if the rule is valid
     */
    ret = genS2Json(json, jsonLen, buf, sizeof(buf));
    if (0 >= ret) {
        LELOGW("sengineS2RuleHandler genS2Json [%d]\r\n", ret);
        return -2;
    }
    ret = sengineCall((const char *)scriptCfg2->data.script, scriptCfg2->data.size, S2_IS_VALID,
        (uint8_t *)json, jsonLen, (uint8_t *)&is, sizeof(is));
    if (0 > ret) {
        LELOGW("sengineS2RuleHandler sengineCall("S2_IS_VALID") [%d]\r\n", ret);
        return -3;
    }
    if (!is) {
        LELOG("sengineS2RuleHandler invalid -e1 \r\n");
        return 0;
    }

    /*
     * 1.
     * check if it is a repeat rule
     */
    ret = sengineCall((const char *)scriptCfg2->data.script, scriptCfg2->data.size, S2_GET_RULETYPE,
        NULL, 0, (uint8_t *)buf, sizeof(buf));
    if (0 > ret) {
        LELOGW("sengineS2RuleHandler sengineCall("S2_GET_RULETYPE") [%d]\r\n", ret);
        return -4;
    }
    isRepeat = *((int *)buf);
    isAND = *((int *)buf + 1);
    LELOG("sengineS2RuleHandler sengineCall("S2_GET_RULETYPE") [%d][%d]\r\n", isRepeat, isAND);

    /*
     * 2. 
     * check all the reserved targets in a rule
     */
    count = sengineS2GetBeingReservedInfo(scriptCfg2, cacheInt->beingReservedUUID);
    if (0 >= count) {
        return 0;
    }
    cacheInt->beingReservedNum = count;
    LELOG("sengineS2RuleHandler sengineS2GetBeingReservedInfo[%d]\r\n", count);
    for (i = 0; i < count; i++) {
        int isFound = 0;
        /*
        * 3. 
        * check if it is a corresponding info(uuid)
        */
        memset(buf, 0, sizeof(buf));
        LELOG("idx[%d] [%s] <=> [%s]\r\n", i, uuid, cacheInt->beingReservedUUID[i]);
        // 1st param
        if (0 == memcmp(uuid, cacheInt->beingReservedUUID[i], MAX_UUID)) {
            LELOG("1st param[%d][%s]\r\n", strlen(json), json);
            strcpy(buf, json);
            isFound = 1;
        } else {
            LELOG("has no 1st param\r\n");
            strcpy(buf, "{}");
        }

        if (!isFound && !isRepeat) {
            continue;
        }

        // 2nd param
        if (0 < strlen(cacheInt->beingReservedStatus[i])) {
            LELOG("cp beingReservedStatus [%d][%s]\r\n", MIN(strlen(cacheInt->beingReservedStatus[i]), MAX_BUF), cacheInt->beingReservedStatus[i]);
            memcpy(buf + strlen(buf) + 1, cacheInt->beingReservedStatus[i], MIN(strlen(cacheInt->beingReservedStatus[i]), MAX_BUF));
        } else {
            strcpy(buf + strlen(buf) + 1, "{}");
        }
        ret = sengineCall((const char *)scriptCfg2->data.script, scriptCfg2->data.size, S2_GET_ISOK,
            (uint8_t *)buf, sizeof(buf), (uint8_t *)&is, sizeof(is));
        if (0 > ret) {
            LELOGW("senginePollingRules 1 sengineCall("S2_GET_ISOK") [%d]\r\n", ret);
            continue;
        }
        if (!is) {
            LELOGW("senginePollingRules condition not ok -e2 \r\n");
            continue;
        }
        LELOG("senginePollingRules sengineCall("S2_GET_ISOK") ok [%d]\r\n", is);

        /*
         * 4.
         * get the self ctrl cmd
         */
        ret = sengineCall((const char *)scriptCfg2->data.script, scriptCfg2->data.size, S2_GET_BECMD,
            NULL, 0, (uint8_t *)&buf, sizeof(buf));
        if (0 > ret) {
            LELOGW("senginePollingRules sengineCall("S2_GET_BECMD") [%d]\r\n", ret);
            continue;
        }
        LELOG("senginePollingRules sengineCall("S2_GET_BECMD") ctrl [%d][%s]\r\n", ret, buf);

        // 5. do ctrl
        ret = sengineSetStatus((char *)buf, ret);
        LELOG("senginePollingRules sengineSetStatus [%d]\r\n", ret);      
    }

    LELOG("senginePollingRules condition -e \r\n");

    // /*
    //  * 1. get corrsponding rules to do sth. 
    //  * gen an AND list, an OR list
    //  * make lite cache for every script2
    //  */
    // LELOG("JSON [%d][%s]\r\n", jsonLen, json);
    // ret = sengineFindRule(json, jsonLen);
    // if (0 >= ret) {
    //     LELOGW("findTheRule [%d]\r\n", ret);
    //     return -3;
    // }

    // ret = genS2Json(json, jsonLen, result, sizeof(result));
    // if (0 >= ret) {
    //     LELOGW("sengineS2RuleHandler genS2Json [%d]\r\n", ret);
    //     return -4;
    // }
    // resultLen = ret;

    // // 2. check if the rule has been expired
    // ret = sengineCall((const char *)scriptCfg2->data.script, scriptCfg2->data.size, S2_IS_VALID,
    //     (uint8_t *)result, ret, (uint8_t *)&is, sizeof(is));
    // if (0 > ret) {
    //     LELOGW("sengineS2RuleHandler sengineCall("S2_IS_VALID") [%d]\r\n", ret);
    //     return -5;
    // }
    // if (!is) {
    //     LELOG("sengineS2RuleHandler invalid -e1 \r\n");
    //     return 0;
    // }
    // LELOG("sengineS2RuleHandler sengineCall("S2_IS_VALID") valid [%d]\r\n", is);

    /*
     * 0. 
     * check if it is valid
     * check if it is a repeat rule
     * check if it is a corresponding info(uuid)
     */ 
    // ret = sengineFindRule(json, jsonLen);
    // if (0 >= ret) {
    //     LELOGW("findTheRule [%d]\r\n", ret);
    //     return -3;
    // }


    /*
     * 1.
     * check if condition is ok
     */
    return 0;
}


int senginePollingRules(const char *json, int jsonLen) {
    int ret, i = 0, isFound = 0;
    // int64_t utc = 0;
    // uint32_t utcH = 0, utcL = 0;
    // char jsonUTC[64] = {0};
    // char result[MAX_BUF] = {0};
    char strSelfRuleName[MAX_RULE_NAME] = {0};

    PrivateCfg privCfg;
    // char strBeingReserved[64] = {0};
    char strUUID[64] = {0};

    LELOG("senginePollingRules -s \r\n");
    ret = lelinkStorageReadPrivateCfg(&privCfg);
    if (privCfg.csum != crc8((const uint8_t *)&(privCfg.data), sizeof(privCfg.data))) {
        LELOGW("senginePollingRules csum failed\r\n");
        return -1;
    }
    // LELOG("sengineFindRule [%d][%d]\r\n", privCfg.data.iaCfg.num);

    if (0 >= privCfg.data.iaCfg.num) {
        LELOGW("senginePollingRules rules num[%d]\r\n", privCfg.data.iaCfg.num);
        return 0;
    }
    LELOG("senginePollingRules rules num[%d] \r\n", privCfg.data.iaCfg.num);

    ret = getUUIDFromJson(json, jsonLen, strUUID, sizeof(strUUID));
    if (0 >= ret) {
        LELOGW("senginePollingRules getUUIDFromJson[%d]\r\n", ret);
        return -2;
    }

    // for every single rule
    for (i = 0; i < privCfg.data.iaCfg.num; i++) {
        if (0 < privCfg.data.iaCfg.arrIA[i]) {
            memset(ginScriptCfg2, 0, sizeof(ScriptCfg));
            ret = lelinkStorageReadScriptCfg(ginScriptCfg2, OTA_TYPE_IA_SCRIPT, i);
            if (0 > ret) {
                LELOGW("senginePollingRules failed arrIA idx[%d]\r\n", i);
                continue;
            }
            if (ginScriptCfg2->csum != crc8((uint8_t *)&(ginScriptCfg2->data), sizeof(ginScriptCfg2->data))) {
                LELOGW("senginePollingRules failed crc8 idx[%d]\r\n", i);
                continue;
            }

            // set the rule's name
            ret = sengineCall((const char *)ginScriptCfg2->data.script, ginScriptCfg2->data.size, S2_GET_SELFNAME,
                NULL, 0, (uint8_t *)&strSelfRuleName, sizeof(strSelfRuleName));
            if (0 > ret) {
                LELOGW("senginePollingRules sengineCall("S2_GET_SELFNAME") [%d]\r\n", ret);
                continue;
            }
            memcpy(ginIACache.cache[i].ruleName, strSelfRuleName, MIN(ret, MAX_RULE_NAME));
            ginIACache.cache[i].ruleName[MIN(ret, MAX_RULE_NAME-1)] = 0;

            sengineS2RuleHandler(ginScriptCfg2, strUUID, strlen(strUUID), json, jsonLen, &(ginIACache.cache[i]));

        }
    }
    LELOG("senginePollingRules [%d]-e \r\n", isFound);

    return 0;
}



int test_lf_call(char *luacode, int size)
{
    // int i = 0;
    // for (i = 0; i < (sizeof(func_list) / sizeof(FUNC_LIST) - 1); i++)
    // {
    //     sengineCall(luacode, size, func_list[i].func_name);
    // }

    return 0;
}


