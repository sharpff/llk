#ifndef __CACHE_H_  
#define __CACHE_H_  

#ifdef __cplusplus
extern "C"
{
#endif

#include "leconfig.h"

#define CACHE_MAX_NODE 30
// #define CACHE_NODE_TYPE NodeData

typedef struct {
	CACHE_NODE_HEADER;
}NodeHead;

typedef struct CACHE
{
    void *pBase;
    int maxsize;
    int singleSize;
    int currsize;
    uint16_t flagAuto;
    uint32_t sDevVer; // 0x00030001 app(0x0003) + sdk(0x0001)
}CACHE, *PCACHE;

typedef int (*NodeCB_t)(void *curr);

int qFullCache(PCACHE C);
int qEmptyCache(PCACHE C);
int qEnCache(PCACHE C, void *val);
int qForEachfromCache(PCACHE C, int (*currNodeCB)(void *curr, void *uData), void *uData);
int qCheckForClean(PCACHE C, int (*isNeedDelCB)(void *curr));


#ifdef __cplusplus
}
#endif
#endif 
