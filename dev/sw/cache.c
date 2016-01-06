#include "leconfig.h"
#include "cache.h"  
#include "network.h"
#include "protocol.h"

/***********************************************
 Function: Create a empty stack;
 ************************************************/


/***********************************************
 Function: Print the stack element;
 ************************************************/
int qFullCache(PCACHE C)
{
    if (C->currsize == C->maxsize)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int qEmptyCache(PCACHE C)
{
    if (0 == C->currsize)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
int qEnCache(PCACHE C, void *val)
{
    int i = 0;
    //int bFind = 0;
    CACHE_NODE_TYPE *tmp = (CACHE_NODE_TYPE *) val;

    if (qFullCache(C))
        return 0;
    else
    {
        if (!tmp->seqId) // invalid seq id, it will only be used as Cache for send
        {
            if (0 == C->seqIdAuto)

            {
                C->seqIdAuto = 1;
            }
            else
            {
                C->seqIdAuto++;
            }
            tmp->seqId = C->seqIdAuto;
        }
        else  // it will only be used as Cache for remote req
        {
            for (i = 0; i < C->maxsize; i++)
            {
                if (tmp->seqId == ((CACHE_NODE_TYPE*) (C->pBase))[i].seqId)
                {
                    // already in Cache
                    return 0;
                }
            }
        }

        // find an empty space to put in
        for (i = 0; i < C->maxsize; i++)
        {
            // the current not has not been ocupied.
            if (0 == ((CACHE_NODE_TYPE*) (C->pBase))[i].seqId)
            {
                memcpy(&((CACHE_NODE_TYPE*) (C->pBase))[i], val,
                        sizeof(CACHE_NODE_TYPE));
                ((CACHE_NODE_TYPE*) (C->pBase))[i].seqId = tmp->seqId;

                C->currsize++;
                return 1;
            }
        }
    }
    LELOGE("qEnCache [%d/%d]\r\n", C->currsize, C->maxsize);
    return 0;
}

int qForEachfromCache(PCACHE C, int (*currNodeCB)(void *curr, void *uData), void *uData)
{
    int i = 0;
    int ret = 0;
    for (i = 0; i < C->maxsize; i++)
    {
        if (0 != ((CACHE_NODE_TYPE*) (C->pBase))[i].seqId)
        {
            ret = currNodeCB(&((CACHE_NODE_TYPE*) (C->pBase))[i], uData);
            if (ret > 0) {
                return ret;
            }
        }
        //i++;
    }

    return 0;
}

int qCheckForClean(PCACHE C, int (*isNeedDelCB)(void *curr))
{
    int i = 0;
    for (i = 0; i < C->maxsize; i++)
    {
        if (0 != ((CACHE_NODE_TYPE*) (C->pBase))[i].seqId)
        {
            if (isNeedDelCB(&((CACHE_NODE_TYPE*) (C->pBase))[i]))
            {
                memset(&((CACHE_NODE_TYPE*) (C->pBase))[i], 0,
                        sizeof(CACHE_NODE_TYPE));
                C->currsize--;
            }
        }
    }
    return C->currsize;

}

