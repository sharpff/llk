#ifndef __SENGINE_H__
#define __SENGINE_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_SCRIPT_SIZE (1024*16)
#define MAX_QUERY_COUNTS 16
#define MAX_ALL_QUERYS 128
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

extern ScriptCfg ginScriptCfg;

int sengineInit(void);
int sengineCall(const char *script, int scriptSize, const char *funcName, const uint8_t *input, int inputLen, uint8_t *output, int outputLen);
int sengineGetStatus(char *json, int jsonLen);
int sengineSetStatus(char *json, int jsonLen);
int sengineGetDevProfile(char *json, int jsonLen);

#ifdef __cplusplus
}
#endif

#endif