#include "halHeader.h"
#include <lelink/sw/protocol.h>

// static uint8_t ginUUID[32];

/* 
 * prepare data for local req.
 * return value: 
 * positive num, if u need to rsp the remote. then the CBLocalRsp will be called.
 * nagtive num or  0, drop this & do nothing.
 * 
 * cmdInfo is local
 */
int halCBLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *data, int len) {
    int ret = 0;
    APPLOG("halCBLocalReq -s\r\n");
    switch (cmdInfo->cmdId) {
        // case LELINK_CMD_HELLO_REQ: {
        //     if (LELINK_SUBCMD_HELLO_REQ == cmdInfo->subCmdId) {
        //         const char *helloReq = "{\"msg\":\"hello\"}";
        //         ret = len < strlen(helloReq) ? len : strlen(helloReq);
        //         memcpy(data, helloReq, ret);
        //     }
        // }break;
        case LELINK_CMD_DISCOVER_REQ: {
        
        }break;
        case LELINK_CMD_CTRL_REQ: {
            if (LELINK_SUBCMD_CTRL_CMD_REQ == cmdInfo->subCmdId) {
                const char *reqCtrlStd = "{\"ctrl\":{\"pwr\":1,\"mark\":3,\"level\":5}}";
                ret = len < strlen(reqCtrlStd) ? len : strlen(reqCtrlStd);
                memcpy(data, reqCtrlStd, ret);
            }
        }break;
        // remote ctrl
        case LELINK_CMD_CLOUD_MSG_CTRL_C2R_REQ: {
            if (LELINK_SUBCMD_CLOUD_MSG_CTRL_C2R_REQ == cmdInfo->subCmdId) {
                const char *rmtCtrl = "{\"ctrl\":{\"pwr\":1,\"mark\":3,\"level\":5}}";
                ret = len < strlen(rmtCtrl) ? len : strlen(rmtCtrl);
                memcpy(data, rmtCtrl, ret);
            }
             // else if (LELINK_SUBCMD_CLOUD_MSG_CTRL_C2R_REQ == cmdInfo->subCmdId)
        }break;
    }
    APPLOG("halCBLocalReq [%d] -e\r\n", ret);
    return ret;
}

/* 
 * rsp from remote.
 * 
 * cmdInfo is remote
 */
void halCBRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *payloadBody, int len) {
    APPLOG("halCBRemoteRsp -s\r\n");
    APPLOG("halCBRemoteRsp status[%d] cmdId[%u] subCmdId[%u] seqId[%u] randID[%u] \r\n\
        passThru[%d] uuid[%s]\r\n", 
        cmdInfo->status, 
        cmdInfo->cmdId, 
        cmdInfo->subCmdId, 
        cmdInfo->seqId, 
        cmdInfo->randID, 
        cmdInfo->passThru, 
        cmdInfo->uuid);
    APPLOG("halCBRemoteRsp len[%d][%s]\r\n", len, payloadBody);

    switch (cmdInfo->cmdId) {
        // remote ctrl
        case LELINK_CMD_CLOUD_MSG_CTRL_R2T_REQ: {
            if (LELINK_SUBCMD_CLOUD_MSG_CTRL_R2T_REQ == cmdInfo->subCmdId) {
            }
        }
    }
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
    APPLOG("halCBRemoteReq -s\r\n");
    APPLOG("halCBRemoteReq status[%d] cmdId[%u] subCmdId[%u] seqId[%u] randID[%u] \r\n\
        passThru[%d] uuid[%s]\r\n", 
        cmdInfo->status, 
        cmdInfo->cmdId, 
        cmdInfo->subCmdId, 
        cmdInfo->seqId, 
        cmdInfo->randID, 
        cmdInfo->passThru, 
        cmdInfo->uuid);
    APPLOG("halCBRemoteReq len[%d][%s]\r\n", len, payloadBody);

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
    int ret = 0;
    APPLOG("halCBLocalRsp -s\r\n");
    switch (cmdInfo->cmdId) {
        case LELINK_CMD_HELLO_RSP: {
            if (LELINK_SUBCMD_HELLO_RSP == cmdInfo->subCmdId) {
                char helloReq[] = "{\"msg\":\"hello\"}";
                ret = nwLenOut < strlen(helloReq) ? nwLenOut : strlen(helloReq);
                memcpy(nw, helloReq, ret);
            }
        }break;
        case LELINK_CMD_DISCOVER_REQ: {
        
        }break;
        case LELINK_CMD_CTRL_REQ: {
        
        }break;

        case LELINK_CMD_CLOUD_MSG_CTRL_R2T_RSP: {
            if (LELINK_SUBCMD_CLOUD_MSG_CTRL_R2T_RSP == cmdInfo->subCmdId) {
                // char status[64] = {0};
                const char *tmp = "{\"status\":{\"pwr\":1,\"mark\":3,\"level\":5},\"cloud\":1,\"test\":1}";
                memcpy(nw, tmp, strlen(tmp));
                return strlen(tmp);
            }
        }break;
    }
    APPLOG("halCBLocalRsp [%d] -e\r\n", ret);
    return ret;
}
