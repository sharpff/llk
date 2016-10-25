#include "halHeader.h"
#include "protocol.h"

int halCBLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *data, int len) {
    int ret = 0;
    APPLOG("halCBLocalReq -s");
    
    switch (cmdInfo->cmdId) 
    {
    case LELINK_CMD_DISCOVER_REQ: 
        {
            break;
        }
    case LELINK_CMD_CTRL_REQ: 
        {
            if (LELINK_SUBCMD_CTRL_CMD_REQ == cmdInfo->subCmdId) {
            }
            break;
        }
    case LELINK_CMD_CLOUD_MSG_CTRL_C2R_REQ: 
        {
            if (LELINK_SUBCMD_CLOUD_MSG_CTRL_C2R_REQ == cmdInfo->subCmdId) {
            }
        }
        break;
    }
    APPLOG("halCBLocalReq [%d] -e", ret);
    return ret;
}

void halCBRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *payloadBody, int len) {
    
    switch (cmdInfo->cmdId) {
    case LELINK_CMD_CLOUD_MSG_CTRL_R2T_REQ:
        {
            if (LELINK_SUBCMD_CLOUD_MSG_CTRL_R2T_REQ == cmdInfo->subCmdId) {
            }
        }
    }
}

int halCBRemoteReq(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *payloadBody, int len) {
    int ret = 1;
    APPLOG("halCBRemoteReq -s");
    
    switch (cmdInfo->cmdId) {
    case LELINK_CMD_HELLO_REQ: 
        {
            if (LELINK_SUBCMD_HELLO_REQ == cmdInfo->subCmdId) {
            }
        }
        break;
    case LELINK_CMD_CLOUD_MSG_CTRL_R2T_REQ: 
        {
            if (LELINK_SUBCMD_CLOUD_MSG_CTRL_R2T_REQ == cmdInfo->subCmdId) {
            }
        }
        break;
    }
    APPLOG("halCBRemoteReq -e");
    
    return ret;
}

int halCBLocalRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len, char *nw, int nwLenOut) {
    int ret = 0;
    
    APPLOG("halCBLocalRsp -s");
    switch (cmdInfo->cmdId) {
    case LELINK_CMD_HELLO_RSP: 
        {
            if (LELINK_SUBCMD_HELLO_RSP == cmdInfo->subCmdId) {
            }
        }
        break;
    case LELINK_CMD_DISCOVER_REQ: 
        {
        }
        break;
    case LELINK_CMD_CTRL_REQ:
        {
        }
        break;
        
    }
    APPLOG("halCBLocalRsp [%d] -e", ret);
    return ret;
}

