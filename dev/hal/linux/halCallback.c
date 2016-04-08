

#include "halHeader.h"
#include "protocol.h"
#include "ota.h"

extern uint8_t ginBeCtrlToken[];
static uint8_t ginOTAUrl[RSA_LEN + 128] = {0};
extern char *ginCtrlUUID;
extern char *ginCtrlCmd1;
extern char *ginCtrlCmd2;

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
#if 0
    const char *cmd1 = "{\"ctrl\":{\"pwr\":1,\"action\":1}}";
    const char *cmd2 = "{\"ctrl\":{\"pwr\":1,\"action\":2}}";
    const char *cmd3 = "{\"ctrl\":{\"pwr\":1,\"action\":3}}";
    const char *cmd4 = "{\"ctrl\":{\"pwr\":1,\"action\":4}}";
#else
    char *cmd1 = ginCtrlCmd1;
    char *cmd2 = ginCtrlCmd2;
    // char *cmd1 = "{\"ctrl\":{\"pwr\":1,\"idx1\":1,\"idx2\":1,\"idx3\":1,\"idx4\":1}}";
    // char *cmd2 = "{\"ctrl\":{\"pwr\":1,\"idx1\":0,\"idx2\":1,\"idx3\":1,\"idx4\":1}}";
    // const char *cmd3 = "{\"ctrl\":{\"pwr\":1,\"idx1\":0,\"idx2\":0,\"idx3\":0,\"idx4\":0}}";
    // const char *cmd3 = "{\"ctrl\":{\"pwr\":1,\"idx1\":0,\"idx2\":0,\"idx3\":0,\"idx4\":0}}";
    // const char *cmd4 = "{\"ctrl\":{\"pwr\":1,\"action\":4}}";
#endif
    char cmdCtrl[64] = {0};
    static int a = 0;
    // strcpy(cmdCtrl, cmd4);

    // char cmdCtrl[64] = "{\"ctrl\":{\"idx2\":1,\"idx3\":1}}";
    // char cmdCtrl[64] = "{\"ctrl\":{\"idx1\":1}}";
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
                if (a) {
                    strcpy(cmdCtrl, cmd1);
                } else {
                    strcpy(cmdCtrl, cmd2);
                }
                ret = len < strlen(cmdCtrl) ? len : strlen(cmdCtrl);
                memcpy(data, cmdCtrl, ret);
                APPLOG("CTRL req[%d][%s]", ret, cmdCtrl);
            }
        }break;
        // remote ctrl
        case LELINK_CMD_CLOUD_MSG_CTRL_C2R_REQ: {
            if (LELINK_SUBCMD_CLOUD_MSG_CTRL_C2R_REQ == cmdInfo->subCmdId) {
                if (a) {
                    strcpy(cmdCtrl, cmd1);
                } else {
                    strcpy(cmdCtrl, cmd2);
                }
                ret = len < strlen(cmdCtrl) ? len : strlen(cmdCtrl);
                memcpy(data, cmdCtrl, ret);
                APPLOG("C2R req[%d][%s]", ret, cmdCtrl);
            } else if (LELINK_SUBCMD_CLOUD_MSG_CTRL_C2R_DO_OTA_REQ == cmdInfo->subCmdId) {
                int type = 0, sizeOTA = 0;
                sizeOTA = strlen(ginOTAUrl + RSA_LEN) + RSA_LEN;
                // test only for trig a OTA
                if (RSA_LEN >= sizeOTA) {
                    type = OTA_TYPE_FW;
                    // type = OTA_TYPE_FW_SCRIPT;
                    // type = OTA_TYPE_IA_SCRIPT;
                    // type = OTA_TYPE_AUTH;
                    // type = OTA_TYPE_PRIVATE;
                    switch (type) {
                        case OTA_TYPE_FW: {
                            sprintf(ginOTAUrl + RSA_LEN, "{\"url\":\"%s\",\"type\":%d,\"force\":%d}", "http://115.182.63.167/feng/le_demo.bin", type, 35);
                        } break;
                        case OTA_TYPE_FW_SCRIPT: {
                            // sprintf(ginOTAUrl + RSA_LEN, "{\"url\":\"%s\",\"type\":%d,\"force\":%d}", "http://115.182.63.167/feng/dooya.lua", type, 35);
                            // sprintf(ginOTAUrl + RSA_LEN, "{\"url\":\"%s\",\"type\":%d,\"force\":%d}", "http://115.182.63.167/feng/honyar.lua", type, 35);
                            // sprintf(ginOTAUrl + RSA_LEN, "{\"url\":\"%s\",\"type\":%d,\"force\":%d}", "http://115.182.63.167/feng/honyar2.lua", type, 35);
                            sprintf(ginOTAUrl + RSA_LEN, "{\"url\":\"%s\",\"type\":%d,\"force\":%d}", "http://115.182.63.167/feng/dingding.lua", type, 35);
                        } break;
                        case OTA_TYPE_IA_SCRIPT: {
                            // sprintf(ginOTAUrl + RSA_LEN, "{\"url\":\"%s\",\"type\":%d,\"force\":%d}", "http://115.182.63.167/feng/ia_forDooya_honyarOn.lua", type, 35);
                            sprintf(ginOTAUrl + RSA_LEN, "{\"url\":\"%s\",\"type\":%d,\"force\":%d}", "http://115.182.63.167/feng/ia_forDooya_honyarOff.lua", type, 35);
                            // sprintf(ginOTAUrl + RSA_LEN, "{\"url\":\"%s\",\"type\":%d,\"force\":%d}", "http://115.182.63.167/feng/ia_forDooya_switcherOn.lua", type, 35);
                            // sprintf(ginOTAUrl + RSA_LEN, "{\"url\":\"%s\",\"type\":%d,\"force\":%d}", "http://115.182.63.167/feng/ia_forDooya_switcherOff.lua", type, 35);
                            // sprintf(ginOTAUrl + RSA_LEN, "{\"url\":\"%s\",\"type\":%d,\"force\":%d}", "http://115.182.63.167/feng/ia_forDooya_hOn_sOn.lua", type, 35);
                            // sprintf(ginOTAUrl + RSA_LEN, "{\"url\":\"%s\",\"type\":%d,\"force\":%d}", "http://115.182.63.167/feng/ia_forDooya_hOff_sOff.lua", type, 35);
                        } break;
                        case OTA_TYPE_AUTH: {
                            sprintf(ginOTAUrl + RSA_LEN, "{\"url\":\"%s\",\"type\":%d,\"force\":%d}", "http://115.182.63.167/feng/0x1c2000.bin", type, 35);
                        } break;
                        case OTA_TYPE_PRIVATE: {
                            sprintf(ginOTAUrl + RSA_LEN, "{\"url\":\"%s\",\"type\":%d,\"force\":%d}", "http://115.182.63.167/feng/0x1c8000.bin", type, 35);
                        } break;
                        default: {
                            APPLOG("unknown type");
                        } break;
                    }
                    sizeOTA += strlen(ginOTAUrl + RSA_LEN);
                }
                // test only END
                memcpy(data, ginOTAUrl, len > sizeOTA ? sizeOTA : len);
                
                APPLOG("LELINK_SUBCMD_CLOUD_MSG_CTRL_C2R_DO_OTA_REQ [%d][%s]", sizeOTA, ginOTAUrl + RSA_LEN);
                ret = sizeOTA;
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
            char uuid[64] = {0};
            // PF-FW-PROTOCOL-FWS
            char ver[] = {"\"1-0.9.9-1-1.0\""};
            if (LELINK_SUBCMD_CLOUD_REPORT_REQ == cmdInfo->subCmdId) {
                strcpy(uuid, ginCtrlUUID);
                ret = sprintf(data, "{\"uuid\":\"%s\"}", uuid);
            } else if (LELINK_SUBCMD_CLOUD_REPORT_OTA_QUERY_REQ == cmdInfo->subCmdId) {
                int type = OTA_TYPE_FW;
                // int type = OTA_TYPE_FW_SCRIPT;
                // int type = OTA_TYPE_IA_SCRIPT;
                strcpy(uuid, ginCtrlUUID);
                ret = sprintf(data, "{\"uuid\":\"%s\",\"type\":%d,\"ver\":%s}", uuid, type, ver);
                // memcpy(data, uuid, ret);
                APPLOG("OTA query data [%d][%s]", ret, data);
            }
        }break;

    }
    APPLOG("halCBLocalReq [%d] -e", ret);

    a = ~a;

    return ret;
}

/* 
 * rsp from remote.
 * 
 * cmdInfo is remote
 */
void halCBRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *payloadBody, int len) {
    char tmpUUID[MAX_UUID + 1] = {0};
    char tmpBuf[1500] = {0};
    if (len > 0) {
        memcpy(tmpBuf, payloadBody, len);
    }
    APPLOG("halCBRemoteRsp id[%d][%d] [%d][%s] -s", cmdInfo->cmdId, cmdInfo->subCmdId, len, tmpBuf);
    memcpy(tmpUUID, cmdInfo->uuid, MAX_UUID);
    // APPLOG("halCBRemoteRsp status[%d] cmdId[%u] subCmdId[%u] seqId[%u] randID[%u] \
    //     passThru[%d] uuid[%s]", 
    //     cmdInfo->status, 
    //     cmdInfo->cmdId, 
    //     cmdInfo->subCmdId, 
    //     cmdInfo->seqId, 
    //     cmdInfo->randID, 
    //     cmdInfo->passThru, 
    //     tmpUUID);
    // APPLOG("halCBRemoteRsp len[%d][%s]", len, payloadBody);

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
                char tmp[512] = {0};
                jsontok_t jsonToken[256];
                char tokenStr[AES_LEN*2 + 1] = {0};
                jobj_t jobj;
                memcpy(tmp, payloadBody, len < sizeof(tmp) ? len : sizeof(tmp) - 1);

                // char *str = "{\"cloud\":\"1\",\"status\":{\"level\":\"3\",\"mark\":\"2\",\"pwr\":\"1\"},\"token\":\"1113a3e617b7f0e63df0e82879765e97\",\"uuid\":\"10000100011000510005123456abcdef\"}";
                ret = json_init(&jobj, jsonToken, sizeof(jsonToken), (char *)payloadBody, len);
                if (WM_SUCCESS != ret) {
                    return;
                }

                if (WM_SUCCESS != json_get_val_str(&jobj, "token", tokenStr, sizeof(tokenStr))) {
                    return;
                }

                hexStr2bytes(tokenStr, ginBeCtrlToken, AES_LEN);
                // APPLOG("report rsp[%d][%s]", len, tmp);
            } else if (LELINK_SUBCMD_CLOUD_REPORT_OTA_QUERY_RSP == cmdInfo->subCmdId) {
                memset(ginOTAUrl, 0, sizeof(ginOTAUrl));
                memcpy(ginOTAUrl, payloadBody, len > sizeof(ginOTAUrl) ? sizeof(ginOTAUrl) : len);                
                APPLOG("set ota url [%d][%s]", RSA_LEN + strlen(ginOTAUrl + RSA_LEN), ginOTAUrl + RSA_LEN);
            }

        }break;
        case LELINK_CMD_DISCOVER_RSP: {
            if (LELINK_SUBCMD_DISCOVER_RSP == cmdInfo->subCmdId) {
                // APPLOG("halCBRemoteRsp LELINK_SUBCMD_DISCOVER_RSP [%s]", payloadBody);
            } else if (LELINK_SUBCMD_DISCOVER_STATUS_CHANGED_RSP == cmdInfo->subCmdId) {
                // APPLOG("halCBRemoteRsp LELINK_SUBCMD_DISCOVER_STATUS_CHANGED_RSP [%s]", payloadBody);
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
    int ret = 1;
    APPLOG("halCBRemoteReq -s");
    // APPLOG("halCBRemoteReq status[%d] cmdId[%u] subCmdId[%u] seqId[%u] randID[%u] \
    //     passThru[%d] uuid[%s]", 
    //     cmdInfo->status, 
    //     cmdInfo->cmdId, 
    //     cmdInfo->subCmdId, 
    //     cmdInfo->seqId, 
    //     cmdInfo->randID, 
    //     cmdInfo->passThru, 
    //     cmdInfo->uuid);
    APPLOG("halCBRemoteReq len[%d][%s]", len, payloadBody);

    switch (cmdInfo->cmdId) {
        case LELINK_CMD_HELLO_REQ: {
            if (LELINK_SUBCMD_HELLO_REQ == cmdInfo->subCmdId) {
                // TODO: SDK got a new device
                ret = 1;
            }
        }break;
        case LELINK_CMD_CLOUD_MSG_CTRL_R2T_REQ: {
            if (LELINK_SUBCMD_CLOUD_MSG_CTRL_R2T_REQ == cmdInfo->subCmdId || 
                LELINK_SUBCMD_CLOUD_MSG_CTRL_R2T_DO_OTA_REQ == cmdInfo->subCmdId) {
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
