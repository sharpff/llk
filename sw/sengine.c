#include "leconfig.h"
#include "sengine.h"

#include "io.h"
#include "data.h"
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "ota.h"
#include "state.h"
#include "protocol.h"
#include "cache.h"
#include "misc.h"
#include "jsonv2.h"
#include "jsgen.h"
#include "utility.h"

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
    SDEV_BUF_TYPE_INFO,
    SDEV_BUF_TYPE_HW,
    SDEV_BUF_TYPE_MARK_DEL
}SDEV_BUF_TYPE;

IA_CACHE ginIACache;
ScriptCfg *ginScriptCfg;
ScriptCfg2 *ginScriptCfg2;
static uint32_t ginDelayMS;
static int ginCurrCvtType;
static void loadSDevInfo(SDevNode *arr);

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
            loadSDevInfo(ginArrSDev);
        } else {
            LELOGE("Heap is not enough!!!");
        }
    }
    LELOG("[SENGINE] sdevArray usage[%d/%d], node[SDevNBase %d | SDevNode %d], total[flash %d | heap %d]", 
        sdevCache()->currsize, sdevCache()->maxsize, sizeof(SDevNBase), sizeof(SDevNode), sizeof(SDevNBase)*MAX_SDEV_NUM, sdevCache()->singleSize*MAX_SDEV_NUM);
    return ginArrSDev;
}

static void loadSDevInfo(SDevNode *arr) {
    int i = 0, ret = 0;
    if (0 > (ret = lelinkStorageReadSDevInfoCfg(arr))) {
        LELOGW("loadSDevInfo FAILED ret[%d]", ret);
        return;
    }
    for (i = 0; i < MAX_SDEV_NUM; i++) {
        if (arr[i].flag) {            
            LELOG("loadSDevInfo load --------- [%d]", i);
            LELOG("=======> SDevInfoCfg mac[%s] ", arr[i].mac);
            LELOG("=======> SDevInfoCfg ud[%s] ", arr[i].ud);
            LELOG("=======> SDevInfoCfg sdevInfo[%s] ", arr[i].sdevInfo);
            LELOG("=======> SDevInfoCfg sdevEpt[%02x%02x%02x%02x%02x%02x%02x%02x] ", 
                arr[i].sdevEpt[0], arr[i].sdevEpt[1], arr[i].sdevEpt[2], arr[i].sdevEpt[3],
                arr[i].sdevEpt[4], arr[i].sdevEpt[5], arr[i].sdevEpt[6], arr[i].sdevEpt[7]);
            LELOG("=======> SDevInfoCfg sdevMan[%s] ", arr[i].sdevMan);
            LELOG("=======> SDevInfoCfg isSDevInfoDone[0x%02x] ", arr[i].isSDevInfoDone);

            // to check if it is a cleaned flash space.
            if (0x0f < arr[i].isSDevInfoDone) {
                arr[i].isSDevInfoDone = 0;
            }
            sdevCache()->currsize++;
        }
    }
}

static int sdevInfoSerilized(const SDevNode *arr) {
    int ret = 0, i = 0;
    // SDevInfoCfg *sdevInfoCfg = (SDevInfoCfg *)arr;
    for (i = 0; i < MAX_SDEV_NUM; i++) {

    }
    ret = lelinkStorageWriteSDevInfoCfg(arr);
    {
        // SDevInfoCfg tmp = {0};
        // ret = lelinkStorageReadSDevInfoCfg(&tmp, index);
        // uint8_t csum = crc8(&(tmp.data), sizeof(tmp.data));
        // if (!ret && tmp.csum != csum) {
        //     LELOGE("SUM FAILED [0x%02x] cmp [0x%02x]", tmp.csum, csum);
        // }
        // LELOG("sdevInfoSerilized =======> SDevInfoCfg csum[%02x], sdevInfo[%s] ", index, arr[index].sdevInfo);
        // LELOG("sdevInfoSerilized =======> SDevInfoCfg mac[%s] ", arr[index].mac);
        // LELOG("sdevInfoSerilized =======> SDevInfoCfg ud[%s] ", arr[index].ud);
        // LELOG("sdevInfoSerilized =======> SDevInfoCfg sdevEpt[%02x%02x%02x%02x%02x%02x%02x%02x] ", 
        //     arr[index].sdevEpt[0], arr[index].sdevEpt[1], arr[index].sdevEpt[2], arr[index].sdevEpt[3],
        //     arr[index].sdevEpt[4], arr[index].sdevEpt[5], arr[index].sdevEpt[6], arr[index].sdevEpt[7]);
        // LELOG("sdevInfoSerilized =======> SDevInfoCfg sdevMan[%s] ", arr[index].sdevMan);
    }

    return ret;
}

static void sdevArraySet(int index, const SDevNode *node, SDEV_BUF_TYPE bufType) {
    if (index >= sdevCache()->maxsize || 
        0 > index) {
        return;
    }

    if (!sdevArray()[index].flag) {
        return;
    }

    switch (bufType) {
        case SDEV_BUF_TYPE_HW: {
            memcpy(&sdevArray()[index].ud, node->ud, sizeof(node->ud));
            memcpy(&sdevArray()[index].mac, node->mac, sizeof(node->mac));
            }break;
        case SDEV_BUF_TYPE_INFO: {
                memcpy(&(sdevArray()[index]).sdevInfo, node->sdevInfo, sizeof(node->sdevInfo));
            }break;
        case SDEV_BUF_TYPE_STATUS: {
                memcpy(&(sdevArray()[index]).sdevStatus, node->sdevStatus, sizeof(node->sdevStatus));
            }break;
        case SDEV_BUF_TYPE_MARK_DEL: {
                (sdevArray()[index]).isSDevInfoDone = 0x10;
            }break;
        default: 
            // LELOGE("NO MATCHED BUF TYPE FOR SDEV");
            break;
    }
}

// static int sdevArrayGet(int index, SDevNode *node) {
//     if (0 > index && !node) {
//         return -1;
//     }

//     if (!sdevArray()[index].flag) {
//         return -2;
//     }

//     if (0x08 != (0x08 & sdevArray()[index].isSDevInfoDone)) {
//         return -3;
//     }

//     memcpy(node, &(sdevArray()[index]), sizeof(SDevNode));
//     return index;
// }

extern void postSDevRecordChanged(int index, int kind);
int sdevArrayDel(int index) {
    int ret = 0;
    uint8_t restore = sdevArray()[index].isSDevInfoDone;
    sdevArraySet(index, NULL, SDEV_BUF_TYPE_MARK_DEL);
    ret = sdevInfoSerilized(sdevArray());
    if (0 > ret) {
        sdevArray()[index].isSDevInfoDone = restore;
        return ret;
    }
    postSDevRecordChanged(index, 2);
    return ret;
}

static int sdevArrayDelCB(NodeData *currNode) {
    return 1;
}

int sdevArrayReset() {
    int ret = 0;
    qCheckForClean(sdevCache(), (int(*)(void*))sdevArrayDelCB);
    ret = sdevInfoSerilized(sdevArray());
    if (0 <= ret) {
        postSDevRecordChanged(-1, 4);
    }
    return ret;
}

static int forEachNodeSDevForInValid(SDevNode *currNode, void *uData) {
    if (0x08 != (0x08 & currNode->isSDevInfoDone)) {
        return 1;
    }
    return 0;
}

static int forEachNodeSDevForValid(SDevNode *currNode, void *uData) {
    LELOG("[SENGINE] forEachNodeSDevByIdxCB currNode->ud[%s] uData[%s] isSDevInfoDone[0x%02x]", currNode->ud, uData, currNode->isSDevInfoDone);
    if (0 == strcmp(currNode->ud, (char *)uData) && 
        0x08 == (0x08 & currNode->isSDevInfoDone)) {
        return 1;
    }
    return 0;
}

int forEachNodeSDevByMacCB(SDevNode *currNode, void *uData) {
    LELOG("[SENGINE] forEachNodeSDevByMacCB currNode->mac[%s] uData[%s]", currNode->mac, uData);
    if (0 == strcmp(currNode->mac, (char *)uData)) {
        return 1;
    }
    return 0;
}

static int forEachNodeSDevByUDCB(SDevNode *currNode, void *uData) {
    LELOG("[SENGINE] forEachNodeSDevByUDCB [0x%p]", uData);
    if (0 == strcmp(currNode->ud, (char *)uData)) {
        return 1;
    }
    return 0;
}


static void postStatusChanged(int plusIdx) {
    int ret = 0;
    NodeData node = {0};
    node.reserved = plusIdx;
    // TIMEOUT_SECS_BEGIN(1)
    if (isCloudOnlined() && getLock()) {
        node.cmdId = LELINK_CMD_CLOUD_HEARTBEAT_REQ;
        node.subCmdId = LELINK_SUBCMD_CLOUD_STATUS_CHANGED_REQ;
    } else {
        char br[MAX_IPLEN] = {0};
        node.cmdId = LELINK_CMD_DISCOVER_REQ;
        node.subCmdId = LELINK_SUBCMD_DISCOVER_STATUS_CHANGED_REQ;    
        node.rspVal = 1;
        node.noAck = 1;
        ret = halGetBroadCastAddr(br, sizeof(br));
        if (0 >= ret) {
            strcpy(br, "255.255.255.255");
        } else
        strcpy(node.ndIP, br);
        node.ndPort = NW_SELF_PORT;
        lelinkNwPostCmdExt(&node);
    }
    lelinkNwPostCmdExt(&node);
    // TIMEOUT_SECS_END
}


static int sdevInfoRsp(SDevNode *arr, const char *status, int len) {
    int i = 0, j = 0, ret = 0, val = 0, num = 0, index = -1, ept = -1;
    char str[64] = {0};
    int8_t isClusterComplete = 1;
    char totalJson[SDEV_MAX_INFO] = {0};
    // const char *fmt = "{\"%s\":%d}";
    jobj_t jobj;
    jsontok_t jsonToken[NUM_TOKENS];
    json_string_t jstr;

    if (NULL == arr || NULL == status || 0 >= len) {
        return -1;
    }

    ret = json_init(&jobj, jsonToken, NUM_TOKENS, (char *)status, len);
    if (WM_SUCCESS != ret) {
        return -2;
    }

    LELOG("sdevInfoRsp input status[%s]", status);

    // get ud first
    if (WM_SUCCESS != json_get_val_str(&jobj, JSON_NAME_SDEV_USER_DATA, str, sizeof(str))) {
        LELOGE("sdevInfoRsp json_get_val_str [%s] FAILED", JSON_NAME_SDEV_USER_DATA);
        return -3;
    }

    // find the node with ud
    index = qForEachfromCache(sdevCache(), (int(*)(void*, void*))forEachNodeSDevByUDCB, str);
    if (0 > index) {
        LELOGE("sdevInfoRsp ud NOT FOUND [%d]", index);
        return -4;              
    }
    LELOG("forEachNodeSDevByUDCB index[%d] str[%s], ud[%s] mac[%s]", index, str, arr[index].ud, arr[index].mac);

    if (WM_SUCCESS == json_get_val_int(&jobj, JSON_NAME_SDEV_QUERY_EPT, &val) && 2 == val) {
        // 0x01. endpoint list(active response)
        if(WM_SUCCESS != (ret = json_get_array_object(&jobj, JSON_NAME_SDEV_EPT, &num))) {
            LELOGE("sdevInfoRsp [%s] NOT FOUND [%d]", JSON_NAME_SDEV_EPT, ret);
            return -5;
        }
        LELOG("EPT LIST NUM ====> [%d]", num);
        num = num < 0 ? 0 : num;
        for(i = 0; i < num && i < SDEV_MAX_EPT; i++) {
            if(WM_SUCCESS != json_array_get_int(&jobj, i, &val)) {
                continue;
            }
            arr[index].sdevEpt[j++] = (val + 1) | 0x80;
        }
        if (0 < j) {
            arr[index].isSDevInfoDone |= 0x01;
        }
    } else if (WM_SUCCESS == json_get_val_int(&jobj, JSON_NAME_SDEV_QUERY_INFO, &val) && 2 == val) {
        // 0x02. cluster done(simple descriptor response)
        char oldJson[SDEV_MAX_INFO/2] = {0}, tmpJson[SDEV_MAX_INFO/2] = {0};

        if (WM_SUCCESS != json_get_val_int(&jobj, JSON_NAME_SDEV_EPT, &ept)) {
            LELOGE("sdevInfoRsp [%s] NOT FOUND", JSON_NAME_SDEV_EPT);
            return -6;
        }
        LELOG("WILL BE MERGED JSON ====> [%s]", arr[index].sdevInfo);

        if (0 >= getJsonObject(status, len, JSON_NAME_SDEV_SDEVDES, &tmpJson[1], sizeof(tmpJson)-1)) {
            LELOGE("JSON_NAME_SDEV_SDEVDES [%s] Failed", status, JSON_NAME_SDEV_SDEVDES);
            return -7;
        }

        // current ept info
        // json_str_init(&jstr, tmpJson, sizeof(tmpJson));
        // json_start_object(&jstr);
        // json_set_val_int(&jstr, JSON_NAME_SDEV_EPT, ept);
        // if (WM_SUCCESS != json_get_val_str(&jobj, JSON_NAME_SDEV_PID, str, sizeof(str))) {
        //     LELOGE("sdevInfoRsp [%s] NOT FOUND", JSON_NAME_SDEV_PID);
        //     json_close_object(&jstr);
        //     return -7;
        // }
        // json_set_val_str(&jstr, JSON_NAME_SDEV_PID, str);
        // if (WM_SUCCESS != json_get_val_str(&jobj, JSON_NAME_SDEV_DID, str, sizeof(str))) {
        //     LELOGE("sdevInfoRsp [%s] NOT FOUND", JSON_NAME_SDEV_DID);
        //     json_close_object(&jstr);
        //     return -8;
        // }
        // json_set_val_str(&jstr, JSON_NAME_SDEV_DID, str);

        // if(WM_SUCCESS == (ret = json_get_array_object(&jobj, JSON_NAME_SDEV_CLU_IN, &num))) {
        //     LELOG("inCluster num is [%d]", num);
        //     if (0 < num) {
        //         json_push_array_object(&jstr, JSON_NAME_SDEV_CLU_IN);
        //         for(i = 0; i < num; i++) {
        //             if(WM_SUCCESS != json_array_get_str(&jobj, i, str, sizeof(str))) {
        //                 continue;
        //             }
        //             json_set_array_str(&jstr, str);
        //         }
        //         json_pop_array_object(&jstr);
        //     }
        //     json_release_array_object(&jobj);
        // }

        // if(WM_SUCCESS == (ret = json_get_array_object(&jobj, JSON_NAME_SDEV_CLU_OUT, &num))) {
        //     LELOG("outCluster num is [%d]", num);
        //     if (0 < num) {
        //         json_push_array_object(&jstr, JSON_NAME_SDEV_CLU_OUT);
        //         for(i = 0; i < num; i++) {
        //             if(WM_SUCCESS != json_array_get_str(&jobj, i, str, sizeof(str))) {
        //                 continue;
        //             }
        //             json_set_array_str(&jstr, str);
        //         }
        //         json_pop_array_object(&jstr);
        //     }
        //     json_release_array_object(&jobj);
        // }

        // json_close_object(&jstr);

        LELOG("THE NEW JSON ====> [%s] EPT[%d]", &tmpJson[1], ept);

        // make the sdevEpt valid
        for(i = 0; arr[index].sdevEpt[i] && i < SDEV_MAX_EPT; i++) {
            // to void repeat ept entry
            LELOG("arr[%d].sdevEpt[%d] is [%x]", index, i, arr[index].sdevEpt[i]);
            if (((arr[index].sdevEpt[i] & 0x7F) - 1) == ept && (arr[index].sdevEpt[i] & 0x80)) {
                arr[index].sdevEpt[i] &= 0x7F;

                LELOG("valid eptNum[%d] eptVal[%d]", ept, arr[index].sdevEpt[i]);
                // APPEND old ept info
                json_str_init(&jstr, totalJson, sizeof(totalJson));
                json_start_object(&jstr);
                // json_push_object(&jstr, JSON_NAME_SDEV);
                json_push_array_object(&jstr, JSON_NAME_SDEV_DES);
                // append
                if (0 < getJsonArray(arr[index].sdevInfo, sizeof(arr[index].sdevInfo), JSON_NAME_SDEV_DES, oldJson, sizeof(oldJson))) {
                    // for old
                    json_set_val_strobj(&jstr, NULL, oldJson + 1, strlen(oldJson) - 2);
                    // for new
                    if (!strstr(oldJson, &tmpJson[1])) {
                        tmpJson[0] = ',';
                        json_set_val_strobj(&jstr, NULL, tmpJson, strlen(tmpJson));
                    }
                    LELOG("APPEND OLD JSON ====> [%s]", oldJson);
                } else
                    json_set_val_strobj(&jstr, NULL, &tmpJson[1], (strlen(tmpJson)-1));

                json_pop_array_object(&jstr);
                // json_pop_object(&jstr);
                json_close_object(&jstr);

                strcpy(arr[index].sdevInfo, totalJson);
                arr[index].sdevInfo[strlen(totalJson)] = '\0';
                arr[index].isSDevInfoDone |= 0x02;
                LELOG("THE RESULT =======> sdevInfoRsp sdevInfo[%s] ", arr[index].sdevInfo);
                break;
            }
        }
    } else if (WM_SUCCESS == json_get_val_int(&jobj, JSON_NAME_SDEV_QUERY_MAN, &val) && 2 == val) {
        // 0x04. man done(node descriptor response)
        if (WM_SUCCESS != json_get_val_str(&jobj, JSON_NAME_SDEV_MAN, str, sizeof(str))) {
            LELOGE("sdevInfoRsp [%s] NOT FOUND", JSON_NAME_SDEV_MAN);
            return -9;
        }

        strcpy(arr[index].sdevMan, str);
        arr[index].sdevMan[strlen(str)] = 0;
        arr[index].isSDevInfoDone |= 0x04;
        LELOG("DONE =======> sdevInfoRsp index[%d], sdevInfo[%s] ", index, arr[index].sdevInfo);
        LELOG("DONE =======> sdevInfoRsp mac[%s] ", arr[index].mac);
        LELOG("DONE =======> sdevInfoRsp ud[%s] ", arr[index].ud);
        LELOG("DONE =======> sdevInfoRsp sdevEpt[%02x%02x%02x%02x%02x%02x%02x%02x] ", 
            arr[index].sdevEpt[0], arr[index].sdevEpt[1], arr[index].sdevEpt[2], arr[index].sdevEpt[3],
            arr[index].sdevEpt[4], arr[index].sdevEpt[5], arr[index].sdevEpt[6], arr[index].sdevEpt[7]);
        LELOG("DONE =======> sdevInfoRsp sdevMan[%s] ", arr[index].sdevMan);
    }

    // serilized to disk
    for(i = 0; arr[index].sdevEpt[i] && i < SDEV_MAX_EPT; i++) {
        if ((arr[index].sdevEpt[i] & 0x80)) {
            isClusterComplete = 0;
            break;
        }
    }
    if (isClusterComplete && (0x07 == (0x07 & arr[index].isSDevInfoDone))) {
        if (arr[index].sdevMan[0]) {
            arr[index].isSDevInfoDone |= 0x08;
            
            // append 'manufacturer'
            // memset(totalJson, 0, sizeof(totalJson));
            // json_str_init(&jstr, totalJson, sizeof(totalJson));
            // json_start_object(&jstr);
            // json_set_val_strobj(&jstr, NULL, arr[index].sdevInfo, strlen(arr[index].sdevInfo));
            // json_set_val_str(&jstr, JSON_NAME_SDEV_MAN, str);
            // strcpy(arr[index].sdevInfo, totalJson); arr[index].sdevInfo[strlen(arr[index].sdevInfo)] = 0;
            // json_close_object(&jstr);

            // sprintf(&(arr[index].sdevInfo[strlen(arr[index].sdevInfo) - 1]), ",\"%s\":\"%s\"}", JSON_NAME_SDEV_MAN, arr[index].sdevMan);

            ret = sdevInfoSerilized(arr);
            LELOG("COMPLETED =======> sdevInfoRsp[%d] arr[%d].sdevEpt[%d] is [%x] mac[%s], ud[%s], sdevMan[%s]", 
                index, index, i, arr[index].sdevEpt[i], arr[index].mac, arr[index].ud, arr[index].sdevMan);
            if (0 <= ret) {
                postSDevRecordChanged(index, 1);
            }
        }
    }

    return 0;
}

static int sdevInsert(SDevNode *arr, const char *status, int len) {
    // TODO: insert the ud(short mac) & mac
    int valJoin = 0, index = 0, ret = 0;
    jobj_t jobj;
    jsontok_t jsonToken[NUM_TOKENS];
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

        // ret = json_get_composite_object(&jobj, JSON_NAME_SDEV);
        // if (0 == ret) {

            if (WM_SUCCESS != json_get_val_str(&jobj, JSON_NAME_SDEV_MAC, node.mac, sizeof(node.mac))) {
                LELOGE("sdevInsert json_get_val_str [%s] FAILED", JSON_NAME_SDEV_MAC);
                return -3;
            }

            if (WM_SUCCESS != json_get_val_str(&jobj, JSON_NAME_SDEV_USER_DATA, (char *)node.ud, sizeof(node.ud))) {
                LELOGE("sdevInsert json_get_val_str [%s] FAILED", JSON_NAME_SDEV_USER_DATA);
                return -4;
            }

            index = qForEachfromCache(sdevCache(), (int(*)(void*, void*))forEachNodeSDevByMacCB, node.mac);
            if (0 <= index) {
                LELOG("sdevInsert qForEachfromCache already EXIST [%d]", index);
                memcpy(arr[index].ud, node.ud, sizeof(arr[index].ud));
                return 0;                
            }
            // node.occupied = 1;
            // {
            //     json_string_t jstr;

            //     json_str_init(&jstr, node.sdevInfo, sizeof(node.sdevInfo));
            //     json_start_object(&jstr);
            //     json_push_object(&jstr, JSON_NAME_SDEV);

            //     json_push_array_object(&jstr, JSON_NAME_SDEV_DES);
            //     json_pop_array_object(&jstr);

            //     json_set_val_str(&jstr, JSON_NAME_SDEV_MAC, node.mac);

            //     json_pop_object(&jstr);
            //     json_close_object(&jstr);
            // }

            // insert into a new space 
            if (!qEnCache(sdevCache(), &node)) {
                // to place in an invalid(incomplete) space
                index = qForEachfromCache(sdevCache(), (int(*)(void*, void*))forEachNodeSDevForInValid, NULL);
                if (0 <= index) {
                    sdevArraySet(index, &node, SDEV_BUF_TYPE_HW);  
                    LELOG("to replace in an invalid(incomplete) space[%d]", index);
                } else {
                    LELOGE("FAILED SDEV(s) is FULL", index);
                    return -5;
                }
            }
            LELOG("sdevInsert ret[%d] index[%d] mac[%s] sdev[%s] [%p]", ret, index, node.mac, node.sdevInfo, &(sdevArray()[0]));
            // TODO: send HELLO for sdev
        // }


        LELOG("sdevInsert join END ****************************");
    }
    return 0;
}

static int sdevUpdate(SDevNode *arr, const char *status, int len) {
    // TODO: 
    // 1. update the info
    // 2. update the status
    
    // {"subDevGetList":[0,1,2]}
    jobj_t jobj;
    jsontok_t jsonToken[NUM_TOKENS];
    // char name[MAX_RULE_NAME] = {0};
    int ret = 0;
    int sDevIdx = 0;
    char buf[MAX_BUF] = {0};
    SDevNode node;

    memset(&node, 0, sizeof(SDevNode));
    ret = json_init(&jobj, jsonToken, NUM_TOKENS, (char *)status, len);
    if (WM_SUCCESS != ret) {
        return -1;
    }

    // for sub dev status ind
    if (0 < getJsonObject(status, len, JSON_NAME_SDEV_STATUS, buf, sizeof(buf))) {
        LELOG("sdevUpdate sdevStatus START ****************************");
        LELOG("%s", status);
        ret = json_get_composite_object(&jobj, JSON_NAME_SDEV);
        if (WM_SUCCESS == ret) {
            if (WM_SUCCESS != json_get_val_str(&jobj, JSON_NAME_SDEV_USER_DATA, (char *)node.ud, sizeof(node.ud))) {
                LELOGE("sdevUpdate json_get_val_str [%s] FAILED", JSON_NAME_SDEV_USER_DATA);
                return -2;
            }
            LELOG("ud is [%s]", node.ud);
            sDevIdx = qForEachfromCache(sdevCache(), (int(*)(void*, void*))forEachNodeSDevForValid, node.ud);
            if (0 <= sDevIdx) {
                // if (0 <= sdevArrayGet(sDevIdx, &node)) {
                    memcpy(&node, &(sdevArray()[sDevIdx]), sizeof(SDevNode));
                    LELOG("old[%s] new[%s]", node.sdevStatus, buf);
                    ret = memcmp(node.sdevStatus, buf, strlen(buf));
                    memset(node.sdevStatus, 0, sizeof(node.sdevStatus));
                    strcpy(node.sdevStatus, buf);
                    sdevArraySet(sDevIdx, &node, SDEV_BUF_TYPE_STATUS);  
                    if (0 != ret) {
                        postStatusChanged(sDevIdx + 1);
                        if (0 < getSDevStatus(sDevIdx, buf, sizeof(buf))) {
                            senginePollingRules(buf, sizeof(buf));
                        }
                    }
                    LELOG("=> sDevIdx[%d] ud[%s] mac[%s] sdev[%s] sdevStatus[%s]", sDevIdx, node.ud, node.mac, node.sdevInfo, node.sdevStatus);
                // }
            }
        }
        LELOG("sdevUpdate sdevStatus END ****************************");
    }

    if (!(sdevCache()->sDevVer) && WM_SUCCESS != json_get_val_int(&jobj, JSON_NAME_SDEV_VER, (int *)&(sdevCache()->sDevVer))) {
        sdevCache()->sDevVer = 0;
    }


    return 0;
}

static int sdevRemove(SDevNode *arr, const char *status, int len) {
    int ret, index, val = 0;
    char str[64] = {0};
    jobj_t jobj;
    jsontok_t jsonToken[NUM_TOKENS];
    LELOG("sdevRemove START ****************************");
    ret = json_init(&jobj, jsonToken, NUM_TOKENS, (char *)status, len);
    if (WM_SUCCESS != ret) {
        return -1;
    }

    // if (0 > getJsonObject(status, len, JSON_NAME_SDEV, str, sizeof(str))) {
    //     LELOGE("sdevRemove getJsonObject [%s] FAILED", JSON_NAME_SDEV);
    //     return -3;
    // }

    // memset(str, 0, sizeof(str));
    // ret = json_get_composite_object(&jobj, JSON_NAME_SDEV);
    // if (0 == ret) {
        if (WM_SUCCESS != json_get_val_str(&jobj, JSON_NAME_SDEV_MAC, str, sizeof(str))) {
            LELOGE("sdevRemove json_get_val_str [%s] FAILED", JSON_NAME_SDEV_MAC);
            return -4;
        }
    // }

        if (WM_SUCCESS != json_get_val_int(&jobj, JSON_NAME_SDEV_DEL, &val)) {
            LELOGE("sdevRemove json_get_val_int [%s] FAILED", JSON_NAME_SDEV_MAC);
            return -5;
        }
    

    // find the node with ud
    index = qForEachfromCache(sdevCache(), (int(*)(void*, void*))forEachNodeSDevByMacCB, str);
    if (0 > index) {
        LELOGE("sdevRemove ud NOT FOUND [%d]", index);
        return -6;              
    }

    ret = sdevArrayDel(index);
    LELOG("sdevRemove sdevArrayDel[%d] END ****************************", ret);

    return ret >= 0 ? 0 : -7;
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
    
    //return sizeof(Queries);
    return queries->queriesLen;
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
        // LELOG("[SENGINE] s1GetVer: [%d][%s]", size, output);
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
    int sLen = lua_tointeger(L, -2);
    int size = MIN(sLen, outputLen);
    const uint8_t *tmp = (const uint8_t *)lua_tostring(L, -1);
    if (tmp && 0 < size) {
        // int i;
        memcpy(output, tmp, size);
        // LEPRINTF("[SENGINE]_s1CvtStd2Pri_[%d]_cmd: ", size);
        // for (i = 0; i < size; i++) {
        //     LEPRINTF("%02x ", output[i]);
        // }
        // LEPRINTF("\r\n");
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
    int size = sLen;
    if(size > PRI2STD_LEN_INTERNAL)
        size = size - PRI2STD_LEN_INTERNAL;
    size = MIN(size, outputLen);
    const char *tmp = (const char *)lua_tostring(L, -1);
    if (tmp && 0 < size) {
        memcpy(output, tmp, size);
        // LELOGW("[SENGINE] s1CvtPri2Std: [%d][%s]", size, output);
    } else {
        size = 0;
    }

    return sLen;
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
        APPLOG("[SENGINE] lf_s2GetSelfName: [%d][%s]", size, output);
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
    APPLOG("[SENGINE] s2GetRuleType: repeat[%d] isAnd[%d]", *((int *)output), *(((int *)output + 1)));

    return 2*sizeof(int);
}

static IO lf_s2GetBeingReservedInfo_input(lua_State *L, const uint8_t *input, int inputLen) {
    IO io = { 0, 1 };
    return io;
}

static int lf_s2GetBeingReservedInfo(lua_State *L, uint8_t *output, int outputLen) {
    ///* cmd */
    int num = 0, index = 0, tmpLen = sizeof(int);
    char *tmp = NULL;
    // int ret = 0;
  
    index = lua_gettop(L);
    lua_pushnil(L);

    while (lua_next(L, index) != 0) {
        int n = 0;
        tmp = (char *)lua_tostring(L, -1);
        if (tmp) {
            n = strlen(tmp);
            memcpy(output + tmpLen, tmp, n);
            if (0 < 2*MAX_UUID - n) {
                memset(output + tmpLen + n, 0, 2*MAX_UUID - n);
                n = 2*MAX_UUID;
            }
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
    APPLOG("[SENGINE] s2IsConditionOK: [%d]", *((int *)output));
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
        APPLOG("[SENGINE] s2GetSelfCtrlCmd: [%d][%s]", sLen, output);
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
#ifndef STATIC_MEMORY_FOR_SCRIPT
    ginScriptCfg = (ScriptCfg *)halCalloc(1, sizeof(ScriptCfg));
    ginScriptCfg2 = (ScriptCfg2 *)halCalloc(1, sizeof(ScriptCfg2));
#else
    #if defined(PF_VAL) && (PF_VAL == 6 || PF_VAL == 9) // for MT7687
        static volatile ScriptCfg inScriptCfg __attribute__((section(".tcmBSS")));
        static volatile ScriptCfg2 inScriptCfg2 __attribute__((section(".tcmBSS")));
    #else
        static volatile ScriptCfg inScriptCfg;
        static volatile ScriptCfg2 inScriptCfg2;
    #endif
    ginScriptCfg = (ScriptCfg *)&inScriptCfg;
    ginScriptCfg2 = (ScriptCfg2 *)&inScriptCfg2;
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
int s1apiGetDevStatus(lua_State *L);
int s1apiSdevGetUserDataByMac(lua_State *L);
int s1apiSDevGetMacByUserData(lua_State *L);
int s1apiOptString2Table(lua_State *L);
int s1apiOptTable2String(lua_State *L);
int s1apiOptLogTable(lua_State *L);

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
    lua_register(L, "s1apiGetDevStatus", s1apiGetDevStatus);
    lua_register(L, "s1apiSdevGetUserDataByMac", s1apiSdevGetUserDataByMac);
    lua_register(L, "s1apiSDevGetMacByUserData", s1apiSDevGetMacByUserData);
    lua_register(L, "s1apiOptString2Table", s1apiOptString2Table);
    lua_register(L, "s1apiOptTable2String", s1apiOptTable2String);
    lua_register(L, "s1apiOptLogTable", s1apiOptLogTable);
    // lua_register(L, "csum", csum);
    
    // LELOG("[lua engine] START [%s]----------------", funcName);
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
    // LELOG("[lua engine] END -----------");
    return ret;
}

int sengineHasDevs(void) {
    int ret = 0;
    static uint8_t hasDevs = 0xFF;

    if (0xFF != hasDevs) {
        return hasDevs ? 1 : 0;
    }

    ret = sengineCall((const char *)ginScriptCfg->data.script, ginScriptCfg->data.size, S1_OPT_HAS_SUBDEVS,
            NULL, 0, (uint8_t *)&hasDevs, sizeof(hasDevs));
    if (0 >= ret) {
        hasDevs = 0;
    }

    return hasDevs ? 1 : 0;
}

int s1apiGetCurrCvtType(lua_State *L) {
    lua_pushnumber(L, ginCurrCvtType);    //key  
    return 1;
}

int s1apiGetDevStatus(lua_State *L) {
    int ret;
    char jsonState[MAX_BUF] = {0};
    memset(jsonState, 0, MAX_BUF);
    //LELOG("[SENGINE] s1apiGetDevStatus b");
    ret = sengineGetStatusVal(jsonState, MAX_BUF);
    //LELOG("[SENGINE] s1apiGetDevStatus %s", jsonState);
    lua_pushnumber(L, ret);
    lua_pushstring(L, jsonState);
    return 2;
}

int s1apiSdevGetUserDataByMac(lua_State *L) {
    char *strMac = NULL;
    int ret = 0;
    SDevNode *arr = sdevArray();

    strMac = (char *)lua_tostring(L, 1);
    if (NULL == strMac) {
        LELOGE("s1apiSdevGetUserDataByMac -e1");
        return 0;
    }

    LELOG("s1apiSdevGetUserDataByMac strMac[%s]", strMac);
    ret = qForEachfromCache(sdevCache(), (int(*)(void*, void*))forEachNodeSDevByMacCB, strMac);
    if (0 > ret) {
        LELOGE("s1apiSdevGetUserDataByMac -e2");
        return 0;
    }

    LELOG("s1apiSdevGetUserDataByMac ud[%s]", arr[ret].ud);

    lua_pushstring(L, (char *)arr[ret].ud);
    return 1;
}

int s1apiSDevGetMacByUserData(lua_State *L) {
    int ret = 0;
    SDevNode *arr = sdevArray();

    char *userData = (char *)lua_tostring(L, 1);
    if (NULL == userData) {
        LELOGE("s1apiSDevGetMacByUserData -e1");
        return 0;
    }

    LELOG("s1apiSDevGetMacByUserData userData[%s]", userData);
    ret = qForEachfromCache(sdevCache(), (int(*)(void*, void*))forEachNodeSDevByUDCB, userData);
    if (0 > ret) {
        LELOGE("s1apiSDevGetMacByUserData -e2");
        return 0;
    }

    LELOG("s1apiSDevGetMacByUserData strMac[%s]", arr[ret].mac);

    lua_pushstring(L, (char *)arr[ret].mac);

    return 1;
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
                if (0 == memcmp(ginIACache.cache[m].beingReservedUUID[n], strUUID, strlen(strUUID))) {
                    memcpy(ginIACache.cache[m].beingReservedStatus[n], strReservedStatus, MIN(lenReservedStatus, (sizeof(ginIACache.cache[m].beingReservedStatus[n]))));
                    ginIACache.cache[m].beingReservedStatus[n][MIN(lenReservedStatus, (sizeof(ginIACache.cache[m].beingReservedStatus[n]))-1)] = 0;
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
                // LELOG("[%d] [%s] <=> [%s]", n, ginIACache.cache[m].beingReservedUUID[n], ginIACache.cache[m].beingReservedStatus[n]);
                if (ginIACache.cache[m].beingReservedUUID[n][0] && ginIACache.cache[m].beingReservedStatus[n][0]) {
                    /* create table. */
                    if (!hasNewTbl) {
                        lua_newtable(L);
                        hasNewTbl = 1;
                        ret = 1;
                    }
                    lua_pushnumber(L, i + 1);    //key  
                    lua_pushstring(L, ginIACache.cache[m].beingReservedStatus[n]);  //value  
                    lua_rawset(L, -3);       //push key,value  
                	i++;
				}
            }
            break;
        }
    }

    LELOG("s2apiGetLatestStatus [%d] -e", ret);
    return ret;
}

int s1apiOptLogTable(lua_State *L) {
    int i = 0;
    int lenTable = 0;
    // uint8_t strRet[128] = {0};

    LEPRINTF("logTable: ");
    lenTable = lua_rawlen(L, 1);
    for (i = 0; i < lenTable; i++) {
        lua_pushinteger(L, i + 1);
        lua_gettable(L, -2);
        if (lua_isnumber(L, -1)) {
            
            LEPRINTF("%02x ", lua_tonumber(L, -1));
        }
        lua_pop(L, 1);
    }
    LEPRINTF("\r\n");
    return 0;
}

int s1apiOptTable2String(lua_State *L) {
    int i = 0, ret = 0;
    int lenTable = 0;
    uint8_t strRet[128] = {0};

    LELOG("s1apiTable2String -s");
    lenTable = lua_rawlen(L, 1);
    for (i = 0; i < lenTable; i++) {
        lua_pushinteger(L, i + 1);
        lua_gettable(L, -2);
        if (lua_isnumber(L, -1)) {
            ret += 1;
            strRet[i] = lua_tonumber(L, -1);
        } else
            ret = 0;
        /*else {
            ret = 0;
            lua_pushfstring(L,
                strcat(
                    strcat(
                        "invalid entry #%d in array argument #%d (expected number, got ",
                        luaL_typename(L, -1)
                        ),
                    ")"
                    ),
                i, 1
                );
            lua_error(L);
        }*/
        lua_pop(L, 1);
        if (!ret)
            break;
    }
    if (ret)
        lua_pushlstring(L, (const char *)strRet, ret);
    LELOG("s1apiTable2String -e");
    return ret > 0 ? 1 : 0;
}

int s1apiOptString2Table(lua_State *L) {
    int i = 0, len = 0;
    uint8_t *str = NULL;

    LELOG("s1apiOptString2Table -s");
    len = lua_tointeger(L, -2);
    str = (uint8_t *)lua_tostring(L, -1);
    if (0 > len || NULL == str) {
        LELOG("s1apiOptString2Table -e1");
        return 0;
    }
    LELOG("s1apiOptString2Table len[%d] str[%x]", len, str);

    // /* create table. */
    lua_newtable(L);
    for (i = 0; i < len; i++) {
        lua_pushinteger(L, i + 1);    //key  
        lua_pushinteger(L, str[i]);  //value  
        lua_rawset(L, -3);       //push key,value  
    }

    LELOG("s1apiOptString2Table -e");

    return 1;
}

// temp code, will delete when script update
int sengineIs1Version() {
    const char* p = getScriptVer();
    if(p[0] == '1' && p[1] == '.') {
        return 1;
    } else {
        return 0;
    }
}

int sengineSetAction(const char *json, int jsonLen) {
    int ret = 0, type, len, i, j, ret1;
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
        ret = sengineGetStatusVal(&jsonMerged[ret], sizeof(jsonMerged) - ret); 
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
            LELOGW("sengineSetAction sengineCall("S1_OPT_MERGE_ST2ACT") [%d] [%d][%s]", ret, jsonLen, jsonMerged);
        }

        ret = sengineCall((const char *)ginScriptCfg->data.script, ginScriptCfg->data.size, S1_STD2PRI,
            (uint8_t *)jsonMerged, jsonLen, bin, sizeof(bin));
        if (ret <= 0) {
            LELOGW("sengineSetAction sengineCall("S1_STD2PRI") [%d]", ret);
            continue;
        }

        {
            extern int bytes2hexStr(const uint8_t *src, int srcLen, uint8_t *dst, int dstLen);
            uint8_t  hexStr[240] = {0};
            bytes2hexStr(bin, ret, hexStr, sizeof(hexStr));
            LELOG("ioWrite type[0x%x] len[%d] bin[%s]", ioHdl[x].ioType, ret, hexStr);
        }

        if (sengineIs1Version()) {
            ret = ioWrite(ioHdl[x].ioType, ioHdl[x].hdl, bin, ret);
            if (ret <= 0) {
                LELOGW("sengineSetStatus ioWrite1.0 [%d]", ret);
                continue;
            }
        } else {
            for ( i=0; i<ret; ) {
                type = bin[i];
                len = bin[i+1];
                for (j = 0; j < ioGetHdlCounts(); j++) {
                    if (type == ioHdl[j].ioType) {
                        if (NULL == ioHdl[j].hdl) {
                            LELOGW("ioWrite type[0x%x] hdl is NULL", ioHdl[x].ioType);
                        } else {
                            ret1 = ioWrite(ioHdl[j].ioType, ioHdl[j].hdl, &bin[i+2], len);
                            // LELOG("ioWrite data[0x%x] len[%d]", bin[2], len);
                            if (ret1 <= 0) {
                                LELOGW("sengineSetStatus ioWrite2.0 [%d]", ret);
                            }
                        }
                        i = i+len+2;
                        if(i < ret)
                        {
                            uint8_t bin[MAX_BUF] = {0};
                            halDelayms(100);
                            ioRead(ioHdl[j].ioType, ioHdl[j].hdl, bin, sizeof(bin));
                        }
                        break;
                     }
                }
                // LELOGW("ioWrite next i = [%d] [%d]", i, ret);
            }
        }
    FOR_EACH_IO_HDL_END;

    return ret;
}

int sengineGetStatusVal(char *status, int len) {
    int ret;
    ret = cacheGetTerminalStatus(status, len);
    if(0 >= ret) {
        // LELOGW("Can't get cache status");
        strcpy(status, "{}");
        ret = 2;
    }
    return ret;
}

void sengineSetStatusVal(const char *status, int len) {
    cacheSetTerminalStatus(status, len);
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
    Queries queries = {0};
    int ret = 0, i = 0;
    uint16_t currLen = 0, appendLen = 0;

    FOR_EACH_IO_HDL_START;
        ret = sengineCall((const char *)ginScriptCfg->data.script, ginScriptCfg->data.size, S1_GET_QUERIES,
                (uint8_t *)&type, sizeof(type), (uint8_t *)&queries, sizeof(queries));

        if (ret <= 0) {
            //LELOGW("sengineQuerySlave sengineCall("S1_GET_QUERIES") [%d]", ret);
            continue;
        }

        for (i = 0; i < queries.queriesCountsLen; i += 2, appendLen += currLen) {
            memcpy(&currLen, &queries.arrQueriesCounts[i], 2);
            ret = ioWrite(ioHdl[x].ioType, ioHdl[x].hdl, &(queries.arrQueries[appendLen]), currLen);
            if (ret <= 0) {
                // LELOGW("sengineQuerySlave ioWrite [%d]", ret);
                break;
            }
        }

    FOR_EACH_IO_HDL_END;

    return 0;
}

static void sengineIODataHandler(const char *status, int len, int isSDEV) {
    if ((PRI2STD_LEN_INTERNAL == (PRI2STD_LEN_INTERNAL & len)) || 
        (PRI2STD_LEN_BOTH == (PRI2STD_LEN_BOTH & len))) {
        LELOGW("senginePollingSlave native status("S1_PRI2STD") [%d] [%s]", (len & PRI2STD_LEN_MAX), status);
        sengineSetAction((char *)status, (len & PRI2STD_LEN_MAX));
    }
    if (PRI2STD_LEN_MAX > len || 
        (PRI2STD_LEN_BOTH == (PRI2STD_LEN_BOTH & len))) {
        if (isSDEV) {
            if (0 > sdevUpdate(sdevArray(), status, len)) {
                LELOGE("sdevUpdate is FAILED");
            }
        } else {
            if (cacheIsChanged(status, (len & PRI2STD_LEN_MAX))) {
                LELOGW("senginePollingSlave cloud status("S1_PRI2STD") [%d] [%s]", (len & PRI2STD_LEN_MAX), status);
                postStatusChanged(0);
                sengineSetStatusVal(status, (len & PRI2STD_LEN_MAX));
            }
        }
    }
}

int senginePollingSlave(void) {
    Datas datas = {0};
    char status[MAX_BUF] = {0};
    uint8_t bin[MAX_BUF] = {0};
    uint16_t currLen = 0, appendLen = 0;
    int whatKind = 0, ret = 0, size = 0, i;

    FOR_EACH_IO_HDL_START;
        currLen = 0;
        appendLen = 0;
        ret = ioRead(ioHdl[x].ioType, ioHdl[x].hdl, bin, sizeof(bin));
        if (ret <= 0) {
            // LELOGW("senginePollingSlave ioRead [%d]", ret);
            continue;
        }
        size = ret;

        ret = sengineCall((const char *)ginScriptCfg->data.script, ginScriptCfg->data.size, S1_OPT_DO_SPLIT,
                bin, size, (uint8_t *)&datas, sizeof(Datas));
        // LELOG("senginePollingSlave "S1_OPT_DO_SPLIT" ret[%d], datasCountsLen[%d], datasLen[%d] sizeof(Datas) is [%d], ioHdl[x].ioType[%d]", ret, datas.datasCountsLen/2, datas.datasLen, sizeof(Datas), ioHdl[x].ioType);
        // for (i = 0; i < datas.datasCountsLen; i += sizeof(uint16_t)) {
        //     LELOG("senginePollingSlave datas.datasCounts[%x][%d]", datas.arrDatasCounts[i/2], datas.arrDatasCounts[i/2]);
        // }
        if (0 >= ret) {
            // LELOGW("senginePollingSlave sengineCall "S1_OPT_DO_SPLIT" [%d]", ret);
            datas.datasCountsLen = 2;
            datas.arrDatasCounts[0] = size;
        }
        for (i = 0; i < datas.datasCountsLen; i += sizeof(uint16_t)) {
            // LELOG("==>senginePollingSlave datas.datasCounts[%x][%d]", datas.arrDatasCounts[i/2], datas.arrDatasCounts[i/2]);
            memcpy(&currLen, &datas.arrDatasCounts[i/sizeof(uint16_t)], sizeof(uint16_t));
            // LELOG("[SENGINE]_s1OptDoSplit_[%d]_cmd: curr piece len[%d]", i/sizeof(uint16_t), currLen);
            memcpy(datas.arrDatas, &bin[appendLen], currLen);

            if (0)
            {
                int j = 0;
                extern int bytes2hexStr(const uint8_t *src, int srcLen, uint8_t *dst, int dstLen);
                uint8_t hexStr[96] = {0};
                LELOG("[SENGINE]datas.arrDatas currLen[%d], appendLen[%d]", currLen, appendLen);
                bytes2hexStr(&datas.arrDatas[j + appendLen], currLen, hexStr, sizeof(hexStr));
                LELOG("ioRead type[0x%x] bin[%s]", ioHdl[x].ioType, hexStr);
            }

            ret = sengineCall((const char *)ginScriptCfg->data.script, ginScriptCfg->data.size, S1_GET_VALIDKIND,
                    datas.arrDatas, currLen, (uint8_t *)&whatKind, sizeof(whatKind));
            // LELOG("sengineCall ret size [%d], currLen[%d] whatKind [%d]", ret, currLen, whatKind);
            if (0 >= ret) {
                LELOGW("senginePollingSlave sengineCall "S1_GET_VALIDKIND" [%d]", ret);
                continue;
            }
            switch (whatKind) {
                case WHATKIND_MAIN_DEV_RESET: {
                        extern void resetDevice(void);
                        resetDevice();
                    }
                    break;
                case WHATKIND_MAIN_DEV_DATA: {
                        extern int lelinkNwPostCmdExt(const void *node);
                        int len = 0;
                        len = sengineCall((const char *)ginScriptCfg->data.script, ginScriptCfg->data.size, S1_PRI2STD,
                                &datas.arrDatas[appendLen], currLen, (uint8_t *)status, sizeof(status));
                        if (len <= 0) {
                            LELOGW("senginePollingSlave sengineCall("S1_PRI2STD") [%d]", len);
                        } else {
                            sengineIODataHandler(status, len, 0);
                        }
                    }
                    break;
                // for sub devs
                case WHATKIND_SUB_DEV_RESET: {
                        LELOG("WHATKIND_SUB_DEV_RESET");
                    }
                    break;
                case WHATKIND_SUB_DEV_DATA:
                case WHATKIND_SUB_DEV_JOIN:
                case WHATKIND_SUB_DEV_LEAVE:
                case WHATKIND_SUB_DEV_INFO: {
                        int len;
                        SDevNode *tmpArr = sdevArray();
                        if (NULL == tmpArr) {
                            LELOGE("sdevArray is NULL");
                            break;
                        }
                        len = sengineCall((const char *)ginScriptCfg->data.script, ginScriptCfg->data.size, S1_PRI2STD,
                            datas.arrDatas, currLen, (uint8_t *)status, sizeof(status));
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
                        } else if (WHATKIND_SUB_DEV_INFO == whatKind) {
                            LELOG("WHATKIND_SUB_DEV_INFO");
                            sdevInfoRsp(tmpArr, status, len);
                        } else if (WHATKIND_SUB_DEV_DATA == whatKind) {
                            LELOG("WHATKIND_SUB_DEV_DATA");
                            sengineIODataHandler(status, len, 1);
                        } else if (WHATKIND_SUB_DEV_LEAVE == whatKind) {
                            LELOG("WHATKIND_SUB_DEV_LEAVE");
                            sdevRemove(tmpArr, status, len);
                        }
                    }
                    break;
                default:
                    //LELOGW("Unknow whatKind = %d", whatKind);
                    break;
            }

            appendLen += currLen;
            //LEPRINTF("\r\n");
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

int sengineS2GetBeingReservedInfo(const ScriptCfg2 *scriptCfg2, uint8_t strBeingReserved[MAX_RSV_NUM][2*MAX_UUID]) {
    int ret = 0, count = 0, i;
    uint8_t tmpBeingReserved[sizeof(int) + (MAX_RSV_NUM*(2*MAX_UUID))] = {0};
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
        memcpy(strBeingReserved[i], tmp, (2*MAX_UUID));
        tmp += (2*MAX_UUID);
        LELOGW("strBeingReserved[%d][%s]", (2*MAX_UUID), strBeingReserved[i]);
    }

    return count;
}

int sengineS2RuleHandler(const ScriptCfg2 *scriptCfg2, 
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
        ret = sengineSetAction((char *)buf, ret);
        LELOG("sengineS2RuleHandler sengineSetAction DONE [%d]", ret);      
        if (isCloudOnlined()) {
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
    ginIACache.cfg.num = privCfg.data.iaCfg.num;
    for (i = 0; i < MAX_IA; i++) {
        LELOGW("senginePollingRules privCfg.data.iaCfg.arrIA[%d] [%d]", i, privCfg.data.iaCfg.arrIA[i]);
        ginIACache.cfg.arrIA[i] = privCfg.data.iaCfg.arrIA[i];
        if (0 < privCfg.data.iaCfg.arrIA[i]) {
            memset(ginScriptCfg2, 0, sizeof(ScriptCfg2));
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
        privCfg.data.iaCfg.arrIA[whereToPut] = 0;
        privCfg.data.iaCfg.num--;
        lelinkStorageWritePrivateCfg(&privCfg);
        return found;
    }    

    LELOG("sengineRemoveRules found[%d] whereToPut[%d] -e ", found, whereToPut);
    return 0;
}

int test_lf_call(char *luacode, int size) {
    // int i = 0;
    // for (i = 0; i < (sizeof(func_list) / sizeof(FUNC_LIST) - 1); i++)
    // {
    //     sengineCall(luacode, size, func_list[i].func_name);
    // }
    return 0;
}


