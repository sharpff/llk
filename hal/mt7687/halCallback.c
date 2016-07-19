#include "halHeader.h"
#include "protocol.h"
#include "ota.h"
#include "io.h"
/* 
 * prepare data for local req.
 * return value: 
 * positive num, if u need to rsp the remote. then the CBLocalRsp will be called.
 * nagtive num or  0, drop this & do nothing.
 * 
 * cmdInfo is local
 */
int halCBLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *data, int len) {
    return 0;
}

/* 
 * rsp from remote.
 * 
 * cmdInfo is remote
 */
void halCBRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *payloadBody, int len) {

}

/* 
 * req from remote.
 * return value: 
 * positive num, if u need to rsp the remote. then the CBLocalRsp will be called.
 * nagtive num, drop this & do nothing.
 * 0, callback will come again. 
 * 
 * cmdInfo is retmote
 */
int halCBRemoteReq(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *payloadBody, int len) {
    return 0;
}

/* 
 * prepare data for local rsp
 * return value:
 * positive num, the len of sending data.
 * nagtive num, drop this & do nothing.
 * 0, means payload body is empty. 
 *
 * cmdInfo is retmote
 */
int halCBLocalRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len, char *nw, int nwLenOut) {
    return 0;
}
