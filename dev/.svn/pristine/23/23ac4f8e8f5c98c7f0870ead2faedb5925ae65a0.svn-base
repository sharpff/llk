#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "leconfig.h"
#define MAX_UUID 32
#define MAX_IPLEN 32
#define NW_SELF_PORT 59673
// #define NW_SELF_PORT 0


enum
{
    LELINK_ERR_SUCCESS                  = 0,
    LELINK_ERR_BUFFER_SPACE_ERR         = -100,             /*buffer不足*/
    LELINK_ERR_RECV_LEN_ERR             = -101,             /*接收数据长度有误*/
    LELINK_ERR_CHECKSUM_1ST_ERR         = -102,             /*数据校验失败*/
    LELINK_ERR_CHECKSUM_2ND_ERR         = -103,             /*数据校验失败*/
    LELINK_ERR_GET_CMD_ERR              = -104,             /*接收命令类型有误*/
    LELINK_ERR_GET_DEVID_ERR            = -105,             /*设备ID不一致*/
    LELINK_ERR_DEVID_ERR                = -106,             /*设备ID有误*/
    LELINK_ERR_TOKEN_ERR                = -107,             /*设备TOKEN有误*/
    LELINK_ERR_ENC1_ERR                 = -108,             /*不支持的加密策略*/
    LELINK_ERR_ENC2_ERR                 = -109,             /*不支持的加密策略*/
    LELINK_ERR_ENC3_ERR                 = -110,             /*不支持的加密策略*/
    LELINK_ERR_ENC4_ERR                 = -111,             /*不支持的加密策略*/
    LELINK_ERR_DEC1_ERR                 = -112,             /*不支持的解密策略*/
    LELINK_ERR_DEC2_ERR                 = -113,             /*不支持的解密策略*/
    LELINK_ERR_DEC3_ERR                 = -114,             /*不支持的解密策略*/
    LELINK_ERR_DEC4_ERR                 = -115,             /*不支持的解密策略*/
    LELINK_ERR_ENCTYPE_ERR              = -116,             /*不支持的加密类型*/
    LELINK_ERR_MAGIC_ERR                = -117,             /*无效数据包,magic有误*/
    LELINK_ERR_ENCINFO_ERR              = -118,             /*加密信息有误*/
    LELINK_ERR_BAD_TOKEN_ERR            = -119,             /*错误的token*/
    
    
    LELINK_ERR_PARAM_INVALID            = -1000,            /*参数数据有误*/
    LELINK_ERR_SYSTEM_ERR               = -1001,            /*系统调用错误,例如创建socket失败*/
    LELINK_ERR_NETWORK_TIMEOUT          = -1002,            /*网络超时*/
    LELINK_ERR_RECV_DATA_ERR            = -1003,             /*接收到的数据有误*/
    LELINK_ERR_CANCEL_ERR               = -1004,            /*用户取消操作*/
};

typedef enum
{
    LELINK_CMD_HELLO_REQ = 7,
    LELINK_CMD_HELLO_RSP,
    LELINK_CMD_DISCOVER_REQ = 9,
    LELINK_CMD_DISCOVER_RSP,
    // LELINK_CMD_DEVAUTH_REQ = 13,
    // LELINK_CMD_DEVAUTH_RSP,
    LELINK_CMD_CTRL_REQ = 15,
    LELINK_CMD_CTRL_RSP,
    
    // do not modify this item
    LELINK_CMD_CLOUD_GET_TARGET_REQ = 87,
    LELINK_CMD_CLOUD_GET_TARGET_RSP,
    LELINK_CMD_CLOUD_AUTH_REQ = 89,
    LELINK_CMD_CLOUD_AUTH_RSP,
    LELINK_CMD_CLOUD_HEARTBEAT_REQ = 91,
    LELINK_CMD_CLOUD_HEARTBEAT_RSP,
    LELINK_CMD_CLOUD_REPORT_REQ = 93,
    LELINK_CMD_CLOUD_REPORT_RSP,

    LELINK_CMD_CLOUD_MSG_CTRL_C2R_REQ = 95,
    LELINK_CMD_CLOUD_MSG_CTRL_C2R_RSP,
    LELINK_CMD_CLOUD_MSG_CTRL_R2T_REQ = 97,
    LELINK_CMD_CLOUD_MSG_CTRL_R2T_RSP
}E_LELINK_CMD;

typedef enum
{
    LELINK_SUBCMD_HELLO_REQ = 1,
    LELINK_SUBCMD_HELLO_RSP,
    LELINK_SUBCMD_DISCOVER_REQ = 1,
    LELINK_SUBCMD_DISCOVER_RSP,
    LELINK_SUBCMD_DEVNOTICE_REQ = 1,
    LELINK_SUBCMD_DEVNOTICE_RSP,
    LELINK_SUBCMD_DEVAUTH_REQ = 1,
    LELINK_SUBCMD_DEVAUTH_RSP,
    LELINK_SUBCMD_CTRL_CMD_REQ = 1,
    LELINK_SUBCMD_CTRL_CMD_RSP,
    LELINK_SUBCMD_CTRL_GET_STATUS_REQ,
    LELINK_SUBCMD_CTRL_GET_STATUS_RSP,


    
    LELINK_SUBCMD_CLOUD_GET_TARGET_REQ = 1,
    LELINK_SUBCMD_CLOUD_GET_TARGET_RSP,
    LELINK_SUBCMD_CLOUD_AUTH_REQ = 1,
    LELINK_SUBCMD_CLOUD_AUTH_RSP,
    LELINK_SUBCMD_CLOUD_HEARTBEAT_REQ = 1,
    LELINK_SUBCMD_CLOUD_HEARTBEAT_RSP,

    LELINK_SUBCMD_CLOUD_MSG_CTRL_C2R_REQ = 1,
    LELINK_SUBCMD_CLOUD_MSG_CTRL_C2R_RSP,
    LELINK_SUBCMD_CLOUD_MSG_CTRL_R2T_REQ = 1,
    LELINK_SUBCMD_CLOUD_MSG_CTRL_R2T_RSP,
    
    LELINK_SUBCMD_CLOUD_MSG_RELAY_C2R_REQ = 1,
    LELINK_SUBCMD_CLOUD_MSG_RELAY_C2R_RSP,
    LELINK_SUBCMD_CLOUD_MSG_RELAY_R2T_REQ = 1,
    LELINK_SUBCMD_CLOUD_MSG_RELAY_R2T_RSP,
    LELINK_SUBCMD_CLOUD_MSG_NOTIFY_R2T_REQ = 1,
    LELINK_SUBCMD_CLOUD_MSG_NOTIFY_R2T_RSP,
    LELINK_SUBCMD_CLOUD_REPORT_REQ = 1,
    LELINK_SUBCMD_CLOUD_REPORT_RSP
}E_LELINK_SUBCMD;



#define CMD_HEADER_INFO_1ST \
    int16_t status; \
    uint16_t cmdId; \
    uint16_t seqId; \
    uint16_t randID; \
    uint8_t passThru; \
    uint8_t reserved; \
    uint8_t reserved1; \
    uint8_t reserved2; \
    uint8_t uuid[MAX_UUID]

#define CMD_HEADER_INFO \
    CMD_HEADER_INFO_1ST; \
    uint16_t subCmdId; \
    uint16_t reserved3; \
    uint8_t token[AES_LEN]; \
    char ndIP[MAX_IPLEN];


typedef struct {
    CMD_HEADER_INFO;
}CmdHeaderInfo;

typedef struct
{
    CMD_HEADER_INFO;
    void *pCtx; // belong to which contex
    uint32_t timeStamp;
    uint32_t timeoutRef;
    uint16_t ndPort;
    uint8_t needReq;
    uint8_t needRsp;
}NodeData;

#define COMM_CTX(a) ((CommonCtx *)(a))

// #define NW_POST_CMD(ctx, s, sid, cid, scid, pt) \
//     do { \
//         CmdHeaderInfo cmdInfo; \
//         cmdInfo.status = s; \
//         cmdInfo.cmdId = cid; \
//         cmdInfo.seqId = sid; \
//         cmdInfo.passThru = pt; \
//         cmdInfo.subCmdId = scid; \
//         nwPostCmd(ctx, &cmdInfo); \
//     } while (0);


int lelinkInit();
void lelinkDeinit();

int doPollingQ2A(void *ctx);
int doPollingR2R(void *ctx);

void *nwNew(const char *remoteIP, int remotePort, int selfPort, void *ctx);
int nwPostCmd(void *ctx, const void *cmdInfo);
int nwDelete(void *ctx);

#ifdef __cplusplus
}
#endif

#endif