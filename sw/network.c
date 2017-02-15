#include "utility.h"
#include "network.h"
#include "protocol.h"
#include "io.h"

typedef struct
{
    int alloced;
    CommonCtx ctx;
    NodeData q[CACHE_MAX_NODE];
}PoolCommonCtx;

static PoolCommonCtx poolCommonCtx[MAX_CTX_SIZE];

static CommonCtx *newCtx()
{
    int i = 0;
    for (i = 0; i < MAX_CTX_SIZE; i++)
    {
        if (!poolCommonCtx[i].alloced)
        {
            memset(&(poolCommonCtx[i]), 0, sizeof(PoolCommonCtx));
            poolCommonCtx[i].ctx.cacheCmd.maxsize = CACHE_MAX_NODE;
            poolCommonCtx[i].ctx.cacheCmd.singleSize = sizeof(NodeData);
            poolCommonCtx[i].ctx.cacheCmd.pBase = poolCommonCtx[i].q;
            poolCommonCtx[i].alloced = 1;
            return &(poolCommonCtx[i].ctx);
        }
    }
    return NULL;
}

static void deleteCtx(CommonCtx *ctx)
{
    //free(ctx);
    //return;
    int i = 0;
    for (i = 0; i < MAX_CTX_SIZE; i++)
    {
        if (poolCommonCtx[i].alloced && (ctx == &(poolCommonCtx[i].ctx)))
        {
            poolCommonCtx[i].alloced = 0;
            return;
        }
    }
}

int nwInit() {
    return 0;
}

void nwDeinit() {

}

//#define something
void *lelinkNwNew(const char *remoteIP, int remotePort, int selfPort, void *ctx)
{
    CommonCtx *ctx_p = NULL;  
    int sock = 0;
    int broadcastEnable = 1;

    ctx_p = newCtx();

    if (0 == halNwNew(selfPort, 0, &sock, &broadcastEnable)) {
        ctx_p->selfPort = selfPort;
    } else {
        deleteCtx(ctx_p);
        return NULL;
    }

    ctx_p->sock = sock;
    if (remoteIP) {
        strcpy(ctx_p->remoteIP, remoteIP);
        ctx_p->remotePort = remotePort;
    }
    LELOG("socket [%d] rmt[%s:%d]", sock, remoteIP, remotePort);

    
    // wmprintf("nwNew token[%s]", stProfile.token);
    // wmprintf("nwNew did[%s]", stProfile.did);
    // wmprintf("nwNew payloadKey[%s]", stProfile.payloadKey);
    // wmprintf("nwNew payloadIv[%s]", stProfile.payloadIv);

    // strncpy(ctx_p->jlDevInfo.devid, stProfile.did, 32);
    // strncpy(ctx_p->jlDevInfo.payloadIv, "f9fa28f96135dd7e3200c36354317f14", 32);
    // strncpy(ctx_p->token, stProfile.token, 32);
    // strncpy(ctx_p->productId, stProfile.pid, 32);
    //strcpy(devInfo->payloadKey, "201506151f3329cdf123b4430d112233");
    //strcpy(devInfo->token, "201506151f3329cdf123b4430d112233");
    //strcpy(devInfo->priKey, "201506151f3329cdf123b4430d112233");
    //hexStr2bytes("f1fa28f96135dd7e3222c36354317f14", devInfo->pubKey);
    //hexStr2bytes("f1fa28f96135dd7e3222c36354317f14", devInfo->sessionKey);
    //strcpy(devInfo->sessionKey, "201506151f3329cdf123b4430d112233");

    return (void *)ctx_p;
}

int lelinkNwDelete(void *ctx)
{
    CommonCtx *ctx_p = (CommonCtx *)ctx;
    if (!ctx)
        return -1;

    halNwDelete(ctx_p->sock);
    deleteCtx(ctx_p);
    return 0;
}

int nwUDPSendto(void *ctx, const char *ip, int port, const uint8_t *buf, int len)
{
    int ret;
    CommonCtx *info = (CommonCtx *)ctx;

    ret = halNwUDPSendto(info->sock, ip, port, buf, len);
    LELOG("socket[%d] nwUDPSendto [%s:%d][%d]", info->sock, ip, port, ret);

    return ret;
}


int nwUDPRecvfrom(void *ctx, uint8_t *buf, int len, char *ip, int lenIP, uint16_t *port)
{
	int ret;
	char selfIP[MAX_IPLEN] = { 0 };
	int selfPort = 0, lenSelfIP = 0;
    CommonCtx *info = (CommonCtx *)ctx;

    ret = halNwUDPRecvfrom(info->sock, buf, len, ip, lenIP, port);
    if ((lenSelfIP = halGetSelfAddr(selfIP, MAX_IPLEN, &selfPort)) > 0) {
        if (!strncmp(ip, selfIP, lenIP > lenSelfIP ? lenSelfIP : lenIP)) {                                                                      
        	return 0;
        }
    }   
    if (ret >= 0) {
        LELOG("nwUDPRecvfrom [%s:%d][%d]", ip, *port, ret);
    }
    return ret;
}



/*
int nwGetCmd(CommonCtx *ctx_p, void **node)
{
    if (Decache(ctx_p, node))
    {
        return 1;
    }

    return 0;
    //return ctx_p->needReq || ctx_p->rspVal;
}
*/

