#include "leconfig.h"
#include "sengine.h"

#include "io.h"
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "ota.h"
#include "state.h"
#include "protocol.h"

#ifndef LOG_SENGINE
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
    // char buf[1024*10];
}IA_CACHE;

ScriptCfg *ginScriptCfg;
ScriptCfg *ginScriptCfg2;
static uint32_t ginDelayMS;
static IA_CACHE ginIACache;

static IO lf_s1GetQueries_input(lua_State *L, const uint8_t *input, int inputLen) {
    lua_pushinteger(L, *(QuerieType_t *)input);
    IO io = { 1, 4 };
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
    LELOG("[SENGINE] s1GetCvtType: [%d][%s] ginDelayMS[%d]", size, output, ginDelayMS);
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
    // LEPRINTF("[SENGINE] s1GetValidKind: [%d]", *((int *)output));
    return sizeof(int);
}

static IO lf_s1GetVer_input(lua_State *L, const uint8_t *input, int inputLen) {
    // lua_pushlstring(L, (char *)input, inputLen);
    IO io = { 0, 2 };
    return io;
}
static int lf_s1GetVer(lua_State *L, uint8_t *output, int outputLen) {
    // int i = 0;
    // LEPRINTF("[SENGINE] s1GetValidKind: [%d]", *((int *)output));
    int sLen = lua_tointeger(L, -2);
    int size = MIN(sLen, outputLen);
    const uint8_t *tmp = (const uint8_t *)lua_tostring(L, -1);
    if (tmp && 0 < size) {
        memcpy(output, tmp, size);
        LEPRINTF("[SENGINE] s1GetVer: [%d][%s]", size, output);
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
        // LEPRINTF("[SENGINE] s1CvtPri2Std: [%d][%s]", size, output);
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
    LEPRINTF("[SENGINE] s2IsValid: [%d]", *((int *)output));
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
//     LEPRINTF("[SENGINE] s2IsValidExt: [%d]", *((int *)output));
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
        LEPRINTF("[SENGINE] lf_s2GetSelfName: [%d][%s]", size, output);
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
    LEPRINTF("[SENGINE] s2GetRuleType: repeat[%d] isAnd[%d]", *((int *)output), *(((int *)output + 1)));

    return 2*sizeof(int);
}

static IO lf_s2GetBeingReservedInfo_input(lua_State *L, const uint8_t *input, int inputLen) {
    IO io = { 0, 1 };
    return io;
}

static int lf_s2GetBeingReservedInfo(lua_State *L, uint8_t *output, int outputLen) {
    ///* cmd */
    int num = 0, idx = 0, tmpLen = sizeof(int);
    char *tmp = NULL;
    // int ret = 0;
  
    idx = lua_gettop(L);
    lua_pushnil(L);

    while (lua_next(L, idx) != 0) {
        int n = 0;
        tmp = (char *)lua_tostring(L, -1);
        if (tmp) {
            n = strlen(tmp);
            memcpy(output + tmpLen, tmp, n);
            LEPRINTF("[SENGINE] s2GetBeingReservedInfo: [%d][%s]", n, output + tmpLen);
            tmpLen += n;
            // output[tmpLen] = 0;
            // tmpLen += 1;
        }
        lua_pop(L, 1);
        num++;
    }
    *((int *)output) = num;
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
    LEPRINTF("[SENGINE] lf_s2IsConditionOK_input: firstLen[%d/%d][%s]", firstLen, inputLen, input);
    firstLen += 1;
    secondLen = strlen((char *)input + firstLen);
    if (0 == secondLen) {
        // lua_pushlstring(L, NULL, 0);
    } else {
        lua_pushlstring(L, (char *)input + firstLen, secondLen);
    }
    LEPRINTF("[SENGINE] lf_s2IsConditionOK_input: secondLen[%d/%d][%s]", secondLen, inputLen, input + firstLen);
    IO io = { 2, 1 };
    return io;
}
static int lf_s2IsConditionOK(lua_State *L, uint8_t *output, int outputLen) {
    /* cmd */
    *((int *)output) = lua_tointeger(L, -1);
    LEPRINTF("[SENGINE] s2IsConditionOK: [%d]", *((int *)output));
    return sizeof(int);
}

static IO lf_s2IsConditionOKExt_input(lua_State *L, const uint8_t *input, int inputLen) {
    IO io = { 2, 1 };
    int selfLen = 0;
    int rmtLen = 0;

    selfLen = strlen(input);
    if (0 == selfLen) {
        // lua_pushlstring(L, NULL, 0);
    }
    else {
        lua_pushlstring(L, (char *)input, selfLen);
    }


    rmtLen = strlen(input + selfLen + 1);
    if (0 == rmtLen) {
        // lua_pushlstring(L, NULL, 0);
    }
    else {
        lua_pushlstring(L, (char *)input + selfLen + 1, rmtLen);
    }
    return io;
}
static int lf_s2IsConditionOKExt(lua_State *L, uint8_t *output, int outputLen) {
    /* cmd */
    *((int *)output) = lua_tointeger(L, -1);
    LEPRINTF("[SENGINE] s2IsConditionOK: [%d]", *((int *)output));
    return sizeof(int);
}


static IO lf_s2GetSelfCtrlCmd_input(lua_State *L, const uint8_t *input, int inputLen) {
    IO io = { 0, 2 };
    return io;
}
static int lf_s2GetSelfCtrlCmd(lua_State *L, uint8_t *output, int outputLen) {
    /* cmd */
    int sLen = lua_tointeger(L, -2);
    int size = MIN(sLen + 1, outputLen);
    const char *tmp = (const char *)lua_tostring(L, -1);
    if (tmp && 0 < size) {
        memcpy(output, tmp, sLen); output[sLen] = 0;
        LEPRINTF("[SENGINE] s2GetSelfCtrlCmd: [%d][%s]", sLen, output);
    } else {
        sLen = 0;
    }

    return sLen;
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
    { S2_GET_ISOK_EXT, { lf_s2IsConditionOKExt_input, lf_s2IsConditionOKExt } },
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
    ginScriptCfg = (ScriptCfg *)halCalloc(1, sizeof(ScriptCfg));
    ginScriptCfg2 = (ScriptCfg *)halCalloc(1, sizeof(ScriptCfg));
    #else
    static ScriptCfg inScriptCfg;
    static ScriptCfg inScriptCfg2;
    ginScriptCfg = &inScriptCfg;
    ginScriptCfg2 = &inScriptCfg2;
    #endif
    ret = lelinkStorageReadScriptCfg(ginScriptCfg, E_FLASH_TYPE_SCRIPT, 0);
    
    if (0 > ret) {
        LELOG("lelinkStorageReadScriptCfg 1 read from flash FAILED");
        return -1;
    }

    if (ginScriptCfg->csum != crc8((uint8_t *)&(ginScriptCfg->data), sizeof(ginScriptCfg->data))) {
        LELOG("ginScriptCfg crc8 FAILED");
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
int s2apiSetCurrStatus(lua_State *L);
int s2apiGetLatestStatus(lua_State *L);

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
    lua_register(L, "s2apiSetCurrStatus", s2apiSetCurrStatus);
    lua_register(L, "s2apiGetLatestStatus", s2apiGetLatestStatus);
    // lua_register(L, "csum", csum);
    
    if (script == NULL || scriptSize <= 0)
        return -1;

    luaL_openlibs(L);
    if (luaL_loadbuffer(L, script, scriptSize, "lelink") || lua_pcall(L, 0, 0, 0))
    {
        lua_pop(L, 1);
        ret = -1;
        LELOGE("[lua engine] lua code syntax error");
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
            LELOGE("[lua engine] lua error: %s", err);
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

    //wmprintf("[config]: config[0x%x] lua[0x%x] timer[0x%x] ", SYS_CONFIG_OFFSET, LUA_STORE_ADDRESS, JOYLINK_TIMER_MEM_ADDR);

    lua_close(L);
    return ret;
}

int s2apiSetCurrStatus(lua_State *L) {
    int ret = 0;
    int m, n = 0;
    // char strsum[3] = { 0 };
    char *strSelfName = NULL;
    char *strUUID = NULL;
    char *strReservedStatus = NULL;
    int lenSelfName = 0, lenUUID = 0, lenReservedStatus = 0;

    // get self name
    LELOG("s2apiSetCurrStatus -s");
    lenSelfName = lua_tointeger(L, 1);
    if (0 >= lenSelfName) {
        LELOGE("s2apiSetCurrStatus -e1");
        return 0;
    }
    strSelfName = (char *)lua_tostring(L, 2);
    if (NULL == strSelfName) {
        // lua_pushstring(L, strsum);
        LELOGE("s2apiSetCurrStatus -e2");
        return 0;
    }

    // get uuid 
    lenUUID = lua_tointeger(L, 3);
    if (0 >= lenUUID) {
        LELOGE("s2apiSetCurrStatus -e3");
        return 0;
    }
    strUUID = (char *)lua_tostring(L, 4);
    if (NULL == strUUID) {
        // lua_pushstring(L, strsum);
        LELOGE("s2apiSetCurrStatus -e4");
        return 0;
    }

    // get the current status
    lenReservedStatus = lua_tointeger(L, 5);
    if (0 >= lenReservedStatus) {
        LELOGE("s2apiSetCurrStatus -e3");
        return 0;
    }
    strReservedStatus = (char *)lua_tostring(L, 6);
    if (NULL == strReservedStatus) {
        // lua_pushstring(L, strsum);
        LELOGE("s2apiSetCurrStatus -e4");
        return 0;
    }

    for (m = 0; m < MAX_IA && 0 == ret; m++) {
        // LELOG("[%d] [%s] <=> [%s]", m, ginIACache.cache[m].ruleName, strSelfName); 
        if (0 == strcmp(ginIACache.cache[m].ruleName, strSelfName)) {
            for (n = 0; n < MAX_RSV_NUM; n++) {
                // LELOG("[%d] [%s] <=> [%s]", n, ginIACache.cache[m].beingReservedUUID[n], strUUID); 
                if (0 == memcmp(ginIACache.cache[m].beingReservedUUID[n], strUUID, MAX_UUID)) {
                    memcpy(ginIACache.cache[m].beingReservedStatus[n], strReservedStatus, MIN(lenReservedStatus, MAX_BUF));
                    ginIACache.cache[m].beingReservedStatus[n][MIN(lenReservedStatus, MAX_BUF-1)] = 0;
                    ret = 1;
                    break;
                }
            }            
        }
    }
    LELOG("s2apiSetCurrStatus[%d] IDX[%d][%d]", ret, ret ? m - 1 : m, n); 
    LELOG("[%d][%s] [%d][%s] [%d][%s]", 
        lenSelfName, strSelfName, lenUUID, strUUID, lenReservedStatus, strReservedStatus); 

    // LELOG("s2apiSetCurrStatus IDX[%d][%d] [%d][%s], [%d][%s]", 
    //     i, j, lenUUID, str1, len2, strReservedStatus);
    LELOG("s2apiSetCurrStatus -e");
    return 0;

}

int s2apiGetLatestStatus(lua_State *L) {
    int i = 0, m, n, ret = 0;
    char *strSelfName = NULL;
    int lenSelfName = 0;

    // get self name
    LELOG("s2apiGetLatestStatus -s");
    lenSelfName = lua_tointeger(L, 1);
    if (0 >= lenSelfName) {
        LELOGE("s2apiGetLatestStatus -e1");
        return 0;
    }
    strSelfName = (char *)lua_tostring(L, 2);
    if (NULL == strSelfName) {
        // lua_pushstring(L, strsum);
        LELOGE("s2apiGetLatestStatus -e2");
        return 0;
    }

    for (m = 0; m < MAX_IA && 0 == ret; m++) {
        // LELOG("[%d] [%s] <=> [%s]", m, ginIACache.cache[m].ruleName, strSelfName); 
        if (0 == strcmp(ginIACache.cache[m].ruleName, strSelfName)) {
            int hasNewTbl = 0;
            for (n = 0; n < MAX_RSV_NUM; n++) {
                // LELOG("[%d] [%s] <=> [%s]", n, ginIACache.cache[m].beingReservedUUID[n], strUUID); 
                if (ginIACache.cache[m].beingReservedUUID[n][0] && ginIACache.cache[m].beingReservedStatus[n][0]) {
                    /* create table. */
                    if (!hasNewTbl) {
                        lua_newtable(L);
                        hasNewTbl = 1;
                        ret = 1;
                    }
                    lua_pushnumber(L, i + 1);    //key  
                    lua_pushstring(L, ginIACache.cache[m].beingReservedStatus[n]);  //value  
                    lua_settable(L, -3);       //push key,value  
                	i++;
				}
            }
            break;
        }
    }

    return ret;
}

int sengineSetStatus(char *json, int jsonLen) {
    int ret = 0, ioType = 0;
    uint8_t bin[512] = {0};
    void **hdl = NULL;
    ret = sengineCall((const char *)ginScriptCfg->data.script, ginScriptCfg->data.size, S1_STD2PRI,
        (uint8_t *)json, jsonLen, bin, sizeof(bin));
    if (ret <= 0) {
        LELOGW("sengineSetStatus sengineCall("S1_STD2PRI") [%d]", ret);
        return ret;
    }
    hdl = ioGetHdl(&ioType);
    if (NULL == hdl || *hdl == NULL) {
        LELOG("sengineGetStatus ioGetHdl NULL");
        return -1;
    }
    {
        int i;
        for(i = 0; i < ret; i++) {
            LELOG("bin[%d] = %02x", i, bin[i]);
        }
    }
    ret = ioWrite(ioType, *hdl, bin, ret);
    if (ret <= 0) {
        LELOGW("sengineSetStatus ioWrite [%d]", ret);
        return ret;
    }

    return ret;
}

int sengineGetTerminalProfileCvtType(char *json, int jsonLen) {
    int ret = 0;
    ret = sengineCall((const char *)ginScriptCfg->data.script, ginScriptCfg->data.size, S1_GET_CVTTYPE,
            NULL, 0, (uint8_t *)json, jsonLen);
    if (ret <= 0) {
        LELOGW("sengineGetStatus sengineCall("S1_GET_CVTTYPE") [%d]", ret);
        return ret;
    }
    return ret;
}

int sengineQuerySlave(QuerieType_t type)
{
    Queries queries;
    int ret = 0, i = 0, ioType = 0;
    uint16_t currLen = 0, appendLen = 0;
    void **hdl = NULL;

    // 0. getQueries from script
    ret = sengineCall((const char *)ginScriptCfg->data.script, ginScriptCfg->data.size, S1_GET_QUERIES,
            (uint8_t *)&type, sizeof(type), (uint8_t *)&queries, sizeof(queries));
    if (ret <= 0) {
        LELOGW("sengineGetStatus sengineCall("S1_GET_QUERIES") [%d]", ret);
        return ret;
    }
    hdl = ioGetHdl(&ioType);
    if (NULL == hdl || *hdl == NULL) {
        LELOGE("sengineGetStatus ioGetHdl NULL");
        return -1;
    }
    for (i = 0; i < queries.queriesCountsLen; i += 2, appendLen += currLen) {
        memcpy(&currLen, &queries.arrQueriesCounts[i], 2);
        ret = ioWrite(ioType, *hdl, &(queries.arrQueries[appendLen]), currLen);
        if (ret <= 0) {
            LELOGW("sengineGetStatus ioWrite [%d]", ret);
            return ret;
        }
    }
    return 0;
}

int senginePollingSlave(void) {
    char status[MAX_BUF];
    uint8_t bin[128] = {0};
    int whatKind = 0, ret = 0, size, ioType = 0;
    void **hdl = NULL;

    hdl = ioGetHdl(&ioType);
    if (NULL == hdl || *hdl == NULL) {
        LELOGW("senginePollingSlave ioGetHdl NULL");
        return -1;
    }
    ret = ioRead(ioType, *hdl, bin, sizeof(bin));
    if (ret <= 0) {
        LELOGW("senginePollingSlave ioRead [%d]", ret);
        return ret;
    }
#if 0
    {
        int i;
        LELOGE("ioRead ret = %d", ret);
        for(i = 0; i < ret; i++) {
            LELOGE("bin[%d] = %02x", i, bin[i]);
        }
    }
#endif
    size = ret;
    ret = sengineCall((const char *)ginScriptCfg->data.script, ginScriptCfg->data.size, S1_GET_VALIDKIND,
            bin, size, (uint8_t *)&whatKind, sizeof(whatKind));
    // LELOGE("sengineCall ret = %d, what = %d", ret, whatKind);
    if (ret <= 0) {
        LELOGW("senginePollingSlave sengineCall "S1_GET_VALIDKIND" [%d]", ret);
        return -1;
    }
    switch (whatKind) {
        case 1: {
                extern int resetConfigData(void);
                ret = resetConfigData();
                LELOG("resetConfigData [%d]", ret);
                if (0 <= ret) {
                    halReboot();
                }
            }
            break;
        case 2: {
                extern int lelinkNwPostCmdExt(const void *node);
                int len = 0;
                len = sengineCall((const char *)ginScriptCfg->data.script, ginScriptCfg->data.size, S1_PRI2STD,
                        bin, size, (uint8_t *)status, sizeof(status));
                // LELOGE("sengineCall len = %d. [%s]", len, status);
                if (len <= 0) {
                    LELOGW("senginePollingSlave sengineCall("S1_PRI2STD") [%d]", len);
                } else if (cacheIsChanged(status, len)) {
                    NodeData node = {0};
                    if (isCloudAuthed()) {
                        node.cmdId = LELINK_CMD_CLOUD_HEARTBEAT_REQ;
                        node.subCmdId = LELINK_SUBCMD_CLOUD_STATUS_CHANGED_REQ;
                    } else {
                        char br[MAX_IPLEN] = {0};
                        node.cmdId = LELINK_CMD_DISCOVER_REQ;
                        node.subCmdId = LELINK_SUBCMD_DISCOVER_STATUS_CHANGED_REQ;        
                        ret = halGetBroadCastAddr(br, sizeof(br));
                        if (0 >= ret) {
                            strcpy(br, "255.255.255.255");
                        } else
                            strcpy(node.ndIP, br);
                        node.ndPort = NW_SELF_PORT;
                    }
                    lelinkNwPostCmdExt(&node);
                    cacheSetTerminalStatus(status, len);
                    LELOG("Cache status:%s", status);
                }
            }
            break;
        default:
            LELOGW("Unknow whatKind = %d", whatKind);
            return -3;
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
        LELOGW("sengineS2GetBeingReservedInfo sengineCall("S2_GET_BERESERVED") [%d]", ret);
        return -2;
    }

    count = MIN(*((int *)tmpBeingReserved), MAX_RSV_NUM);
    LELOGW("sengineS2GetBeingReservedInfo count[%d]", count);

    // point to the list of being reserved uuidList
    tmp = (char *)tmpBeingReserved + sizeof(int);
    for (i = 0; i < count; i++) {
        memcpy(strBeingReserved[i], tmp, MAX_UUID);
        tmp += MAX_UUID;
        LELOGW("strBeingReserved[%d][%s]", MAX_UUID, strBeingReserved[i]);
    }

    return count;
}

int sengineS2RuleHandler(const ScriptCfg *scriptCfg2, 
    const char *localJson, int localJsonLen, 
    const char *rmtJson, int rmtJsonLen,
    IA_CACHE_INT *cacheInt) {

    int ret = 0, i, count = 0, isRepeat = 0, is = 0;
    uint8_t buf[MAX_BUF * 2] = {0};
    // char strUUID[64] = {0};
    // char validList[MAX_IA][MAX_UUID + 1] = {0};
    if (NULL == scriptCfg2) {
        return -1;
    }
    // ret = getUUIDFromJson(rmtJson, rmtJsonLen, strUUID, sizeof(strUUID));
    // if (0 >= ret) {
    //     LELOGW("senginePollingRules getUUIDFromJson[%d]", ret);
    //     return -2;
    // }
    /*
     * 1.
     * check if it is a repeat rule
     */
    ret = sengineCall((const char *)scriptCfg2->data.script, scriptCfg2->data.size, S2_GET_RULETYPE,
        NULL, 0, (uint8_t *)buf, sizeof(buf));
    if (0 > ret) {
        LELOGW("sengineS2RuleHandler sengineCall("S2_GET_RULETYPE") [%d]", ret);
        return -4;
    }
    isRepeat = *((int *)buf);
    // isAND = *((int *)buf + 1);
    LELOG("sengineS2RuleHandler sengineCall("S2_GET_RULETYPE") [%d]", isRepeat);
    /*
     * 2. 
     * check all the reserved targets in a rule
     */
    count = sengineS2GetBeingReservedInfo(scriptCfg2, cacheInt->beingReservedUUID);
    if (0 >= count) {
        return 0;
    }
    cacheInt->beingReservedNum = count;
    LELOG("sengineS2RuleHandler reserved NUM[%d]", count);
    for (i = 0; i < 1; i++)
    {
        int hasNewComming = 0, tmpStrLen = 0;
        /*
        * 3. 
        * check if it is a corresponding info(uuid)
        */
        memset(buf, 0, sizeof(buf));
        // LELOG("FOR idx[%d] [%s] <=> [%s]", i, strUUID, cacheInt->beingReservedUUID[i]);
        // 1st param
        if (localJson && 0 <= localJsonLen) {
            LELOG("1st PARAM [%d][%s]", localJsonLen, localJson);
            memcpy(buf, localJson, localJsonLen);
            tmpStrLen = strlen(buf);
            buf[tmpStrLen] = 0;
        } else {
            LELOG("1st no PARAM");
            strcpy(buf, "{}");
            tmpStrLen = strlen(buf);
            buf[tmpStrLen] = 0;
        }

        // 2nd param
        if (rmtJson && 0 <= rmtJsonLen) {
            LELOG("2nd PARAM [%d][%s]", rmtJsonLen, rmtJson);
            memcpy(buf + tmpStrLen + 1, rmtJson, rmtJsonLen);
            tmpStrLen += rmtJsonLen + 1;
            hasNewComming = 1;
        } else {
            LELOG("2nd no PARAM");
            strcpy(buf + tmpStrLen + 1, "{}");
            tmpStrLen += (2 + 1);
        }

        if (!hasNewComming && !isRepeat) {
            continue;
        }

        ret = sengineCall((const char *)scriptCfg2->data.script, scriptCfg2->data.size, S2_GET_ISOK_EXT,
            (uint8_t *)buf, tmpStrLen, (uint8_t *)&is, sizeof(is));
        if (0 > ret) {
            LELOGW("sengineS2RuleHandler sengineCall("S2_GET_ISOK_EXT") [%d]", ret);
            continue;
        }
        if (!is) {
            LELOGW("sengineS2RuleHandler condition NOT OK");
            continue;
        }
        LELOG("sengineS2RuleHandler sengineCall("S2_GET_ISOK_EXT") ok ? [%d]", is);

        /*
         * 4.
         * get the self ctrl cmd
         */
        ret = sengineCall((const char *)scriptCfg2->data.script, scriptCfg2->data.size, S2_GET_BECMD,
            NULL, 0, (uint8_t *)&buf, sizeof(buf));
        if (0 > ret) {
            LELOGW("sengineS2RuleHandler sengineCall("S2_GET_BECMD") [%d]", ret);
            continue;
        }

        // 5. do ctrl
        ret = sengineSetStatus((char *)buf, ret);
        LELOG("sengineS2RuleHandler sengineSetStatus DONE [%d]", ret);      
    }

    LELOG("sengineS2RuleHandler condition -e ");

    return 0;
}


int senginePollingRules(const char *jsonRmt, int jsonLen) {
    int ret, i = 0, is = 0;
    // int64_t utc = 0;
    // uint32_t utcH = 0, utcL = 0;
    // char jsonUTC[64] = {0};
    // char result[MAX_BUF] = {0};
    char strSelfRuleName[MAX_RULE_NAME] = {0};
    char tmpRmtJson[MAX_BUF] = {0};
    int tmpRmtJsonLen = 0;
    char tmpLocalJson[MAX_BUF] = {0};
    int tmpLocalJsonLen = 0;

    PrivateCfg privCfg;
    // char strBeingReserved[64] = {0};

    LELOG("senginePollingRules -s ");
    ret = lelinkStorageReadPrivateCfg(&privCfg);
    if (privCfg.csum != crc8((const uint8_t *)&(privCfg.data), sizeof(privCfg.data))) {
        LELOGW("senginePollingRules lelinkStorageReadPrivateCfg csum FAILED");
        return -1;
    }
    // LELOG("sengineFindRule [%d][%d]", privCfg.data.iaCfg.num);

    if (0 >= privCfg.data.iaCfg.num || MAX_IA < privCfg.data.iaCfg.num) {
        LELOGW("senginePollingRules rules num[%d]", privCfg.data.iaCfg.num);
        return 0;
    }
    LELOGW("senginePollingRules rules num[%d] ", privCfg.data.iaCfg.num);

    // remote json
    if (jsonRmt && 0 < jsonLen) {
        // gen s2 json format
        tmpRmtJsonLen = genS2Json(jsonRmt, jsonLen, jsonRmt, jsonLen, tmpRmtJson, sizeof(tmpRmtJson));
        if (0 >= tmpRmtJsonLen) {
            LELOGW("senginePollingRules genS2Json [%d]", ret);
            return -2;
        }
    } else {
        tmpRmtJsonLen = 0;
    }

    // local json
    tmpLocalJsonLen = getTerminalStatusS2(tmpLocalJson, sizeof(tmpLocalJson));
    if (0 >= tmpLocalJsonLen) {
        LELOGW("senginePollingRules getTerminalStatusS2 [%d]", ret);
        return -3;
    }

    // for every single rule
    for (i = 0; i < privCfg.data.iaCfg.num; i++) {
        if (0 < privCfg.data.iaCfg.arrIA[i]) {
            memset(ginScriptCfg2, 0, sizeof(ScriptCfg));
            ret = lelinkStorageReadScriptCfg(ginScriptCfg2, E_FLASH_TYPE_SCRIPT2, i);
            if (0 > ret) {
                LELOGW("senginePollingRules FAILED arrIA idx[%d]", i);
                continue;
            }
            if (ginScriptCfg2->csum != crc8((uint8_t *)&(ginScriptCfg2->data), sizeof(ginScriptCfg2->data))) {
                LELOGW("senginePollingRules FAILED crc8 idx[%d]", i);
                continue;
            }

            // set the rule's name to cache
            ret = sengineCall((const char *)ginScriptCfg2->data.script, ginScriptCfg2->data.size, S2_GET_SELFNAME,
                NULL, 0, (uint8_t *)&strSelfRuleName, sizeof(strSelfRuleName));
            if (0 > ret) {
                LELOGW("senginePollingRules sengineCall("S2_GET_SELFNAME") [%d]", ret);
                continue;
            }
            memcpy(ginIACache.cache[i].ruleName, strSelfRuleName, MIN(ret, MAX_RULE_NAME));
            ginIACache.cache[i].ruleName[MIN(ret, MAX_RULE_NAME-1)] = 0;

            // check if the rule is valid
            ret = sengineCall((const char *)ginScriptCfg2->data.script, ginScriptCfg2->data.size, S2_IS_VALID,
                (uint8_t *)tmpLocalJson, tmpLocalJsonLen, (uint8_t *)&is, sizeof(is));
            if (0 > ret) {
                LELOGW("senginePollingRules sengineCall("S2_IS_VALID") [%d]", ret);
                continue;
            }
            if (!is) {
                LELOG("senginePollingRules invalid rule ");
                continue;
            }

            sengineS2RuleHandler(ginScriptCfg2, tmpLocalJson, tmpLocalJsonLen, 
                tmpRmtJsonLen > 0 ? tmpRmtJson : NULL, tmpRmtJsonLen, &(ginIACache.cache[i]));

        }
    }
    LELOG("senginePollingRules -e ");

    return 0;
}

int sengineRemoveRules(const char *name) {
    LELOG("sengineRemoveRules -s ");
    LELOG("sengineRemoveRules -e ");
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


