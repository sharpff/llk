#ifndef __SENGINE_H__
#define __SENGINE_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "ota.h"

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


/*
 * script info
 */
typedef struct {
    int size;
    uint8_t script[MAX_SCRIPT_SIZE];
}ALIGNED ScriptData;

typedef struct {
    ScriptData data;
    uint8_t csum;
}ALIGNED ScriptCfg;


typedef struct {
    uint16_t queriesLen;
    uint16_t queriesCountsLen;
    uint8_t arrQueries[MAX_ALL_QUERYS];
    uint16_t arrQueriesCounts[MAX_QUERY_COUNTS];
}Queries;

extern ScriptCfg *ginScriptCfg;
extern ScriptCfg *ginScriptCfg2;

int sengineInit(void);
int sengineCall(const char *script, int scriptSize, const char *funcName, const uint8_t *input, int inputLen, uint8_t *output, int outputLen);
int sengineGetStatus(char *json, int jsonLen);
int sengineSetStatus(char *json, int jsonLen);
int sengineGetTerminalProfileCvtType(char *json, int jsonLen);
int sengineQuerySlave(void);
int senginePollingSlave(void);
int senginePollingRules(const char *jsonRmt, int jsonLen);


#ifdef __cplusplus
}
#endif

#endif