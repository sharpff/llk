#include "leconfig.h"
#include "sengine.h"

#include "io.h"
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "ota.h"
#include "state.h"
#include "protocol.h"
#include "cache.h"
#include "misc.h"
#include "jsonv2.h"

#ifndef LOG_SENGINE
#ifdef LELOG
#undef LELOG
#define LELOG(...)
#endif

#ifdef LELOGW
#undef LELOGW
#define LELOGW(...)
#endif

// // #ifdef LELOGE
// // #undef LELOGE
// // #define LELOGE(...)
// // #endif

#ifdef LEPRINTF
#undef LEPRINTF
#define LEPRINTF(...)
#endif
#endif

// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>

#define MAX_SDEV_NUM 64
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) < (b)) ? (b) : (a))


#define FOR_EACH_IO_HDL_START \
    IOHDL *ioHdl = NULL; \
    int x = 0; \
    ioHdl = ioGetHdlExt(); \
    if (NULL == ioHdl) { \
        return -1; \
    } \
    for (x = 0; x < ioGetHdlCounts(); x++) { \
        ginCurrCvtType = ioHdl[x].ioType; \
        if (NULL == ioHdl[x].hdl) { \
            continue; \
        }

#define FOR_EACH_IO_HDL_END \
    }

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

typedef enum {
    SDEV_BUF_TYPE_NONE,
    SDEV_BUF_TYPE_STATUS,
    SDEV_BUF_TYPE_INFO
}SDEV_BUF_TYPE;

IA_CACHE ginIACache;
ScriptCfg *ginScriptCfg;
ScriptCfg *ginScriptCfg2;
static uint32_t ginDelayMS;
static int ginCurrCvtType;

PCACHE sdevCache() {
    static CACHE cache;
    return &cache;
}

SDevNode *sdevArray() {
    static SDevNode *ginArrSDev;

    if (NULL == ginArrSDev) {
        ginArrSDev = (SDevNode *)halCalloc(MAX_SDEV_NUM, sizeof(SDevNode));
        if (ginArrSDev) {
            sdevCache()->maxsize = MAX_SDEV_NUM;
            sdevCache()->singleSize = sizeof(SDevNode);
            sdevCache()->pBase = ginArrSDev;
        } else {
            LELOGE("Heap is not enough!!!");
        }
    }
    // LELOG("[SENGINE] sdevArray size[%d/%d], singleSize[%d], pBase[0x%p]", sdevCache()->currsize, sdevCache()->maxsize, sdevCache()->singleSize, sdevCache()->pBase);
    return ginArrSDev;
}

static void sdevArraySet(int index, const SDevNode *node, SDEV_BUF_TYPE bufType) {
    if (sdevCache()->currsize == sdevCache()->maxsize || 
        0 > index) {
        return;
    }
    if (!sdevArray()[index].occupied) {
        sdevCache()->currsize++;
        sdevArray()[index].occupied = 1;
    }
    switch (bufType) {
        case SDEV_BUF_TYPE_INFO: {
                memcpy(&sdevArray()[index].mac, node->mac, sizeof(node->mac));
                memcpy(&(sdevArray()[index]).sdevInfo, node->sdevInfo, sizeof(node->sdevInfo));
            }break;
        case SDEV_BUF_TYPE_STATUS: {
                memcpy(&(sdevArray()[index]).sdevStatus, node->sdevStatus, sizeof(node->sdevStatus));
            }break;
        default: 
            // LELOGE("NO MATCHED BUF TYPE FOR SDEV");
            break;
    }
}

static int sdevArrayGet(int index, SDevNode *node) {
    if (0 > index && !node) {
        return -1;
    }
    // if (!sdevArray()[index].occupied) {
    //     return -2;
    // }

    memcpy(node, &(sdevArray()[index]), sizeof(SDevNode));
    return index;
}

static void sdevArrayReset() {
    SDevNode *arr = sdevArray();
    if (arr) {
        sdevCache()->currsize = 0;
        memset(arr, 0, MAX_SDEV_NUM*sizeof(SDevNode));
    }
}

static int forEachNodeSDevCB(SDevNode *currNode, void *uData) {
    LELOG("[SENGINE] forEachNodeSDevCB currNode->mac[%s] uData[%s]", currNode->mac, uData);
    if (0 == strcmp(currNode->mac, (char *)uData)) {
        return 1;
    }
    return 0;
}

static void postStatusChanged(int plusIdx) {
    int ret = 0;
    NodeData node = {0};
    node.reserved = plusIdx;
    // TIMEOUT_SECS_BEGIN(1)
    if (isCloudAuthed() && getLock()) {
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
    // TIMEOUT_SECS_END
}

static int sdevInsert(SDevNode *arr, const char *status, int len) {
    int valJoin = 0, index = 0, ret = 0;
    jobj_t jobj;
    jsontok_t jsonToken[NUM_TOKENS];
    char sDev[MAX_BUF] = {0};
    SDevNode node;

    memset(&node, 0, sizeof(SDevNode));
    ret = json_init(&jobj, jsonToken, NUM_TOKENS, (char *)status, len);
    if (WM_SUCCESS != ret) {
        return -1;
    }

    // for sub dev join ind
    if (WM_SUCCESS == json_get_val_int(&jobj, JSON_NAME_SDEV_JOIN, &valJoin)) {
        LELOG("sdevInsert join START ****************************");
        if (2 != valJoin) {
            return -2;
        }

        if (0 > getJsonObject(status, len, JSON_NAME_SDEV, sDev, sizeof(sDev))) {
            LELOGE("sdevInsert getJsonObject [%s] FAILED", JSON_NAME_SDEV);
            return -3;
        }

        ret = json_get_composite_object(&jobj, JSON_NAME_SDEV);
        if (0 == ret) {

            if (WM_SUCCESS != json_get_val_str(&jobj, JSON_NAME_SDEV_MAC, node.mac, sizeof(node.mac))) {
                LELOGE("sdevInsert json_get_val_str [%s] FAILED", JSON_NAME_SDEV_MAC);
                return -4;
            }

            index = qForEachfromCache(sdevCache(), (int(*)(void*, void*))forEachNodeSDevCB, node.mac);
            if (0 <= index) {
                LELOG("sdevInsert qForEachfromCache already EXIST [%d]", index);
                return 0;                
            }
            strcpy(node.sdevInfo, sDev);
            LELOG("sdevInsert index[%d] mac[%s] sdev[%s]", index, node.mac, node.sdevInfo);
            node.occupied = 1;
            sdevArraySet(index, &node, SDEV_BUF_TYPE_INFO);
            // TODO: send HELLO for sdev
        }
        LELOG("sdevInsert join END ****************************");
    }
    return 0;
}

static int sdevUpdate(SDevNode *arr, const char *status, int len) {
    // {"subDevGetList":[0,1,2]}
    jobj_t jobj;
    jsontok_t jsonToken[NUM_TOKENS];
    // char name[MAX_RULE_NAME] = {0};
    int ret = 0;
    int sDevIdx = 0, num = 0, i = 0;
    char buf[MAX_BUF] = {0};
    SDevNode node;

    memset(&node, 0, sizeof(SDevNode));
    ret = json_init(&jobj, jsonToken, NUM_TOKENS, (char *)status, len);
    if (WM_SUCCESS != ret) {
        return -1;
    }

    // for sub dev list rsp
    if((ret = json_get_array_object(&jobj, JSON_NAME_SDEV_GET_LIST, &num)) == WM_SUCCESS) {
        LELOG("sdevUpdate list START ****************************");
        num = num < 0 ? 0 : num;
        // refresh the array
        if (num < sdevCache()->currsize) {
            LELOG("sdevArrayReset.......................num [%d], array currsize[%d]", num, sdevCache()->currsize);
            sdevArrayReset();
        }
        for(i = 0; i < num; i++) {
            if((ret = json_array_get_int(&jobj, i, &sDevIdx)) != WM_SUCCESS) {
                continue;
            }
            // TODO: update the list
            // LELOG("sDev get list for idx[%d]", sDevIdx);
            if (0 <= sdevArrayGet(sDevIdx, &node) && !(node.occupied)) {
                node.occupied = 1;
                sdevArraySet(sDevIdx, &node, SDEV_BUF_TYPE_NONE);             
            }

            // qEnCache(sdevCache(), &node);
        }
        LELOG("sdevUpdate list END ****************************");
        return 0;
    }

    // for sub dev info rsp
    if (WM_SUCCESS == json_get_val_int(&jobj, JSON_NAME_SDEV_GET_INFO, &sDevIdx)) {
        LELOG("sdevUpdate info START ****************************");
        if (0 > getJsonObject(status, len, JSON_NAME_SDEV, buf, sizeof(buf))) {
            LELOGE("getJsonObject [%s] FAILED", JSON_NAME_SDEV);
            return -2;
        }

        // ret = getJsonObject(json, jsonLen, JSON_NAME_SDEV, buf, sizeof(buf));
        // if (0 < ret) {
        ret = json_get_composite_object(&jobj, JSON_NAME_SDEV);
        if (0 == ret) {
            if (0 <= sdevArrayGet(sDevIdx, &node) && node.occupied) {
                if (WM_SUCCESS != json_get_val_str(&jobj, JSON_NAME_SDEV_MAC, node.mac, sizeof(node.mac))) {
                    LELOGE("json_get_val_str [%s] FAILED", JSON_NAME_SDEV_MAC);
                    return -3;
                }

                strcpy(node.sdevInfo, buf);
                LELOG("=> sDevIdx[%d] mac[%s] sdev[%s] sdevStatus[%s]", sDevIdx, node.mac, node.sdevInfo, node.sdevStatus);
                sdevArraySet(sDevIdx, &node, SDEV_BUF_TYPE_INFO);            
            } else {
                LELOGE("unexpect ....");
                return -4;
            }
            // qEnCache(sdevCache(), &node);

        }
        LELOG("sdevUpdate info END ****************************");
    }

        // for sub dev status ind
    if (0 < getJsonObject(status, len, JSON_NAME_SDEV_STATUS, buf, sizeof(buf))) {
        LELOG("sdevUpdate sdevStatus START ****************************");
        ret = json_get_composite_object(&jobj, JSON_NAME_SDEV);
        if (0 == ret) {
            if (0 <= sdevArrayGet(sDevIdx, &node) && node.occupied) {
                if (WM_SUCCESS != json_get_val_str(&jobj, JSON_NAME_SDEV_MAC, node.mac, sizeof(node.mac))) {
                    LELOGE("json_get_val_str [%s] FAILED", JSON_NAME_SDEV_MAC);
                    return -5;
                }
                LELOG("mac is [%s]", node.mac);
                sDevIdx = qForEachfromCache(sdevCache(), (int(*)(void*, void*))forEachNodeSDevCB, node.mac);
                if (0 <= sDevIdx) {
                    LELOG("old[%s] new[%s]", node.sdevStatus, buf);
                    if (0 != memcmp(node.sdevStatus, buf, strlen(buf))) {
                        postStatusChanged(sDevIdx + 1);
                    }
                    memset(node.sdevStatus, 0, sizeof(node.sdevStatus));
                    strcpy(node.sdevStatus, buf);
                    sdevArraySet(sDevIdx, &node, SDEV_BUF_TYPE_STATUS);  
                    LELOG("=> sDevIdx[%d] mac[%s] sdev[%s] sdevStatus[%s]", sDevIdx, node.mac, node.sdevInfo, node.sdevStatus);               
                }
            }
        }
        LELOG("sdevUpdate sdevStatus END ****************************");
    }

    return 0;
}


static IO lf_s1OptDoSplit_input(lua_State *L, const uint8_t *input, int inputLen) {

    lua_pushlstring(L, (char *)input, inputLen);
    IO io = { 1, 4 };
    return io;
}
static int lf_s1OptDoSplit(lua_State *L, uint8_t *output, int outputLen) {
    int size = 0;// i, j;
    uint8_t *tmp = 0;
    // uint16_t currLen = 0, appendLen = 0;
    Datas *datas = (Datas *)output;

    if (NULL == datas || sizeof(Datas) > outputLen) {
        return 0;
    }

    datas->datasCountsLen = lua_tointeger(L, -4);
    size = MIN(datas->datasCountsLen, sizeof(datas->arrDatasCounts));
    tmp = (uint8_t *)lua_tostring(L, -3);
    if (tmp && 0 < size) {
        memcpy(datas->arrDatasCounts, tmp, size);
    } else {
        size = 0;
    }

    datas->datasLen = lua_tointeger(L, -2);
    size = MIN(datas->datasLen, sizeof(datas->arrDatas));
    tmp = (uint8_t *)lua_tostring(L, -1);
    if (tmp && 0 < size) {
        memcpy(datas->arrDatas, tmp, size);
    } else {
        size = 0;
    }

    // test only
    // {
    //     int currLen = 0, i = 0, j = 0, appendLen = 0;
    //     LELOG("[SENGINE]_s1OptDoSplit_datasCountsLen[%d] ", datas->datasCountsLen);
    //     for (i = 0; i < datas->datasCountsLen; i += sizeof(uint16_t)) {
    //         memcpy(&currLen, &datas->arrDatasCounts[i/sizeof(uint16_t)], sizeof(uint16_t));
    //         LEPRINTF("[SENGINE]_s1OptDoSplit_[%d]_cmd: curr piece len[%d]", i/sizeof(uint16_t), currLen);
    //         for (j = 0; j < currLen; j++) {
    //             LEPRINTF("%02x ", datas->arrDatas[j + appendLen]);
    //         }
    //         appendLen += currLen;
    //         LEPRINTF("\r\n");
    //     }
    // }
    
    return sizeof(Datas);
}

static IO lf_s1GetQueries_input(lua_State *L, const uint8_t *input, int inputLen) {
    lua_Integer tmp = *(uint8_t *)input;
    lua_pushinteger(L, tmp);
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

    // test only
    // {
    //     int currLen = 0, i = 0, j = 0, appendLen = 0;
    //     for (i = 0; i < queries->queriesCountsLen; i += sizeof(uint16_t)) {
    //         LEPRINTF("[SENGINE]_s1GetQueries_[%d]_cmd: ", i/sizeof(uint16_t));
    //         memcpy(&currLen, &queries->arrQueriesCounts[i/sizeof(uint16_t)], sizeof(uint16_t));
    //         for (j = 0; j < currLen; j++) {
    //             LEPRINTF("%02x ", queries->arrQueries[j + appendLen]);
    //         }
    //         appendLen += currLen;
    //         LEPRINTF("\r\n");
    //     }
    // }
    
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
    // LELOG("[SENGINE] s1GetCvtType: [%d][%s] ginDelayMS[%d]", size, output, ginDelayMS);
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

static IO lf_s1OptHasSubDevs_input(lua_State *L, const uint8_t *input, int inputLen) {
    // lua_pushlstring(L, (char *)input, inputLen);
    IO io = { 0, 1 };
    return io;
}
static int lf_s1OptHasSubDevs(lua_State *L, uint8_t *output, int outputLen) {
    // int i = 0;
    *((int*)output) = lua_tointeger(L, -1);
    // LELOG("[SENGINE] s1HasSubDevs [%d]", *((int*)output));
    return sizeof(int);
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

static IO lf_s1OptMergeCurrStatus2Action_input(lua_State *L, const uint8_t *input, int inputLen) {
    // int firstLen = 0;
    // int secondLen = 0;

    // firstLen = strlen(input);
    // if (0 == firstLen) {
    //     // lua_pushlstring(L, NULL, 0);
    // } else {
    //     lua_pushlstring(L, (char *)input, firstLen);
    // }
    // LELOG("[SENGINE] lf_s1OptMergeCurrStatus2Action_input: firstLen[%d/%d][%s]", firstLen, inputLen, input);
    // firstLen += 1;
    // secondLen = strlen((char *)input + firstLen);
    // if (0 == secondLen) {
    //     // lua_pushlstring(L, NULL, 0);
    // } else {
    //     lua_pushlstring(L, (char *)input + firstLen, secondLen);
    // }
    // LELOG("[SENGINE] lf_s1OptMergeCurrStatus2Action_input: secondLen[%d/%d][%s]", secondLen, inputLen, input + firstLen);
    // IO io = { 2, 2 };
    int firstLen = 0;
    int secondLen = 0;
    const char *empty = "{}";
    firstLen = strlen(input);
    if (0 == firstLen) {
        lua_pushlstring(L, empty, 2);
    } else {
        lua_pushlstring(L, (char *)input, firstLen);
    }
    LELOG("[SENGINE] lf_s1OptMergeCurrStatus2Action_input: firstLen[%d/%d][%s]", firstLen, inputLen, input);
    firstLen += 1;
    secondLen = strlen((char *)input + firstLen);
    if (0 == secondLen) {
        lua_pushlstring(L, empty, 2);
    } else {
        lua_pushlstring(L, (char *)input + firstLen, secondLen);
    }
    LELOG("[SENGINE] lf_s1OptMergeCurrStatus2Action_input: secondLen[%d/%d][%s]", secondLen, inputLen, input + firstLen);
    IO io = { 2, 2 };
    return io;
}
static int lf_s1OptMergeCurrStatus2Action(lua_State *L, uint8_t *output, int outputLen) {
    /* cmd */
    int sLen = lua_tointeger(L, -2);
    int size = MIN(sLen, outputLen);
    const char *tmp = (const char *)lua_tostring(L, -1);
    if (tmp && 0 < size) {
        memcpy(output, tmp, size);
        LELOG("[SENGINE] lf_s1OptMergeCurrStatus2Action: [%d][%s]", size, output);
    } else {
        size = 0;
    }

    return size;
    // *((int *)output) = lua_tointeger(L, -1);

    // return sizeof(int);

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
        LELOG("[SENGINE] s1GetVer: [%d][%s]", size, output);
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
        // LELOGW("[SENGINE] s1CvtPri2Std: [%d][%s]", size, output);
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
    { S1_OPT_HAS_SUBDEVS, { lf_s1OptHasSubDevs_input, lf_s1OptHasSubDevs } },
    { S1_GET_QUERIES, { lf_s1GetQueries_input, lf_s1GetQueries } },
    { S1_OPT_DO_SPLIT, { lf_s1OptDoSplit_input, lf_s1OptDoSplit } },
    { S1_STD2PRI, { lf_s1CvtStd2Pri_input, lf_s1CvtStd2Pri } },
    { S1_PRI2STD, { lf_s1CvtPri2Std_input, lf_s1CvtPri2Std } },
    { S1_GET_VALIDKIND, { lf_s1GetValidKind_input, lf_s1GetValidKind } },
    { S1_OPT_MERGE_ST2ACT, { lf_s1OptMergeCurrStatus2Action_input, lf_s1OptMergeCurrStatus2Action } },
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
    if (sengineHasDevs()) {
        if (!sdevArray()) {
            LELOGE("sdevArray is Failed");
            return -3;
        }
    }
    LELOG("sengineInit Done");
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
int s1apiGetCurrCvtType(lua_State *L);
int s1apiSdevGetUserDataByMac(lua_State *L);
int s1apiSDevGetMacByUserData(lua_State *L);

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
    lua_register(L, "s1apiGetCurrCvtType", s1apiGetCurrCvtType);
    lua_register(L, "s1apiSdevGetUserDataByMac", s1apiSdevGetUserDataByMac);
    lua_register(L, "s1apiSDevGetMacByUserData", s1apiSDevGetMacByUserData);
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
            if (strcmp(S1_OPT_HAS_SUBDEVS, funcName) && 
                strcmp(S1_OPT_MERGE_ST2ACT, funcName) && 
                strcmp(S1_OPT_DO_SPLIT, funcName))
                LELOGE("[lua engine] lua error: %s => %s", err, funcName);
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

int sengineHasDevs(void) {
    int ret = 0, hasDevs = 0;
    ret = sengineCall((const char *)ginScriptCfg->data.script, ginScriptCfg->data.size, S1_OPT_HAS_SUBDEVS,
            NULL, 0, (uint8_t *)&hasDevs, sizeof(hasDevs));
    if (ret <= 0) {
        // LELOGW("sengineHasDevs sengineCall("S1_OPT_HAS_SUBDEVS") [%d]", ret);
        return 0;
    }
    return hasDevs;
}

int s1apiGetCurrCvtType(lua_State *L) {
    lua_pushnumber(L, ginCurrCvtType);    //key  
    return 1;
}


static int forEachNodeSDevThruMacCB(SDevNode *currNode, void *uData) {
    LELOG("[SENGINE] forEachNodeSDevThruMacCB [0x%p]", uData);
    if (0 == strcmp(currNode->mac, (char *)uData)) {
        return 1;
    }
    return 0;
}

int s1apiSdevGetUserDataByMac(lua_State *L) {
    char *strMac = NULL;
    int lenMac = 0, ret = 0;
    lenMac = lua_tointeger(L, 1);
    if (0 >= lenMac) {
        LELOGE("s1apiSdevGetUserDataByMac -e1");
        return 0;
    }
    strMac = (char *)lua_tostring(L, 2);
    if (NULL == strMac) {
        LELOGE("s1apiSdevGetUserDataByMac -e2");
        return 0;
    }

    ret = qForEachfromCache(sdevCache(), (int(*)(void*, void*))forEachNodeSDevThruMacCB, strMac);
    LELOG("ret[%d] mac[%d][%s]", ret, lenMac, strMac);

    lua_pushinteger(L, ret);
    return 1;
}

int s1apiSDevGetMacByUserData(lua_State *L) {
    char *strMac = NULL;
    int lenMac = 0;
    SDevNode *arr = sdevArray();
    int userData = lua_tointeger(L, 1);
    if (0 > userData) {
        LELOGE("s1apiSDevGetMacByUserData -e1");
        return 0;
    }

    // test only
    // {
    //     arr[userData].occupied = 1;
    //     strcpy(arr[userData].mac, "asdfasdf");
    // }

    LELOG("userData[%d] occupied[%d]", userData, arr[userData].occupied);

    if (0 == arr[userData].occupied) {
        LELOGE("s1apiSDevGetMacByUserData -e2");
        return 0;
    }

    {
        int i = 0, tmpNum = 0;
        SDevNode *arr = sdevArray();
        for (i = 0; i < sdevCache()->maxsize && tmpNum < sdevCache()->currsize; i++) {
            if (arr[i].occupied) {
                LELOG("i[%d] mac[%s] sdev[%s] sdevStatus[%s]", i, arr[i].mac, arr[i].sdevInfo, arr[i].sdevStatus);
                tmpNum++;
            }
        }
    }

    lenMac = MIN(strlen(arr[userData].mac), sizeof(arr[userData].mac));
    strMac = arr[userData].mac;

    LELOG("lenMac[%d] strMac[%s]", lenMac, strMac);

    if (0 < lenMac && NULL != strMac) {        
        lua_pushinteger(L, lenMac);
        lua_pushstring(L, strMac);
        return 2;
    }

    LELOGE("s1apiSDevGetMacByUserData -e2");
    return 0;
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
    int ret = 0;
    uint8_t bin[512] = {0};
    char jsonMerged[2*MAX_BUF] = {0};
    // char *jsonOut = json;
    // int jsonOutLen = jsonLen;
    // IOHDL *ioHdl = NULL;
    // int x = 0;

    // ioHdl = ioGetHdlExt();
    // if (NULL == ioHdl) {
    //     LELOGW("ioGetHdlExt NULL");
    //     return -1;
    // }

    // for (x = 0; x < ioGetHdlCounts(); x++) {
    //     ginCurrCvtType = ioHdl[x].ioType;
    //     if (NULL == ioHdl[x].hdl) {
    //         continue;
    //     }

    FOR_EACH_IO_HDL_START;
        memcpy(jsonMerged, json, jsonLen);
        jsonMerged[jsonLen] = 0;
        ret = jsonLen + 1;
        ret = sengineGetStatus(&jsonMerged[ret], sizeof(jsonMerged) - ret); 
        if (0 < ret) {
            ret = jsonLen + 1 + ret;
            /*
             * jsonMerged includes 2 params before sengineCall. 1st is action, 2nd is current status
             */
            ret = sengineCall((const char *)ginScriptCfg->data.script, ginScriptCfg->data.size, S1_OPT_MERGE_ST2ACT,
                (uint8_t *)jsonMerged, ret, (uint8_t *)jsonMerged, sizeof(jsonMerged));
            if (0 < ret) {
                jsonLen = ret;
            }
            LELOGW("sengineSetStatus sengineCall("S1_OPT_MERGE_ST2ACT") [%d] [%d][%s]", ret, jsonLen, jsonMerged);
        }

        ret = sengineCall((const char *)ginScriptCfg->data.script, ginScriptCfg->data.size, S1_STD2PRI,
            (uint8_t *)jsonMerged, jsonLen, bin, sizeof(bin));
        if (ret <= 0) {
            LELOGW("sengineSetStatus sengineCall("S1_STD2PRI") [%d]", ret);
            continue;
        }

        {
            extern int bytes2hexStr(const uint8_t *src, int srcLen, uint8_t *dst, int dstLen);
            uint8_t  hexStr[96] = {0};
            bytes2hexStr(bin, ret, hexStr, sizeof(hexStr));
            LELOG("bin[%s]", hexStr);
        }

        ret = ioWrite(ioHdl[x].ioType, ioHdl[x].hdl, bin, ret);
        if (ret <= 0) {
            LELOGW("sengineSetStatus ioWrite [%d]", ret);
            continue;
        }
    FOR_EACH_IO_HDL_END;
    
    // }

    return ret;
}

int sengineGetStatus(char *status, int len) {
    int ret;
    ret = cacheGetTerminalStatus(status, len);
    if(0 >= ret) {
        // LELOGW("Can't get cache status");
        strcpy(status, "{}");
        ret = 2;
    }
    LELOG("sengineGetStatus [%d][%s]", ret, status);
    return ret;
}

int sengineGetTerminalProfileCvtType(char *json, int jsonLen) {
    int ret = 0;
    ret = sengineCall((const char *)ginScriptCfg->data.script, ginScriptCfg->data.size, S1_GET_CVTTYPE,
            NULL, 0, (uint8_t *)json, jsonLen);
    if (ret <= 0) {
        LELOG("sengineGetTerminalProfileCvtType sengineCall("S1_GET_CVTTYPE") [%d]", ret);
        return ret;
    }
    return ret;
}

int sengineQuerySlave(QuerieType_t type)
{
    Queries queries;
    int ret = 0, i = 0;
    uint16_t currLen = 0, appendLen = 0;

    FOR_EACH_IO_HDL_START;
        ret = sengineCall((const char *)ginScriptCfg->data.script, ginScriptCfg->data.size, S1_GET_QUERIES,
                (uint8_t *)&type, sizeof(type), (uint8_t *)&queries, sizeof(queries));

        if (ret <= 0) {
            LELOGW("sengineQuerySlave sengineCall("S1_GET_QUERIES") [%d]", ret);
            continue;
        }

        for (i = 0; i < queries.queriesCountsLen; i += 2, appendLen += currLen) {
            memcpy(&currLen, &queries.arrQueriesCounts[i], 2);
            ret = ioWrite(ioHdl[x].ioType, ioHdl[x].hdl, &(queries.arrQueries[appendLen]), currLen);
            if (ret <= 0) {
                LELOGW("sengineQuerySlave ioWrite [%d]", ret);
                break;
            }
        }

        // query for sub dev
        if (sengineHasDevs()) {
            TIMEOUT_SECS_BEGIN(5)
                char json[256] = {0};
                uint8_t cmd[128] = {0};
                sprintf(json, "{\"%s\":1}", JSON_NAME_SDEV_GET_LIST);
                LELOG("json is [%s]", json);
                ret = sengineCall((const char *)ginScriptCfg->data.script, ginScriptCfg->data.size, S1_STD2PRI,
                    (uint8_t *)json, strlen(json), (uint8_t *)cmd, sizeof(cmd));
                if (ret <= 0) {
                    LELOGW("[SUBDEV] sengineQuerySlave sengineCall("S1_STD2PRI") [%d]", ret);
                    continue;
                }
                ret = ioWrite(ioHdl[x].ioType, ioHdl[x].hdl, cmd, ret);
                if (0 >= ret) {
                    LELOGW("[SUBDEV] sengineQuerySlave ioWrite [%d]", ret);
                }
                {
                    int tmpCurrNum = 0, j = 0;
                    const char *fmt = "{\"%s\":%d}";
                    int maxSize = sdevCache()->maxsize;
                    int currSize = sdevCache()->currsize;
                    SDevNode *arr = sdevArray();
                    for (j = 0; (j < maxSize && tmpCurrNum < currSize); j++) {     
                        if (arr[j].occupied) {                            
                            memset(json, 0, sizeof(json));
                            ret = sprintf(json, fmt, JSON_NAME_SDEV_GET_INFO, j);
                            LELOG("idx[%d], json is [%d][%s]", ret, json);
                            ret = sengineCall((const char *)ginScriptCfg->data.script, ginScriptCfg->data.size, S1_STD2PRI,
                                (uint8_t *)json, ret, (uint8_t *)cmd, sizeof(cmd));
                            if (ret <= 0) {
                                LELOGW("[SUBDEV] sengineQuerySlave sengineCall("S1_STD2PRI") [%d]", ret);
                                continue;
                            }
                            ret = ioWrite(ioHdl[x].ioType, ioHdl[x].hdl, cmd, ret);
                            if (0 >= ret) {
                                LELOGW("[SUBDEV] sengineQuerySlave ioWrite [%d]", ret);
                            }
                            tmpCurrNum++;
                        }
                    }               
                }
                LELOG("sengineQuerySlave done");
            TIMEOUT_SECS_END
        }

    FOR_EACH_IO_HDL_END;

    return 0;
}

int senginePollingSlave(void) {
    Datas datas = {0};
    char status[MAX_BUF];
    uint8_t bin[MAX_BUF] = {0};
    uint16_t currLen = 0, appendLen = 0;
    int whatKind = 0, ret = 0, size = 0, i;

    FOR_EACH_IO_HDL_START;
        ret = ioRead(ioHdl[x].ioType, ioHdl[x].hdl, bin, sizeof(bin));
        if (ret <= 0) {
            // LELOGW("senginePollingSlave ioRead [%d]", ret);
            continue;
        }
        size = ret;

        ret = sengineCall((const char *)ginScriptCfg->data.script, ginScriptCfg->data.size, S1_OPT_DO_SPLIT,
                bin, size, (uint8_t *)&datas, sizeof(Datas));
        // LELOG("senginePollingSlave "S1_OPT_DO_SPLIT" ret[%d], datasCountsLen[%d], datasLen[%d]", ret, datas.datasCountsLen/2, datas.datasLen);
        if (0 >= ret) {
            // LELOGW("senginePollingSlave sengineCall "S1_OPT_DO_SPLIT" [%d]", ret);
            datas.datasCountsLen = 1;
            datas.arrDatasCounts[0] = size;
        }
        for (i = 0; i < datas.datasCountsLen; i += sizeof(uint16_t)) {
            memcpy(&currLen, &datas.arrDatasCounts[i/sizeof(uint16_t)], sizeof(uint16_t));
            // LELOG("[SENGINE]_s1OptDoSplit_[%d]_cmd: curr piece len[%d]", i/sizeof(uint16_t), currLen);
            memcpy(&datas.arrDatas[appendLen], &bin[appendLen], currLen);

            if (0)
            {
                int j = 0;
                extern int bytes2hexStr(const uint8_t *src, int srcLen, uint8_t *dst, int dstLen);
                uint8_t hexStr[96] = {0};
                LELOG("[SENGINE]datas.arrDatas currLen[%d], appendLen[%d]", currLen, appendLen);
                bytes2hexStr(&datas.arrDatas[j + appendLen], currLen, hexStr, sizeof(hexStr));
                LELOG("bin[%s]", hexStr);          
            }
            

            ret = sengineCall((const char *)ginScriptCfg->data.script, ginScriptCfg->data.size, S1_GET_VALIDKIND,
                    &datas.arrDatas[appendLen], currLen, (uint8_t *)&whatKind, sizeof(whatKind));
            // LELOG("sengineCall ret size [%d], currLen[%d] whatKind [%d]", ret, currLen, whatKind);
            if (0 >= ret) {
                LELOGW("senginePollingSlave sengineCall "S1_GET_VALIDKIND" [%d]", ret);
                continue;
            }
            switch (whatKind) {
                case WHATKIND_MAIN_DEV_RESET: {
                        extern int resetConfigData(void);
                        ret = resetConfigData();
                        LELOG("resetConfigData [%d]", ret);
                        if (0 <= ret) {
                            halReboot();
                        }
                    }
                    break;
                case WHATKIND_MAIN_DEV_DATA: {
                        extern int lelinkNwPostCmdExt(const void *node);
                        int len = 0;
                        len = sengineCall((const char *)ginScriptCfg->data.script, ginScriptCfg->data.size, S1_PRI2STD,
                                &datas.arrDatas[appendLen], currLen, (uint8_t *)status, sizeof(status));
                        // LELOGE("sengineCall len = %d. [%s]", len, status);
                        if (len <= 0) {
                            LELOGW("senginePollingSlave sengineCall("S1_PRI2STD") [%d]", len);
                        } else if (cacheIsChanged(status, len)) {
                            postStatusChanged(0);
                            cacheSetTerminalStatus(status, len);
                            LELOG("Cache status:%s", status);
                        }
                    }
                    break;
                // for sub devs
                case WHATKIND_SUB_DEV_RESET: {
                        LELOG("WHATKIND_SUB_DEV_RESET");
                    }
                    break;
                case WHATKIND_SUB_DEV_DATA:
                case WHATKIND_SUB_DEV_JOIN: {
                        int len;
                        SDevNode *tmpArr = sdevArray();
                        // NodeData node = {0};
                        if (NULL == tmpArr) {
                            LELOGE("sdevArray is NULL");
                            break;
                        }
                        len = sengineCall((const char *)ginScriptCfg->data.script, ginScriptCfg->data.size, S1_PRI2STD,
                            &datas.arrDatas[appendLen], currLen, (uint8_t *)status, sizeof(status));
                        if (0 >= len) {
                            LELOGW("senginePollingSlave sengineCall("S1_PRI2STD") [%d]", len);
                            break;
                        }

                        if (WHATKIND_SUB_DEV_JOIN == whatKind) {
                            LELOG("WHATKIND_SUB_DEV_JOIN");
                            if (0 > sdevInsert(tmpArr, status, len)) {
                                LELOGE("sdevInsert is FAILED");
                                break;
                            }
                        } else if (WHATKIND_SUB_DEV_DATA == whatKind) {
                            LELOG("WHATKIND_SUB_DEV_DATA");
                            if (0 > sdevUpdate(tmpArr, status, len)) {
                                LELOGE("sdevUpdate is FAILED");
                                break;
                            }
                        }
                        // LELOG("outcomming [%s]", status);
                    }
                    break;
                case WHATKIND_SUB_DEV_LEAVE: {
                        LELOG("WHATKIND_SUB_DEV_LEAVE");
                    }
                    break;

                default:
                    //LELOGW("Unknow whatKind = %d", whatKind);
                    break;
            }

            appendLen += currLen;
            //LEPRINTF("\r\n");
        }
        {
            
        }
    FOR_EACH_IO_HDL_END;
    
    return 0;
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
        if (isCloudAuthed()) {
            NodeData node = {0};
            node.cmdId = LELINK_CMD_CLOUD_HEARTBEAT_REQ;
            node.subCmdId = LELINK_SUBCMD_CLOUD_IA_EXE_NOTIFY_REQ;
            node.reserved = cacheInt - ginIACache.cache;
            lelinkNwPostCmdExt(&node);
        }
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

    // LELOG("senginePollingRules -s ");
    ret = lelinkStorageReadPrivateCfg(&privCfg);
    if (0 > ret || privCfg.csum != crc8((const uint8_t *)&(privCfg.data), sizeof(privCfg.data))) {
        LELOGW("senginePollingRules lelinkStorageReadPrivateCfg csum FAILED");
        return -1;
    }
    // LELOG("sengineFindRule [%d][%d]", privCfg.data.iaCfg.num);

    if (0 >= privCfg.data.iaCfg.num || MAX_IA < privCfg.data.iaCfg.num) {
        // LELOGW("senginePollingRules rules num[%d]", privCfg.data.iaCfg.num);
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
    ginIACache.cfg.num = 0;
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
            ginIACache.cfg.num++;
        }
    }
    LELOG("senginePollingRules -e ");

    return 0;
}

int findPosForIAName(PrivateCfg *privCfg, const char *strSelfRuleName, int lenSelfRuleName, int *whereToPut) {
    int i = 0, found = 0;
    *whereToPut = -1;
    for (i = MAX_IA - 1; i > -1; i--) {
        if (0 < privCfg->data.iaCfg.arrIA[i]) {
            if (0 == memcmp(strSelfRuleName, privCfg->data.iaCfg.arrIAName[i], lenSelfRuleName)) {
                *whereToPut = i;
                found = 1;
                break;
            }
        } else {
            *whereToPut = i;
            found = 0;
        }
    }

    return found;
}

int sengineRemoveRules(const char *name) {
    int ret, whereToPut = -1, found = 0;
    PrivateCfg privCfg;
    LELOG("sengineRemoveRules -s ");
    if (NULL == name) {
        LELOGE("sengineRemoveRules name NULL");
        return -1;
    }
    ret = lelinkStorageReadPrivateCfg(&privCfg);
    if (0 > ret || privCfg.csum != crc8((const uint8_t *)&(privCfg.data), sizeof(privCfg.data))) {
        LELOGE("sengineRemoveRules lelinkStorageWriteScriptCfg2 csum FAILED");
        return -2;
    }
    found = findPosForIAName(&privCfg, name, strlen(name), &whereToPut);
    if (found) {
        privCfg.data.iaCfg.arrIA[whereToPut] = -1;
        privCfg.data.iaCfg.num--;
        lelinkStorageWritePrivateCfg(&privCfg);
        return found;
    }    

    LELOG("sengineRemoveRules found[%d] whereToPut[%d] -e ", found, whereToPut);
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


