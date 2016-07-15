#include "halHeader.h"
#if defined(__MRVL_SDK3_3__)
#include "protocol.h"
#else
#include <lelink/sw/protocol.h>
#endif

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
    APPLOG("halCBLocalReq -S");
    char cmdCtrl[64] = "{\"ctrl\":{\"pwr\":1,\"action\":1}}";
    // char *reqCtrlStd = "{\"ctrl\":{\"pwr\":1,\"action\":4}}";
    //                    "{\"ctrl\":{\"pwr\":1,\"action\":4}}";

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
                ret = len < strlen(cmdCtrl) ? len : strlen(cmdCtrl);
                memcpy(data, cmdCtrl, ret);
            }
        }break;
        // remote ctrl
        case LELINK_CMD_CLOUD_MSG_CTRL_C2R_REQ: {
            if (LELINK_SUBCMD_CLOUD_MSG_CTRL_C2R_REQ == cmdInfo->subCmdId) {
                ret = len < strlen(cmdCtrl) ? len : strlen(cmdCtrl);
                memcpy(data, cmdCtrl, ret);
            }
             // else if (LELINK_SUBCMD_CLOUD_MSG_CTRL_C2R_REQ == cmdInfo->subCmdId)
        }break;
        // case LELINK_CMD_CLOUD_REPORT_REQ: {
        //     if (LELINK_SUBCMD_CLOUD_REPORT_REQ == cmdInfo->subCmdId || 
        //         LELINK_SUBCMD_CLOUD_REPORT_OTA_QUERY_REQ == cmdInfo->subCmdId ||
        //         LELINK_SUBCMD_CLOUD_REPORT_OTA_DO_REQ == cmdInfo->subCmdId) {
        //         char uuid[64] = {0};
        //         strcpy(uuid, UUID_BEING_CTRL);
        //         ret = len < strlen(uuid) ? len : strlen(uuid);
        //         memcpy(data, uuid, ret);
        //         APPLOG("data [%s]", data);
        //     }
        // }break;
    }
    APPLOG("halCBLocalReq [%d] -e", ret);
    return ret;
}

/* 
 * rsp from remote.
 * 
 * cmdInfo is remote
 */
void halCBRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *payloadBody, int len) {
    char tmpUUID[MAX_UUID + 1] = {0};
    // APPLOG("halCBRemoteRsp -s");
    memcpy(tmpUUID, cmdInfo->uuid, MAX_UUID);

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
    int ret = 0;
    APPLOG("halCBLocalRsp -s");
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

    }
    APPLOG("halCBLocalRsp [%d] -e", ret);
    return ret;
}
