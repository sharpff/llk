#ifndef __SENGINE_H__
#define __SENGINE_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "ota.h"
#include "io.h"

#define MAX_SCRIPT_SIZE MAX_PROFILE_SIZE
#define MAX_QUERY_COUNTS 16
#define MAX_ALL_QUERYS 128

/*
 * fw script
 */
#define S1_GET_CVTTYPE "s1GetCvtType"
#define S1_GET_QUERIES "s1GetQueries"
#define S1_STD2PRI "s1CvtStd2Pri"
#define S1_PRI2STD "s1CvtPri2Std"
#define S1_GET_VALIDKIND "s1GetValidKind"
#define S1_GET_VER "s1GetVer"
#define S1_OPT_HAS_SUBDEVS "s1OptHasSubDevs"
#define S1_OPT_DO_SPLIT "s1OptDoSplit"
#define S1_OPT_MERGE_ST2ACT "s1OptMergeCurrStatus2Action"

/*
 * ia script
 */
#define S2_IS_VALID "s2IsValid"
#define S2_IS_VALID_EXT "s2IsValidExt"
#define S2_GET_SELFNAME "s2GetSelfName"
#define S2_GET_RULETYPE "s2GetRuleType"
#define S2_GET_BERESERVED "s2GetBeingReservedInfo"
#define S2_GET_ISOK "s2IsConditionOK"
#define S2_GET_ISOK_EXT "s2IsConditionOKExt"
#define S2_GET_BECMD "s2GetSelfCtrlCmd"
#define MAX_IA_BUF 64
#define MAX_RSV_NUM 4 /* max reserved num for a single IA */ 

#define SDEV_MAX_STATUS MAX_BUF/2

/*
 * 这些enum值通过FW脚本中的s1GetValidKind进行处理并返回
 * 标示当前在脚本中分析出来的数据类型
 */
enum {
	WHATKIND_MAIN_DEV_RESET = 1,
	WHATKIND_MAIN_DEV_DATA,
    WHATKIND_SUB_DEV_RESET = 10,
	WHATKIND_SUB_DEV_DATA,
    WHATKIND_SUB_DEV_JOIN,
    WHATKIND_SUB_DEV_LEAVE,
    WHATKIND_SUB_DEV_INFO,
};

typedef enum {
    QUERIETYPE_INVAIL = 0,
    QUERIETYPE_STATE = 1,       // 1, 查询设备状态
    QUERIETYPE_WAITCONFIG = 2,  // 2, 设备进入配置状态
    QUERIETYPE_CONNECTING = 3,  // 3, 设备进入连接AP状态
    QUERIETYPE_CONNECTED = 4,   // 4, 已经连接到AP，可以本地控制
    QUERIETYPE_CLOUD = 5,       // 5, 已经正常连到云服务，可远程控制
    // QUERIETYPE_SUBDEV_LIST = (1 << 16) & 0xFFFF0000, // 0xFFFF0001, query sub dev state
    // QUERIETYPE_SUBDEV_INFO = (2 << 16) & 0xFFFF0000, // 0xFFFF0001, query sub dev state
}QuerieType_t;

#ifdef LELINK_PACK
#pragma pack(1)
#endif
/*
 * script info
 */
typedef struct {
    int size;
    uint8_t script[MAX_SCRIPT_SIZE];
}LELINK_ALIGNED ScriptData;

typedef struct {
    ScriptData data;
    uint8_t csum;
}LELINK_ALIGNED ScriptCfg;
#ifdef LELINK_PACK
#pragma pack()
#endif

typedef struct {
    uint16_t queriesLen;
    uint16_t queriesCountsLen;
    uint8_t arrQueries[MAX_ALL_QUERYS];
    uint16_t arrQueriesCounts[MAX_QUERY_COUNTS];
}Queries;

typedef struct {
    uint16_t datasLen;
    uint16_t datasCountsLen;
    uint8_t arrDatas[MAX_ALL_QUERYS];
    uint16_t arrDatasCounts[MAX_QUERY_COUNTS];
}Datas;

typedef struct {
    CACHE_NODE_NBASE;
    char sdevStatus[SDEV_MAX_STATUS]; // as json object "sDevStatus"
    uint8_t occupied;
    /* 
     * isSDevInfoDone identify the mask of these info
     * 0x01. endpoint list(active response)
     * 0x02. cluster done(simple descriptor response)
     * 0x04. man done(node descriptor response)
     * 0x08. timeout 
     */
    uint8_t isSDevInfoDone;
}SDevNode;

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

extern IA_CACHE ginIACache;
extern ScriptCfg *ginScriptCfg;
extern ScriptCfg *ginScriptCfg2;

int sengineInit(void);
int sengineCall(const char *script, int scriptSize, const char *funcName, const uint8_t *input, int inputLen, uint8_t *output, int outputLen);
int sengineHasDevs(void);
int sengineGetStatus(char *json, int jsonLen);
int sengineSetStatus(char *json, int jsonLen);
int sengineGetTerminalProfileCvtType(char *json, int jsonLen);
int sengineQuerySlave(QuerieType_t type);
int senginePollingSlave(void);
int senginePollingRules(const char *jsonRmt, int jsonLen);
int sengineRemoveRules(const char *name);



#ifdef __cplusplus
}
#endif

#endif
