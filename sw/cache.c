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
    NodeHead *tmp = (NodeHead *) val;

    if (qFullCache(C))
        return 0;
    else
    {
        if (!tmp->flag) // invalid seq id, it will only be used as Cache for send
        {
            if (0 == C->flagAuto)

            {
                C->flagAuto = 1;
            }
            else
            {
                C->flagAuto++;
            }
            tmp->flag = C->flagAuto;
        }
        // else  // it will only be used as Cache for remote req
        // {
        //     for (i = 0; i < C->maxsize; i++)
        //     {
        //         if (tmp->flag == ((NodeHead*) (C->pBase))[i*C->singleSize].flag)
        //         {
        //             // already in Cache
        //             return 0;
        //         }
        //     }
        // }

        // find an empty space to put in
        for (i = 0; i < C->maxsize; i++)
        {
            // the current not has not been ocupied.
            if (0 == ((NodeHead*)&(((uint8_t*)(C->pBase))[i*C->singleSize]))->flag)
            {
                MUTEX_LOCK;
                memcpy(&(((uint8_t*)(C->pBase))[i*C->singleSize]), val, C->singleSize);
                ((NodeHead*)&(((uint8_t*)(C->pBase))[i*C->singleSize]))->flag = tmp->flag;
                C->currsize++;
                MUTEX_UNLOCK;
                return tmp->flag;
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
    for (i = 0; i < C->maxsize; i++) {
        if (0 != ((NodeHead*)&(((uint8_t*)(C->pBase))[i*C->singleSize]))->flag) {
            ret = currNodeCB(&(((uint8_t*)(C->pBase))[i*C->singleSize]), uData);
            if (ret > 0) {
                return i;
            }
        }
        //i++;
    }

    return -1;
}

int qCheckForClean(PCACHE C, int (*isNeedDelCB)(void *curr))
{
    int i = 0;
    for (i = 0; i < C->maxsize; i++)
    {
        if (0 != ((NodeHead*)&(((uint8_t*)(C->pBase))[i*C->singleSize]))->flag)
        {
            if (isNeedDelCB(&(((uint8_t*)(C->pBase))[i*C->singleSize])))
            {
                MUTEX_LOCK;
                memset(&(((uint8_t*)(C->pBase))[i*C->singleSize]), 0, C->singleSize);
                C->currsize--;
                MUTEX_UNLOCK;
            }
        }
    }
    return C->currsize;

}

