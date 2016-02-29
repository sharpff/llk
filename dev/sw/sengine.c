#include "leconfig.h"
#include "sengine.h"

#include "io.h"

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>

#define MAX_BUF (1024+256)
// #define MAX_STATUS 64
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) < (b)) ? (b) : (a))

ScriptCfg ginScriptCfg;
static uint32_t ginDelayMS;
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

static IO lf_s1GetQueries_input(lua_State *L, const uint8_t *input, int inputLen) {
    // lua_pushlstring(L, (char *)input, inputLen);
    IO io = { 0, 4 };
    return io;
}
static int lf_s1GetQueries(lua_State *L, uint8_t *output, int outputLen) {
    // int i = 0;
    int size = 0, i, j;
    uint8_t *tmp = 0;
    uint16_t currLen = 0, appendLen = 0;
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

    for (i = 0; i < queries->queriesCountsLen; i += 2) {
        LEPRINTF("[SENGINE]_s1GetQueries_[%d]_cmd: ", i/2);
        memcpy(&currLen, &queries->arrQueriesCounts[i], 2);
        for (j = 0; j < currLen; j++) {
            LEPRINTF("%02x ", queries->arrQueries[j + appendLen]);
        }
        appendLen += currLen;
        LEPRINTF("\r\n");
    }
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
    LEPRINTF("[SENGINE] s1GetValidKind: [%d]\r\n", *((int *)output));
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
        LEPRINTF("[SENGINE] s1CvtPri2Std: [%d][%s]\r\n", size, output);
    } else {
        size = 0;
    }

    return size;
}

static IO lf_s1GetRuleInfo_input(lua_State *L, const uint8_t *input, int inputLen) {
    IO io = { 0, 2 };
    return io;
}
static int lf_s1GetRuleInfo(lua_State *L, uint8_t *output, int outputLen) {
    /* cmd */
    int sLen = lua_tointeger(L, -2);
    int size = MIN(sLen, outputLen);
    const char *tmp = (const char *)lua_tostring(L, -1);
    if (tmp && 0 < size) {
        memcpy(output, tmp, size);
        LEPRINTF("[SENGINE] s1GetRuleInfo: [%d][%s]\r\n", size, output);
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
    IO io = { 0, 2 };
    return io;
}
static int lf_s2GetBeingReservedInfo(lua_State *L, uint8_t *output, int outputLen) {
    /* cmd */
    int sLen = lua_tointeger(L, -2);
    int size = MIN(sLen, outputLen);
    const char *tmp = (const char *)lua_tostring(L, -1);
    if (tmp && 0 < size) {
        memcpy(output, tmp, size);
        LEPRINTF("[SENGINE] s2GetBeingReservedInfo: [%d][%s]\r\n", size, output);
    } else {
        size = 0;
    }

    return size;
}

static IO lf_s2IsConditionOK_input(lua_State *L, const uint8_t *input, int inputLen) {
    lua_pushlstring(L, (char *)input, inputLen);
    IO io = { 1, 1 };
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
    { S2_GET_RULEINFO, { lf_s1GetRuleInfo_input, lf_s1GetRuleInfo } },
    { S2_IS_VALID, { lf_s2IsValid_input, lf_s2IsValid } },
    // { S2_IS_VALID_EXT, { lf_s2IsValidExt_input, lf_s2IsValidExt } },
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
    int ret = lelinkStorageReadScriptCfg(&ginScriptCfg, 0);
    if (0 > ret) {
        return -1;
    }

    if (ginScriptCfg.csum != crc8((uint8_t *)&(ginScriptCfg.data), sizeof(ginScriptCfg.data))) {
        LELOG("ScriptCfg read from flash failed\r\n");
        return -2;
    }

    return 0;
}

int bitshift(lua_State *L);
int bitshiftL(lua_State *L);
int bitor(lua_State *L);
int bitxor(lua_State *L);
int bitnor(lua_State *L);
int bitand(lua_State *L);
// int csum(lua_State *L);

int sengineCall(const char *script, int scriptSize, const char *funcName, const uint8_t *input, int inputLen, uint8_t *output, int outputLen)
{
    int ret = 0;
    lua_State *L = luaL_newstate();

    lua_register(L, "bitshift", bitshift);
    lua_register(L, "bitshiftL", bitshiftL);
    lua_register(L, "bitand", bitand);
    lua_register(L, "bitor", bitor);
    lua_register(L, "bitxor", bitxor);
    lua_register(L, "bitnor", bitnor);
    // lua_register(L, "csum", csum);

    if (script == NULL || scriptSize <= 0) {
        return -1;
    }

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

int sengineSetStatus(char *json, int jsonLen) {
    int ret = 0;
    uint8_t bin[512] = {0};
    ret = sengineCall((const char *)ginScriptCfg.data.script, ginScriptCfg.data.size, "s1CvtStd2Pri",
        (uint8_t *)json, jsonLen, bin, sizeof(bin));
    if (ret <= 0) {
        LELOGW("sengineSetStatus sengineCall(s1CvtStd2Pri) [%d]\r\n", ret);
        return ret;
    }

    ret = ioWrite(IO_TYPE_UART, *((void **)ioGetHdl(IO_TYPE_UART)), bin, ret);
    if (ret <= 0) {
        LELOGW("sengineSetStatus ioWrite [%d]\r\n", ret);
        return ret;
    }

    return ret;
}

int sengineGetStatus(char *json, int jsonLen) {
    Queries queries;
    uint8_t bin[128] = {0};
    // static uint8_t bin[1024*9] = {0};
    int ret = 0, i = 0;
    uint16_t currLen = 0, appendLen = 0;


    // 0. getQueries from script
    ret = sengineCall((const char *)ginScriptCfg.data.script, ginScriptCfg.data.size, "s1GetQueries",
            NULL, 0, (uint8_t *)&queries, sizeof(queries));
    if (ret <= 0) {
        LELOGW("sengineGetStatus sengineCall(s1GetQueries) [%d]\r\n", ret);
        return ret;
    }

    // 1. write & read uart
    for (i = 0; i < queries.queriesCountsLen; i += 2) {
        memcpy(&currLen, &queries.arrQueriesCounts[i], 2);
        ret = ioWrite(IO_TYPE_UART, *((void **)ioGetHdl(IO_TYPE_UART)), &(queries.arrQueries[appendLen]), currLen);
        if (ret <= 0) {
            LELOGW("sengineGetStatus ioWrite [%d]\r\n", ret);
            return ret;
        }
        delayms(ginDelayMS);
        ret = ioRead(IO_TYPE_UART, *((void **)ioGetHdl(IO_TYPE_UART)), bin, sizeof(bin));
        if (ret <= 0) {
            LELOGW("sengineGetStatus ioRead [%d]\r\n", ret);
            return ret;
        }
        appendLen += currLen;
    }

    // 2. engine parsing pri2std
    ret = sengineCall((const char *)ginScriptCfg.data.script, ginScriptCfg.data.size, "s1CvtPri2Std",
        bin, ret, (uint8_t *)json, jsonLen);
    if (ret <= 0) {
        LELOGW("sengineGetStatus sengineCall(s1CvtPri2Std) [%d]\r\n", ret);
    }

    return ret;
}

int sengineGetTerminalProfileCvtType(char *json, int jsonLen) {
    int ret = 0;
    ret = sengineCall((const char *)ginScriptCfg.data.script, ginScriptCfg.data.size, "s1GetCvtType",
            NULL, 0, (uint8_t *)json, jsonLen);
    if (ret <= 0) {
        LELOGW("sengineGetStatus sengineCall(s1GetCvtType) [%d]\r\n", ret);
        return ret;
    }
    return ret;
}

int senginePollingSlave(void) {
    int ret = 0;
    int whatKind = 0;
    uint8_t bin[128] = {0};
    ret = ioRead(IO_TYPE_UART, *((void **)ioGetHdl(IO_TYPE_UART)), bin, sizeof(bin));
    if (ret <= 0) {
        return 0;
    }

    ret = sengineCall((const char *)ginScriptCfg.data.script, ginScriptCfg.data.size, "s1GetValidKind",
            bin, ret, (uint8_t *)&whatKind, sizeof(whatKind));
    // LELOG("senginePollingSlave sengineCall(whatKind) [%d][%d]\r\n", ret, whatKind);
    if (ret <= 0) {
        return 0;
    }

	// TODO: cache or status checking

    return whatKind;
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


