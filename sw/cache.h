#ifndef __CACHE_H_  
#define __CACHE_H_  

#ifdef __cplusplus
extern "C"
{
#endif

#define CACHE_MAX_NODE 10
#define CACHE_NODE_TYPE NodeData

// #define CACHE_NODE_HEAD CMD_HEADER_INFO

// typedef struct
// {
//     CACHE_NODE_HEAD;
// }CACHEHead;

typedef struct CACHE
{
    void *pBase;
    int currsize;
    int maxsize; 
    unsigned short seqIdAuto;
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
