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
    int ret = 0;
    char buf[512] = {0};
    APPLOG("halCBRemoteReq -s");
    memcpy(buf, payloadBody, sizeof(buf) > len ? len : sizeof(buf) - 1);
    APPLOG("halCBRemoteReq len[%d/%d][%s]", len, sizeof(buf), buf);

    switch (cmdInfo->cmdId) {
        case LELINK_CMD_HELLO_REQ: {
            if (LELINK_SUBCMD_HELLO_REQ == cmdInfo->subCmdId) {
                // TODO: SDK got a new device
                ret = 1;
            }
        }break;
        case LELINK_CMD_CLOUD_MSG_CTRL_R2T_REQ: {
            if (LELINK_SUBCMD_CLOUD_MSG_CTRL_R2T_REQ == cmdInfo->subCmdId) {
                // if u have got the RemoteReq, set ret as 1
                // cmdInfo->uuid
                ret = 1;
            }
        }break;
    }

    return ret;
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
