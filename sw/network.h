#ifndef __NETWORK_H__
#define __NETWORK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "leconfig.h"
#include "cache.h"
#include "pack.h"

#define UDP_MTU 1500
#define MAX_CTX_SIZE 3
#define MAX_SVR_LIST 4


typedef struct _CommonCtx CommonCtx;
// typedef struct {
//     int cnt;
//     char list[4][128];
// } ServerListResp;

struct _CommonCtx
{
    int sock;
    int remotePort;
    // CommonCtx *ctx;
    char remoteIP[MAX_IPLEN];
    // joylinkDevInfo jlDevInfo;
    // ServerListResp lstSvr;
    int selfPort;
    CACHE cacheCmd;
    // char sessionKey[64];
    uint8_t nwBuf[UDP_MTU];
    uint8_t protocolBuf[UDP_MTU];
    // char token[33];
 //    char productId[40];

// #if defined(__ATMEL_G55__)
//     xQueueHandle qAsyncData;
//     os_mutex_t mtxAsyncData;
// #endif
};




int nwInit();
void nwDeinit();

void *lelinkNwNew(const char *remoteIP, int remotePort, int selfPort, void *ctx);
int lelinkNwDelete(void *ctx);

int nwUDPSendto(void *ctx, const char *ip, int port, const uint8_t *buf, int len);
int nwUDPRecvfrom(void *ctx, uint8_t *buf, int len, char *ip, int lenIP, uint16_t *port);


#ifdef __cplusplus
}
#endif

#endif
