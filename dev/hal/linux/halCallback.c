

#include "halHeader.h"
#include "protocol.h"

extern uint8_t ginBeCtrlToken[];

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
    APPLOG("halCBLocalReq -S\r\n");
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
        }break;
        case LELINK_CMD_CLOUD_HEARTBEAT_REQ: {
            if (LELINK_SUBCMD_CLOUD_HEARTBEAT_REQ == cmdInfo->subCmdId) {
                // optional, sdk need to set account
                const char *account = "\"test1\"";
                ret = len < strlen(account) ? len : strlen(account);
                memcpy(data, account, ret);
            }
        }break;
        case LELINK_CMD_CLOUD_REPORT_REQ: {
            if (LELINK_SUBCMD_CLOUD_REPORT_REQ == cmdInfo->subCmdId || 
                LELINK_SUBCMD_CLOUD_REPORT_OTA_QUERY_REQ == cmdInfo->subCmdId ||
                LELINK_SUBCMD_CLOUD_REPORT_OTA_DO_REQ == cmdInfo->subCmdId) {
                char uuid[64] = {0};
                strcpy(uuid, UUID_BEING_CTRL);
                ret = len < strlen(uuid) ? len : strlen(uuid);
                memcpy(data, uuid, ret);
                APPLOG("data [%s]\r\n", data);
            }
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
    char tmpUUID[MAX_UUID + 1] = {0};
    APPLOG("halCBRemoteRsp -s\r\n");
    memcpy(tmpUUID, cmdInfo->uuid, MAX_UUID);
    // APPLOG("halCBRemoteRsp status[%d] cmdId[%u] subCmdId[%u] seqId[%u] randID[%u] \r\n\
    //     passThru[%d] uuid[%s]\r\n", 
    //     cmdInfo->status, 
    //     cmdInfo->cmdId, 
    //     cmdInfo->subCmdId, 
    //     cmdInfo->seqId, 
    //     cmdInfo->randID, 
    //     cmdInfo->passThru, 
    //     tmpUUID);
    // APPLOG("halCBRemoteRsp len[%d][%s]\r\n", len, payloadBody);

    switch (cmdInfo->cmdId) {
        // remote ctrl
        case LELINK_CMD_CLOUD_MSG_CTRL_R2T_REQ: {
            if (LELINK_SUBCMD_CLOUD_MSG_CTRL_R2T_REQ == cmdInfo->subCmdId) {
            }
        }break;
        case LELINK_CMD_CLOUD_REPORT_RSP: {
            if (LELINK_SUBCMD_CLOUD_REPORT_RSP == cmdInfo->subCmdId) {
#include "jsmn/jsonv2.h"
#include "jsmn/jsgen.h"
                int ret = -1;
                int dir = 0;
                jsontok_t jsonToken[256];
                char tokenStr[AES_LEN*2 + 1];

                jobj_t jobj;
                // char *str = "{\"cloud\":\"1\",\"status\":{\"level\":\"3\",\"mark\":\"2\",\"pwr\":\"1\"},\"token\":\"1113a3e617b7f0e63df0e82879765e97\",\"uuid\":\"10000100011000510005123456abcdef\"}";

                ret = json_init(&jobj, jsonToken, 256, (char *)payloadBody, len);
                if (WM_SUCCESS != ret) {
                    return;
                }

                if (WM_SUCCESS != json_get_val_str(&jobj, "token", tokenStr, sizeof(tokenStr))) {
                    return;
                }

                hexStr2bytes(tokenStr, ginBeCtrlToken, AES_LEN);

            }
        }break;
        case LELINK_CMD_DISCOVER_RSP: {
            if (LELINK_SUBCMD_DISCOVER_RSP == cmdInfo->subCmdId) {
                APPLOG("halCBRemoteRsp LELINK_SUBCMD_DISCOVER_RSP [%s]\r\n", payloadBody);
            }
        }break;
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
    // APPLOG("halCBRemoteReq status[%d] cmdId[%u] subCmdId[%u] seqId[%u] randID[%u] \r\n\
    //     passThru[%d] uuid[%s]\r\n", 
    //     cmdInfo->status, 
    //     cmdInfo->cmdId, 
    //     cmdInfo->subCmdId, 
    //     cmdInfo->seqId, 
    //     cmdInfo->randID, 
    //     cmdInfo->passThru, 
    //     cmdInfo->uuid);
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

    }
    APPLOG("halCBLocalRsp [%d] -e\r\n", ret);
    return ret;
}
