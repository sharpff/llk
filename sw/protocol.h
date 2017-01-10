#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

// EXPORT

#ifdef __cplusplus
extern "C"
{
#endif

#include "leconfig.h"
#define MAX_IPLEN 32
#define NW_SELF_PORT 59673


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
    LELINK_ERR_LOGIC_ERR1               = -120,             /*错误的logic1*/
    LELINK_ERR_LOGIC_ERR2               = -121,             /*错误的logic2*/
    
    
    LELINK_ERR_PARAM_INVALID            = -1000,            /*参数数据有误*/
    LELINK_ERR_SYSTEM_ERR               = -1001,            /*系统调用错误,例如创建socket失败*/
    LELINK_ERR_NETWORK_TIMEOUT          = -1002,            /*网络超时*/
    LELINK_ERR_RECV_DATA_ERR            = -1003,             /*接收到的数据有误*/
    LELINK_ERR_CANCEL_ERR               = -1004,            /*用户取消操作*/
    LELINK_ERR_BUSY_ERR                 = -1005,            /*in processing*/
    LELINK_ERR_IA_DELETE                = -1006,            /*IA delete error, it means IA has not been deleted*/
    LELINK_ERR_LOCK_UNLOCK              = -1008,            /*lock OR unlock failed*/
    LELINK_ERR_HTTP_UPDATE              = -1009,            /*halUpdate*/
    LELINK_ERR_VERIFY_SCRIPT            = -1010,            /*script1, script2 verification failed*/
    LELINK_ERR_WRITE_SCRIPT1            = -1011,            /*script1 write error*/
    LELINK_ERR_WRITE_SCRIPT2            = -1012,            /*script2 write error*/
};

typedef enum
{
    LELINK_CMD_ASYNC_OTA_REQ = 1,
    LELINK_CMD_ASYNC_OTA_RSP,
    LELINK_CMD_ASYNC_REBOOT_REQ = 3,
    LELINK_CMD_ASYNC_REBOOT_RSP,
    /*
     * send this cmd only for a case that getting AP connection 
     * by the wlan configure info in the air 
     */
    LELINK_CMD_HELLO_REQ = 7,
    LELINK_CMD_HELLO_RSP,
    /*
     * a broad cast cmd for discovering the shared device(s)
     * the ndIP of NodeData should be "255.255.255.255" normally.
     */
    LELINK_CMD_DISCOVER_REQ = 9,
    LELINK_CMD_DISCOVER_RSP,
    /*
     * a standard control cmd for a valid device in LAN.
     */
    LELINK_CMD_CTRL_REQ = 15,
    LELINK_CMD_CTRL_RSP,
    /*
     * R2R for interacting with cloud.
     */
    LELINK_CMD_CLOUD_GET_TARGET_REQ = 87,
    LELINK_CMD_CLOUD_GET_TARGET_RSP,
    LELINK_CMD_CLOUD_AUTH_REQ = 89,
    LELINK_CMD_CLOUD_AUTH_RSP,
    LELINK_CMD_CLOUD_HEARTBEAT_REQ = 91,
    LELINK_CMD_CLOUD_HEARTBEAT_RSP,
    LELINK_CMD_CLOUD_REPORT_REQ = 93,
    LELINK_CMD_CLOUD_REPORT_RSP,
    /*
     * remote ctrl.
     */
    LELINK_CMD_CLOUD_MSG_CTRL_C2R_REQ = 95,
    LELINK_CMD_CLOUD_MSG_CTRL_C2R_RSP,
    LELINK_CMD_CLOUD_MSG_CTRL_R2T_REQ = 97,
    LELINK_CMD_CLOUD_MSG_CTRL_R2T_RSP,
    /*
     * a request comming from cloud.
     */
    LELINK_CMD_CLOUD_IND_REQ = 99,
    LELINK_CMD_CLOUD_IND_RSP,

    LELINK_CMD_CLOUD_ONLINE_REQ = 101,
    LELINK_CMD_CLOUD_ONLINE_RSP

}E_LELINK_CMD;

typedef enum
{
    /*
     * belong to LELINK_CMD_HELLO_XXX
     */
    LELINK_SUBCMD_HELLO_REQ = 1,
    LELINK_SUBCMD_HELLO_RSP,
    /*
     * belong to LELINK_CMD_DISCOVER_XXX
     */
    LELINK_SUBCMD_DISCOVER_REQ = 1,
    LELINK_SUBCMD_DISCOVER_RSP,
    LELINK_SUBCMD_DISCOVER_STATUS_CHANGED_REQ,
    LELINK_SUBCMD_DISCOVER_STATUS_CHANGED_RSP,
    /*
     * belong to LELINK_CMD_HELLO_XXX
     */
    LELINK_SUBCMD_CTRL_CMD_REQ = 1,
    LELINK_SUBCMD_CTRL_CMD_RSP,
    /*
     * belong to LELINK_CMD_CTRL_XXX
     */
    LELINK_SUBCMD_CTRL_GET_STATUS_REQ,
    LELINK_SUBCMD_CTRL_GET_STATUS_RSP,
    /*
     * belong to LELINK_CMD_CLOUD_GET_TARGET_XXX
     */    
    LELINK_SUBCMD_CLOUD_GET_TARGET_REQ = 1,
    LELINK_SUBCMD_CLOUD_GET_TARGET_RSP,
    /*
     * belong to LELINK_CMD_CLOUD_AUTH_XXX
     */    
    LELINK_SUBCMD_CLOUD_AUTH_REQ = 1,
    LELINK_SUBCMD_CLOUD_AUTH_RSP,
    /*
     * belong to LELINK_CMD_CLOUD_HEARTBEAT_XXX
     */    
    LELINK_SUBCMD_CLOUD_HEARTBEAT_REQ = 1,
    LELINK_SUBCMD_CLOUD_HEARTBEAT_RSP,
    LELINK_SUBCMD_CLOUD_STATUS_CHANGED_REQ = 3,
    LELINK_SUBCMD_CLOUD_STATUS_CHANGED_RSP,
    LELINK_SUBCMD_CLOUD_IA_EXE_NOTIFY_REQ = 5,
    LELINK_SUBCMD_CLOUD_IA_EXE_NOTIFY_RSP,
    /*
     * belong to LELINK_CMD_CLOUD_MSG_CTRL_XXX_XXX
     */    
    LELINK_SUBCMD_CLOUD_MSG_CTRL_C2R_REQ = 1,
    LELINK_SUBCMD_CLOUD_MSG_CTRL_C2R_RSP,
    LELINK_SUBCMD_CLOUD_MSG_CTRL_C2R_DO_OTA_REQ,
    LELINK_SUBCMD_CLOUD_MSG_CTRL_C2R_DO_OTA_RSP,
    LELINK_SUBCMD_CLOUD_MSG_CTRL_C2R_TELL_SHARE_REQ,
    LELINK_SUBCMD_CLOUD_MSG_CTRL_C2R_TELL_SHARE_RSP,
    LELINK_SUBCMD_CLOUD_MSG_CTRL_C2R_CONFIRM_SHARE_REQ,
    LELINK_SUBCMD_CLOUD_MSG_CTRL_C2R_CONFIRM_SHARE_RSP,
    LELINK_SUBCMD_CLOUD_MSG_CTRL_R2T_REQ = 1,
    LELINK_SUBCMD_CLOUD_MSG_CTRL_R2T_RSP,
    LELINK_SUBCMD_CLOUD_MSG_CTRL_R2T_DO_OTA_REQ,
    LELINK_SUBCMD_CLOUD_MSG_CTRL_R2T_DO_OTA_RSP,
    LELINK_SUBCMD_CLOUD_MSG_CTRL_R2T_TELL_SHARE_REQ,
    LELINK_SUBCMD_CLOUD_MSG_CTRL_R2T_TELL_SHARE_RSP,
    LELINK_SUBCMD_CLOUD_MSG_CTRL_R2T_CONFIRM_SHARE_REQ,
    LELINK_SUBCMD_CLOUD_MSG_CTRL_R2T_CONFIRM_SHARE_RSP,
    
    // LELINK_SUBCMD_CLOUD_MSG_RELAY_C2R_REQ = 1,
    // LELINK_SUBCMD_CLOUD_MSG_RELAY_C2R_RSP,
    // LELINK_SUBCMD_CLOUD_MSG_RELAY_R2T_REQ = 1,
    // LELINK_SUBCMD_CLOUD_MSG_RELAY_R2T_RSP,
    // LELINK_SUBCMD_CLOUD_MSG_NOTIFY_R2T_REQ = 1,
    // LELINK_SUBCMD_CLOUD_MSG_NOTIFY_R2T_RSP,
    /*
     * belong to LELINK_CMD_CLOUD_REPORT_XXX
     */    
    LELINK_SUBCMD_CLOUD_REPORT_REQ = 1,
    LELINK_SUBCMD_CLOUD_REPORT_RSP,
    LELINK_SUBCMD_CLOUD_REPORT_OTA_QUERY_REQ,
    LELINK_SUBCMD_CLOUD_REPORT_OTA_QUERY_RSP,
    LELINK_SUBCMD_CLOUD_REPORT_OTA_DO_REQ,
    LELINK_SUBCMD_CLOUD_REPORT_OTA_DO_RSP,
    /*
     * belong to LELINK_CMD_CLOUD_IND_XXX_XXX
     */    
    LELINK_SUBCMD_CLOUD_IND_CTRL_REQ = 1,
    LELINK_SUBCMD_CLOUD_IND_CTRL_RSP,
    LELINK_SUBCMD_CLOUD_IND_OTA_REQ,
    LELINK_SUBCMD_CLOUD_IND_OTA_RSP,
    LELINK_SUBCMD_CLOUD_IND_STATUS_REQ,
    LELINK_SUBCMD_CLOUD_IND_STATUS_RSP,
    LELINK_SUBCMD_CLOUD_IND_MSG_REQ,
    LELINK_SUBCMD_CLOUD_IND_MSG_RSP,

    /*
     * handle the async request
     */
    LELINK_SUBCMD_ASYNC_OTA_REQ = 1,
    LELINK_SUBCMD_ASYNC_OTA_RSP,
    LELINK_SUBCMD_ASYNC_REBOOT_REQ = 1,
    LELINK_SUBCMD_ASYNC_REBOOT_RSP,

    LELINK_SUBCMD_CLOUD_ONLINE_REQ = 1,
    LELINK_SUBCMD_CLOUD_ONLINE_RSP
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

#define CMD_INFO_HEADER \
    CMD_HEADER_INFO_1ST; \
    uint16_t subCmdId; \
    uint16_t reserved3; \
    uint8_t token[AES_LEN]; \
    char ndIP[MAX_IPLEN];


typedef struct {
    CACHE_NODE_HEADER;
    CMD_INFO_HEADER;
}CmdHeaderInfo;

typedef struct {
    CACHE_NODE_HEADER;
    CMD_INFO_HEADER;
    void *pCtx; // belong to which contex
    uint32_t timeStamp;
    uint32_t timeoutRef;
    uint16_t ndPort;
    uint8_t needReq;
    uint8_t needRsp;
}NodeData;

#define COMM_CTX(a) ((CommonCtx *)(a))

/*
 * 'auth' is an obj of AuthData, that could be referred in io.h
 * Just pass NULL if loading from flash.
 */
int lelinkInit();
void lelinkDeinit();

int lelinkDoPollingQ2A(void *ctx);
int lelinkDoPollingR2R(void *ctx);

void *lelinkNwNew(const char *remoteIP, int remotePort, int selfPort, void *ctx);
int lelinkNwPostCmd(void *ctx, const void *cmdInfo);
int lelinkNwDelete(void *ctx);

int lelinkDoConfig(const char *configInfo);
int lelinkVerify(uint32_t startAddr, uint32_t size);
int lelinkVerifyBuf(uint8_t *buf, uint32_t size);


int halCBLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *data, int len);
void halCBRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len);
int halCBRemoteReq(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len);
int halCBLocalRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len, char *nw, int nwLenOut);

#ifdef __cplusplus
}
#endif

#endif
