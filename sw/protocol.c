#include "leconfig.h"
#include "protocol.h"
#include "network.h"
#include "pack.h"
#include "io.h"
#include "data.h"
#include "ota.h"
#include "state.h"
#include "utility.h"
#include "airconfig_ctrl.h"
#include "sengine.h"
#include "misc.h"
#include "rsaWrapper.h"
#include "ota.h"
#define RETRY_HEARTBEAT 2

// lftp letv:1q2w3e4r@115.182.63.167:21
// 115.182.94.173 <=> 10.204.28.134
// xxx <=> 10.154.252.130
// ./Debug/linux 10000100091000610006C80E77ABCD40 192.168.3.238 \{\"ctrl\":\{\"action\":1\}\} \{\"ctrl\":\{\"action\":2\}\}
// ./Debug/linux 10000100101000010007C80E77ABCD50 192.168.3.104 \{\"ctrl\":\{\"idx1\":1,\"idx2\":1,\"idx3\":1,\"idx4\":1\}\} \{\"ctrl\":\{\"idx1\":0,\"idx2\":0,\"idx3\":0,\"idx4\":0\}\}
// ./Debug/linux 10000100111000810008C80E77ABCD60 192.168.3.120 \{\"ctrl\":\{\"abc\":1\}\} \{\"ctrl\":\{\"abc\":2\}\}
// ./Debug/linux 10000100051000710010C80E77ABCD70 192.168.3.136 \{\"ctrl\":\{\"pwr\":1\}\} \{\"ctrl\":\{\"pwr\":0}\}
// ./Debug/linux 10000100051000710010C80E77ABCD80 192.168.3.152 \{\"ctrl\":\{\"pwr\":1,\"mode\":2,\"temp\":27,\"speed\":1\}\} \{\"ctrl\":\{\"pwr\":1,\"mode\":2\,\"temp\":27,\"speed\":2\}\}
// ./Debug/linux 10000100111000810008C80E77ABCDFF 192.168.3.129 \{\"ctrl\":\{\"abc\":1\}\} \{\"ctrl\":\{\"abc\":2\}\}
// ./Debug/linux 10000100131000910011C80E77ABCD90 192.168.3.110 \{\"ctrl\":\{\"sDevJoin\":1\}\} \{\"ctrl\":\{\"sDevJoin\":1\}\}
// ./Debug/linux 10000100131000010011000C43768722 192.168.3.110 "{\"ctrl\":{\"pwm\":[{\"id\":33,\"val\":1024},{\"id\":34,\"val\":1024},{\"id\":35,\"val\":1024},{\"id\":18,\"val\":1024}]}}" "{\"ctrl\":{\"pwm\":[{\"id\":33,\"val\":1024},{\"id\":34,\"val\":1024},{\"id\":35,\"val\":1024},{\"id\":18,\"val\":1024}]}}"
// ./Debug/linux 10000100131000010011000C43768722 192.168.3.110 "{\"key\":2,\"val\":{\"name\":\"2016120219244600018\"}}" "{\"key\":2,\"val\":{\"name\":\"2016120219244600018\"}}"
// ./Debug/linux 10000100131000010011B01BD2F00023 192.168.3.110 "{\"ctrl\":{\"light\":0,\"mode\":2,\"timeout\":0,\"brightness\":1024,\"red\":1024,\"green\":1024,\"blue\":1024,\"service\":0,\"wifimode\":0}}" "{\"ctrl\":{\"light\":0,\"mode\":0,\"timeout\":0,\"brightness\":1024,\"red\":1024,\"green\":1024,\"blue\":1024,\"service\":0,\"wifimode\":0}}"
// ./Debug/linux 10000100131000010011B01BD2F00023 192.168.3.110 "{\"key\":2,\"val\":{\"name\":\"2017022215151600006\",\"act\":0}}" "{\"key\":2,\"val\":{\"name\":\"2017022215151600006\",\"act\":0}}"
// {
//     uint8_t mac[6] = {0xC8, 0x0E, 0x77, 0xAB, 0xCD, 0x40}; // dooya
//     // uint8_t mac[6] = {0xC8, 0x0E, 0x77, 0xAB, 0xCD, 0x50}; // honyar
//     // uint8_t mac[6] = {0xC8, 0x0E, 0x77, 0xAB, 0xCD, 0x51}; // honyar
//     // uint8_t mac[6] = {0xC8, 0x0E, 0x77, 0xAB, 0xCD, 0x60}; // dingding
//     // uint8_t mac[6] = {0xC8, 0x0E, 0x77, 0xAB, 0xCD, 0x70}; // honyar 86 box
//     // uint8_t mac[6] = {0xC8, 0x0E, 0x77, 0xAB, 0xCD, 0xFF}; // my local
//     wlan_set_mac_addr(mac);
// }


#ifndef LOG_PROTOCOL
#ifdef LELOG
#undef LELOG
#define LELOG(...)
#endif

#ifdef LELOGW
#undef LELOGW
#define LELOGW(...)
#endif

// #ifdef LELOGE
// #undef LELOGE
// #define LELOGE(...)
// #endif

#ifdef LEPRINTF
#undef LEPRINTF
#define LEPRINTF(...)
#endif
#endif


typedef struct
{
    int cmdId;
    int subCmdId;
    void *procR2R;
    void *procQ2A;
}CmdRecord;

typedef struct 
{
    void *what;
    uint8_t *token;
    int lenToken;
}FindToken;

#define DEF_JSON "{\"def\":\"nothing\"}"
#define RETRY_TIMES 2

// init state
static int8_t ginInitOk = 0;
// for state
extern int8_t ginStateCloudLinked;
extern int8_t ginStateCloudAuthed;
// static CmdRecord tblCmdType[];
extern const char *otaGetLatestUrl();
extern void otaSetLatestUrl(const char *url, int lenUrl);
extern const uint8_t *otaGetLatestSig();
extern void otaSetLatestSig(const uint8_t *sig);
extern int otaGetLatestType();
extern void otaSetLatestType(int type);

extern PCACHE sdevCache();
extern SDevNode *sdevArray();
extern int forEachNodeSDevByMacCB(SDevNode *currNode, void *uData);

typedef int (*CBLocalReq)(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *data, int len);
typedef void (*CBRemoteRsp)(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len);

typedef int (*CBRemoteReq)(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len);
typedef int (*CBLocalRsp)(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len, uint8_t *nw, int nwLenOut);


// static int cbLocalReq(void *ctx, uint8_t *dataOut, int dataLen);
// static int cbRemoteRsp(void *ctx, const uint8_t *dataIn, int dataLen);
// static int cbRemoteReq(void *ctx, const uint8_t *dataIn, int dataLen);
// static int cbLocalRsp(void *ctx, const uint8_t *data, int len, uint8_t *dataOut, int dataLen);
static int cbHelloLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen);
static void cbHelloRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen);
static int cbHelloRemoteReq(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen);
static int cbHelloLocalRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len, uint8_t *dataOut, int dataLen);

static int cbDiscoverLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen);
static void cbDiscoverRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen);
static int cbDiscoverRemoteReq(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen);
static int cbDiscoverLocalRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len, uint8_t *dataOut, int dataLen);

static int cbDiscoverStatusChangedLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen);
static void cbDiscoverStatusChangedRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen);
static int cbDiscoverStatusChangedRemoteReq(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen);
static int cbDiscoverStatusChangedLocalRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len, uint8_t *dataOut, int dataLen);

static int cbCtrlCmdLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen);
static void cbCtrlCmdRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen);
static int cbCtrlCmdRemoteReq(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen);
static int cbCtrlCmdLocalRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len, uint8_t *dataOut, int dataLen);

static int cbCtrlGetStatusLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen);
static void cbCtrlGetStatusRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen);
static int cbCtrlGetStatusRemoteReq(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen);
static int cbCtrlGetStatusLocalRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len, uint8_t *dataOut, int dataLen);

static int cbCloudGetTargetLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen);
static void cbCloudGetTargetRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen);

static int cbCloudOnlineLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen);
static void cbCloudOnlineRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen);

static int cbCloudAuthLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen);
static void cbCloudAuthRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen);

static int cbCloudHeartBeatLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen);
static void cbCloudHeartBeatRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen);

static int cbCloudStatusChangedLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen);
static void cbCloudStatusChangedRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen);

static int cbCloudIAExeNotifyLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen);
static void cbCloudIAExeNotifyRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen);

static int cbCloudMsgCtrlC2RLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen);
static void cbCloudMsgCtrlC2RRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen);
// static int cbCloudMsgCtrlC2RRemoteReq(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen); // test only
// static int cbCloudMsgCtrlC2RLocalRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len, uint8_t *dataOut, int dataLen); // test only

// static int cbCloudMsgCtrlR2TLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen); // test only
// static void cbCloudMsgCtrlR2TRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen); // test only
static int cbCloudMsgCtrlR2TRemoteReq(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen);
static int cbCloudMsgCtrlR2TLocalRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len, uint8_t *dataOut, int dataLen);

static int cbCloudReportLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen);
static void cbCloudReportRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen);
static int cbCloudReportOTAQueryLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen);
static void cbCloudReportOTAQueryRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen);
static int cbCloudSDevRecordChangedLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen);
static void cbCloudSDevRecordChangedRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen);
static int cbCloudMsgCtrlC2RDoOTALocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen);
static void cbCloudMsgCtrlC2RDoOTARemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen);
static int cbCloudMsgCtrlR2TDoOTARemoteReq(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen);
static int cbCloudMsgCtrlR2TDoOTALocalRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len, uint8_t *dataOut, int dataLen);
// for share
static int cbCloudMsgCtrlC2RTellShareLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen);
static void cbCloudMsgCtrlC2RTellShareRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen);
static int cbCloudMsgCtrlR2TTellShareRemoteReq(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen);
static int cbCloudMsgCtrlR2TTellShareLocalRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len, uint8_t *dataOut, int dataLen);
static int cbCloudMsgCtrlC2RConfirmShareLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen);
static void cbCloudMsgCtrlC2RConfirmShareRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen);
static int cbCloudMsgCtrlR2TConfirmShareRemoteReq(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen);
static int cbCloudMsgCtrlR2TConfirmShareLocalRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len, uint8_t *dataOut, int dataLen);

static int cbCloudIndOTARemoteReq(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen);
static int cbCloudIndOTALocalRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len, uint8_t *dataOut, int dataLen);
static int cbCloudIndStatusRemoteReq(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len);
static int cbCloudIndStatusLocalRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len, uint8_t *dataOut, int dataLen);
static int cbCloudIndMsgRemoteReq(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len);
static int cbCloudIndMsgLocalRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len, uint8_t *dataOut, int dataLen);

static int cbAsyncOTALocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen);
static void cbAsyncOTARemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen);
static int cbAsyncRebootLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen);
static void cbAsyncRebootRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen);


static int getPackage(void *pCtx, char ipTmp[MAX_IPLEN], uint16_t *port, CmdHeaderInfo *cmdInfo);
static int isNeedDelCB(NodeData *currNode);
static int forEachNodeQ2ARspCB(NodeData *currNode, void *uData);
static int forEachNodeR2RPostSendCB(NodeData *currNode, void *uData);
static int forEachNodeR2RFindTokenByUUID(NodeData *currNode, void *uData);
static int forEachNodeR2RFindTokenIP(NodeData *currNode, void *uData);
static int forEachNodeR2RFindNode(NodeData *currNode, void *uData);
static int isFromRemote(const CommonCtx *ctx, const char *ip, uint16_t len, uint16_t port);
// static int isCommingFromItself(const CommonCtx *ctx, const char *ip, uint16_t port);
static int findTokenByUUID(CommonCtx *ctx, const char uuid[MAX_UUID], uint8_t *token, int lenToken);
static int findTokenByIP(CommonCtx *ctx, const char ip[MAX_IPLEN], uint8_t *token, int lenToken);
extern CmdRecord *getCmdRecord(uint32_t cmdId, uint32_t subCmdId);
static int doQ2ARemoteReq(void *ctx,
    const CmdHeaderInfo* cmdInfo, 
    const uint8_t *protocolBuf,
    int pbLen,
    uint8_t *nwBufOut,
    int nwBufLen,
    int *isRepeat);
static int doQ2AProcessing(CommonCtx *pCtx, 
    int protocolBufLen, 
    const CmdHeaderInfo *cmdInfo, 
    char ip[MAX_IPLEN], 
    uint16_t port);
static uint8_t ginFlagHeartBeat = RETRY_HEARTBEAT;
static void flagHeartBeatReset(void);
static int flagHeartBeatMinus(void);



static CmdRecord tblCmdType[] = {
    { LELINK_CMD_HELLO_REQ, LELINK_SUBCMD_HELLO_REQ, cbHelloLocalReq, cbHelloRemoteReq },
    { LELINK_CMD_HELLO_RSP, LELINK_SUBCMD_HELLO_RSP, cbHelloRemoteRsp, cbHelloLocalRsp },
    { LELINK_CMD_DISCOVER_REQ, LELINK_SUBCMD_DISCOVER_REQ, cbDiscoverLocalReq, cbDiscoverRemoteReq },
    { LELINK_CMD_DISCOVER_RSP, LELINK_SUBCMD_DISCOVER_RSP, cbDiscoverRemoteRsp, cbDiscoverLocalRsp },
    // STATUS CHAGNED req from node (LOCAL)
    { LELINK_CMD_DISCOVER_REQ, LELINK_SUBCMD_DISCOVER_STATUS_CHANGED_REQ, cbDiscoverStatusChangedLocalReq, cbDiscoverStatusChangedRemoteReq },
    { LELINK_CMD_DISCOVER_RSP, LELINK_SUBCMD_DISCOVER_STATUS_CHANGED_RSP, cbDiscoverStatusChangedRemoteRsp, cbDiscoverStatusChangedLocalRsp },
    { LELINK_CMD_CTRL_REQ, LELINK_SUBCMD_CTRL_CMD_REQ, cbCtrlCmdLocalReq, cbCtrlCmdRemoteReq },
    { LELINK_CMD_CTRL_RSP, LELINK_SUBCMD_CTRL_CMD_RSP, cbCtrlCmdRemoteRsp, cbCtrlCmdLocalRsp },
    { LELINK_CMD_CTRL_REQ, LELINK_SUBCMD_CTRL_GET_STATUS_REQ, cbCtrlGetStatusLocalReq, cbCtrlGetStatusRemoteReq },
    { LELINK_CMD_CTRL_RSP, LELINK_SUBCMD_CTRL_GET_STATUS_RSP, cbCtrlGetStatusRemoteRsp, cbCtrlGetStatusLocalRsp },
    { LELINK_CMD_CLOUD_GET_TARGET_REQ, LELINK_SUBCMD_CLOUD_GET_TARGET_REQ, cbCloudGetTargetLocalReq, NULL },
    { LELINK_CMD_CLOUD_GET_TARGET_RSP, LELINK_SUBCMD_CLOUD_GET_TARGET_RSP, cbCloudGetTargetRemoteRsp, NULL },
    { LELINK_CMD_CLOUD_AUTH_REQ, LELINK_SUBCMD_CLOUD_AUTH_REQ, cbCloudAuthLocalReq, NULL },
    { LELINK_CMD_CLOUD_AUTH_RSP, LELINK_SUBCMD_CLOUD_AUTH_RSP, cbCloudAuthRemoteRsp, NULL },

    // ONLINE
    { LELINK_CMD_CLOUD_ONLINE_REQ, LELINK_SUBCMD_CLOUD_ONLINE_REQ, cbCloudOnlineLocalReq, NULL },
    { LELINK_CMD_CLOUD_ONLINE_RSP, LELINK_SUBCMD_CLOUD_ONLINE_RSP, cbCloudOnlineRemoteRsp, NULL },

    { LELINK_CMD_CLOUD_HEARTBEAT_REQ, LELINK_SUBCMD_CLOUD_HEARTBEAT_REQ, cbCloudHeartBeatLocalReq, NULL },
    { LELINK_CMD_CLOUD_HEARTBEAT_RSP, LELINK_SUBCMD_CLOUD_HEARTBEAT_RSP, cbCloudHeartBeatRemoteRsp, NULL },
    // STATUS CHAGNED req from node (REMOTE)
    { LELINK_CMD_CLOUD_HEARTBEAT_REQ, LELINK_SUBCMD_CLOUD_STATUS_CHANGED_REQ, cbCloudStatusChangedLocalReq, NULL },
    { LELINK_CMD_CLOUD_HEARTBEAT_RSP, LELINK_SUBCMD_CLOUD_STATUS_CHANGED_RSP, cbCloudStatusChangedRemoteRsp, NULL },
    // IA execute
    { LELINK_CMD_CLOUD_HEARTBEAT_REQ, LELINK_SUBCMD_CLOUD_IA_EXE_NOTIFY_REQ, cbCloudIAExeNotifyLocalReq, NULL },
    { LELINK_CMD_CLOUD_HEARTBEAT_RSP, LELINK_SUBCMD_CLOUD_IA_EXE_NOTIFY_RSP, cbCloudIAExeNotifyRemoteRsp, NULL },
    // remote ctrl
    { LELINK_CMD_CLOUD_MSG_CTRL_C2R_REQ, LELINK_SUBCMD_CLOUD_MSG_CTRL_C2R_REQ, cbCloudMsgCtrlC2RLocalReq, NULL },
    { LELINK_CMD_CLOUD_MSG_CTRL_C2R_RSP, LELINK_SUBCMD_CLOUD_MSG_CTRL_C2R_RSP, cbCloudMsgCtrlC2RRemoteRsp, NULL },
    { LELINK_CMD_CLOUD_MSG_CTRL_R2T_REQ, LELINK_SUBCMD_CLOUD_MSG_CTRL_R2T_REQ, NULL, cbCloudMsgCtrlR2TRemoteReq },
    { LELINK_CMD_CLOUD_MSG_CTRL_R2T_RSP, LELINK_SUBCMD_CLOUD_MSG_CTRL_R2T_RSP, NULL, cbCloudMsgCtrlR2TLocalRsp },
    // 
    { LELINK_CMD_CLOUD_REPORT_REQ, LELINK_SUBCMD_CLOUD_REPORT_REQ, cbCloudReportLocalReq, NULL },
    { LELINK_CMD_CLOUD_REPORT_RSP, LELINK_SUBCMD_CLOUD_REPORT_RSP, cbCloudReportRemoteRsp, NULL },
    // query ota
    { LELINK_CMD_CLOUD_REPORT_REQ, LELINK_SUBCMD_CLOUD_REPORT_OTA_QUERY_REQ, cbCloudReportOTAQueryLocalReq, NULL },
    { LELINK_CMD_CLOUD_REPORT_RSP, LELINK_SUBCMD_CLOUD_REPORT_OTA_QUERY_RSP, cbCloudReportOTAQueryRemoteRsp, NULL },
    // sub devs record changed
    { LELINK_CMD_CLOUD_REPORT_REQ, LELINK_SUBCMD_CLOUD_REPORT_SDEV_RECORD_REQ, cbCloudSDevRecordChangedLocalReq, NULL },
    { LELINK_CMD_CLOUD_REPORT_RSP, LELINK_SUBCMD_CLOUD_REPORT_SDEV_RECORD_RSP, cbCloudSDevRecordChangedRemoteRsp, NULL },
    // cloud push ota
    { LELINK_CMD_CLOUD_IND_REQ, LELINK_SUBCMD_CLOUD_IND_OTA_REQ, NULL, cbCloudIndOTARemoteReq },
    { LELINK_CMD_CLOUD_IND_RSP, LELINK_SUBCMD_CLOUD_IND_OTA_RSP, NULL, cbCloudIndOTALocalRsp},
    // do ota
    { LELINK_CMD_CLOUD_MSG_CTRL_C2R_REQ, LELINK_SUBCMD_CLOUD_MSG_CTRL_C2R_DO_OTA_REQ, cbCloudMsgCtrlC2RDoOTALocalReq, NULL },
    { LELINK_CMD_CLOUD_MSG_CTRL_C2R_RSP, LELINK_SUBCMD_CLOUD_MSG_CTRL_C2R_DO_OTA_RSP, cbCloudMsgCtrlC2RDoOTARemoteRsp, NULL },
    { LELINK_CMD_CLOUD_MSG_CTRL_R2T_REQ, LELINK_SUBCMD_CLOUD_MSG_CTRL_R2T_DO_OTA_REQ, NULL, cbCloudMsgCtrlR2TDoOTARemoteReq },
    { LELINK_CMD_CLOUD_MSG_CTRL_R2T_RSP, LELINK_SUBCMD_CLOUD_MSG_CTRL_R2T_DO_OTA_RSP, NULL, cbCloudMsgCtrlR2TDoOTALocalRsp },
    // for share
    { LELINK_CMD_CLOUD_MSG_CTRL_C2R_REQ, LELINK_SUBCMD_CLOUD_MSG_CTRL_C2R_TELL_SHARE_REQ, cbCloudMsgCtrlC2RTellShareLocalReq, NULL },
    { LELINK_CMD_CLOUD_MSG_CTRL_C2R_RSP, LELINK_SUBCMD_CLOUD_MSG_CTRL_C2R_TELL_SHARE_RSP, cbCloudMsgCtrlC2RTellShareRemoteRsp, NULL },
    { LELINK_CMD_CLOUD_MSG_CTRL_R2T_REQ, LELINK_SUBCMD_CLOUD_MSG_CTRL_R2T_TELL_SHARE_REQ, NULL, cbCloudMsgCtrlR2TTellShareRemoteReq },
    { LELINK_CMD_CLOUD_MSG_CTRL_R2T_RSP, LELINK_SUBCMD_CLOUD_MSG_CTRL_R2T_TELL_SHARE_RSP, NULL, cbCloudMsgCtrlR2TTellShareLocalRsp },
    { LELINK_CMD_CLOUD_MSG_CTRL_C2R_REQ, LELINK_SUBCMD_CLOUD_MSG_CTRL_C2R_CONFIRM_SHARE_REQ, cbCloudMsgCtrlC2RConfirmShareLocalReq, NULL },
    { LELINK_CMD_CLOUD_MSG_CTRL_C2R_RSP, LELINK_SUBCMD_CLOUD_MSG_CTRL_C2R_CONFIRM_SHARE_RSP, cbCloudMsgCtrlC2RConfirmShareRemoteRsp, NULL },
    { LELINK_CMD_CLOUD_MSG_CTRL_R2T_REQ, LELINK_SUBCMD_CLOUD_MSG_CTRL_R2T_CONFIRM_SHARE_REQ, NULL, cbCloudMsgCtrlR2TConfirmShareRemoteReq },
    { LELINK_CMD_CLOUD_MSG_CTRL_R2T_RSP, LELINK_SUBCMD_CLOUD_MSG_CTRL_R2T_CONFIRM_SHARE_RSP, NULL, cbCloudMsgCtrlR2TConfirmShareLocalRsp },
    // STATUS CHAGNED ind from cloud
    { LELINK_CMD_CLOUD_IND_REQ, LELINK_SUBCMD_CLOUD_IND_STATUS_REQ, NULL, cbCloudIndStatusRemoteReq },
    { LELINK_CMD_CLOUD_IND_RSP, LELINK_SUBCMD_CLOUD_IND_STATUS_RSP, NULL, cbCloudIndStatusLocalRsp},

    { LELINK_CMD_CLOUD_IND_REQ, LELINK_SUBCMD_CLOUD_IND_MSG_REQ, NULL, cbCloudIndMsgRemoteReq },
    { LELINK_CMD_CLOUD_IND_RSP, LELINK_SUBCMD_CLOUD_IND_MSG_RSP, NULL, cbCloudIndMsgLocalRsp},

    { LELINK_CMD_ASYNC_OTA_REQ, LELINK_SUBCMD_ASYNC_OTA_REQ, cbAsyncOTALocalReq, NULL },
    { LELINK_CMD_ASYNC_OTA_RSP, LELINK_SUBCMD_ASYNC_OTA_RSP, cbAsyncOTARemoteRsp, NULL },
    { LELINK_CMD_ASYNC_REBOOT_REQ, LELINK_SUBCMD_ASYNC_REBOOT_REQ, cbAsyncRebootLocalReq, NULL },
    { LELINK_CMD_ASYNC_REBOOT_RSP, LELINK_SUBCMD_ASYNC_REBOOT_RSP, cbAsyncRebootRemoteRsp, NULL },
    { 0, 0, 0, 0 }
};

// #define TEST_RSA
// #define TEST_MD5
// #define TEST_AES
// #define TEST_JSON
// #define TEST_FLASH
// #define TEST_SENGINE

#ifdef TEST_AES

#include "aesWrapper.h"
#define ENC_PIECES_TEST(encBlock, totalLen) \
    (((totalLen) - 1)/encBlock + 1)
#define ENC_SIZE_TEST(encBlock, totalLen) \
    (encBlock*ENC_PIECES_TEST(encBlock, totalLen))
void testAES(void) {

    uint8_t buf[32] = "12341234123412";
    uint32_t encLen = strlen(buf);
    uint32_t decLen = ENC_SIZE_TEST(AES_LEN, strlen(buf) + 1);
    int ret;
    uint8_t iv[AES_LEN] = { 0 };
    uint8_t key[AES_LEN] = { 0 };
    
    LELOG("lelinkAES encrypting encLen[%d]", encLen);
    memcpy(iv, (void *)getPreSharedIV(), AES_LEN);
    memcpy(key, (void *)getPreSharedToken(), AES_LEN);
    ret = aes(iv, 
        key, 
        buf,
        &encLen, /* in-len/out-enc size */
        sizeof(buf),
        1);

    LELOG("lelinkAES encrypted [%d] encLen[%d]", ret, encLen);
    
    memcpy(iv, (void *)getPreSharedIV(), AES_LEN);
    //memcpy(key, getPreSharedToken(), AES_LEN);
    LELOG("lelinkAES decrypting decLen[%d]", decLen);
    ret = aes(iv, 
        key, 
        buf,
        &decLen, /* in-dec size/outs */
        sizeof(buf),
        0);
    buf[decLen] = '\0';
    
    LELOG("lelinkAES decrypted [%d][%s] decLen[%d]", ret, buf, decLen);
}
#endif

#ifdef TEST_RSA
#include "data.h"
#include "rsaWrapper.h"
void testRSA(void) {
    uint8_t raw[RSA_RAW_LEN+1] = {"1234"};
    uint8_t encBuf[RSA_LEN*2];
    int ret;

    // FILE *f;
    // char filename[] = "/home/lf/Documents/dev/crypto/CERT/signature.sig";
    // const uint8_t pubkeyTestVerify[] = PRIATE_KEY_TEST_VERIFY_PEM;
    // char rawTest[] = {"abc"};

    uint8_t signature[RSA_LEN] = {0};
    // const char pubkey[] = "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDCyQMwvQauI1/PbtQ2FwVTZwDcPcDXI1nEUIvqsz+tlmQzwpCWGUOwHbZF3AVw8b1zvF5nW/UU0aF8z2KKCqtm6gB4jSblbJZDUlvMhASiGnCUGg2lHf3MDtiMFOeGy2XmvBLDLZVP3uU8gDLfTfCqW+JWzTqoEBZrEK5IPQbi+wIDAQAB";
    char *beVerifyedDEV1 = "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDCyQMwvQauI1/PbtQ2FwVTZwDcPcDXI1nEUIvqsz+tlmQzwpCWGUOwHbZF3AVw8b1zvF5nW/UU0aF8z2KKCqtm6gB4jSblbJZDUlvMhASiGnCUGg2lHf3MDtiMFOeGy2XmvBLDLZVP3uU8gDLfTfCqW+JWzTqoEBZrEK5IPQbi+wIDAQAB";
    char *beVerifyedDEV2 = "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCRzuA57Yx2vH7z1Rp9Mz0jv8tM4yY3qnPgk0+O0EB9hhmR3f5G6wUBapbbcIV1+1rnMGrK/T/R0vh9I9SIn34O2uF/IwMAI29xUq3B7CSwyBYWOCSKl3AUGv8jVDqAhlTawni/LEU1ocjIkceBCWE1HgGpzCEAtsNyVGZKH+sHWwIDAQAB";
    char *beVerifyedDEV3 = "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQC6M+LMvhAvzInKnWRjaQl7PqrpVgPgbzvZkwd+GwTwgaO+mV9kHFL6Xhor43MgiS3Q416T2JtpnHMdKRM3JTBIKkUFJPKBJO4BYnUBpsVtzuzt3vxOak8rC4GG95vb7r0ghWq6nICr54hGlkh19o/u6uohTBsZi87NZ/75Ay32kwIDAQAB";
    char *beVerifiedDEV4 = "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCWgzz4zfQCCUCEDAtBMX0TxFBwTAO51LBMoLcC86Y1SLATa+neotTsJn/kjm8i9RIMKJL52gEGzvSPJ6YNcWM0a6jDTqeOT3HFigbWHVTa09q3f64vlGbAJ5wFDJ3Mf6q2PNztJ05mtsBNC6PcvMAIIQ8YRN1bEbcCb5CyCGWVIwIDAQAB";
    char *beVerifyedSDK1 = "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCCJHkF6iOBX8ObCiS1tmyM6tgrU2QqR0ByyfMXjvp1x2nKiSNABXtjLAiEUPHBMypThBUSADlw6zM7XOANHmaWSoDKWOjDZKeK82uZDh/4C2k806zLb8wiTrh+e+qeKaBiNnY8PMMoBCYrNvO/gnM8aU4GIyhQnGfTkgcpSm2aqwIDAQAB";
    char *beVerifyedSDK2 = "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCf2eu1g68UFbocZLROH90/3rGWpnJkOkRWSO4C3QUjMJ00b4nJDqbTwwkr9w1sLIJd5VsQ0UHwl80+62E6PcUV1ST9KgLfPyvqNbhN3NmpqPOS5wCZGsFp8zGkS9NYdtc3KmClF2K5OlSTaxg7EgdYwytRa1IxZdRNc1MJiKgEtwIDAQAB";
    char *beVerifyedSDK3 = "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDavUkoWA4MmUGXiAaQz+1qalFyFdBTZYkwToTYlmreN43sbH1DZU0TMSqsdbDUWeyLRapjtImCpGiVLgkA4ruad0WFTcCJ4EqdvZ+JhxXv5gyaeB/qd3C95G9PuGPiO2gCUZtVN6+l3wHIh6dyyomEiQwwwozyS2ar6LpU20s0MQIDAQAB";
    char *beVerified = beVerifiedDEV4;
    uint8_t pubkey[MAX_RSA_PUBKEY] = {0};
    int pubkeyLen = 0;
    // ret = rsaEncrypt((const uint8_t *)ginStrPubkeyPem, ginStrPubkeyPemLen, (const uint8_t *)raw, sizeof(raw), encBuf, sizeof(encBuf));
    // LELOG("rsaEncrypted pem len[%d]", ret);

    // ret = rsaDecrypt((const uint8_t *)ginStrPrikeyPem, ginStrPrikeyPemLen, (const uint8_t *)encBuf, ret, raw, sizeof(raw));
    // LELOG("rsaDecrypted pem [%d] => [%s]", ret, raw);
    // LELOG("raw is [%s]", raw);

    // // LELOG("pem prikey[%d], pubkey[%d]", ginStrPrikeyPemLen, ginStrPubkeyPemLen);

    // ret = rsaEncrypt((const uint8_t *)ginPubkeyGlobalDer, ginPubkeyGlobalDerLen, (const uint8_t *)raw, sizeof(raw), encBuf, sizeof(encBuf));
    // LELOG("rsaEncrypted global len[%d]", ret);

    // ret = rsaDecrypt((const uint8_t *)ginPrikeyGlobalDer, ginPrikeyGlobalDerLen, (const uint8_t *)encBuf, ret, raw, sizeof(raw));
    // LELOG("rsaDecrypted global [%d] => [%s]", ret, raw);
    // LELOG("raw is [%s]", raw);

    ret = rsaEncrypt((const uint8_t *)ginPubkeyDevKindDer, ginPubkeyDevKindDerLen, (const uint8_t *)raw, sizeof(raw), encBuf, sizeof(encBuf));
    LELOG("rsaEncrypted devkind len[%d]", ret);
    
    memset(raw, 0, sizeof(raw));
    ret = rsaDecrypt((const uint8_t *)ginPrikeyDevKindDer, ginPrikeyDevKindDerLen, (const uint8_t *)encBuf, ret, raw, sizeof(raw));
    LELOG("rsaDecrypted devkind [%d] => [%s]", ret, raw);
    LELOG("raw is [%s]", raw);
    LELOG("der prikey[%d], pubkey[%d]", ginPrikeyGlobalDerLen, ginPubkeyGlobalDerLen);

    // verify 1
    // if( ( f = fopen( filename, "rb" ) ) == NULL )
    // {
    //     LELOG("fopen failed");
    //     return;
    // }
    // ret = fread( signature, 1, sizeof(signature), f );
    // fclose( f );
    // // ret = getTerminalSignature(signature, sizeof(signature));
    // LELOG("signatrue [%d]", ret);
    // ret = rsaVerify((const uint8_t *)pubkeyTestVerify, sizeof(pubkeyTestVerify), 
    //     rawTest, strlen(rawTest), signature, ret);
    // LELOG("rsaVerify [%d]", ret);

    // verify 2
    pubkeyLen = getTerminalPublicKey(pubkey, sizeof(pubkey));
    ret = getTerminalSignature(signature, sizeof(signature));
    LELOG("signatrue [%d]", ret);
    ret = rsaVerify(pubkey, pubkeyLen, beVerified, strlen(beVerified), signature, ret);
    LELOG("rsaVerify [%d]", ret);

    // otaSetLatestSig()
    // lelinkVerify(0x11111111, 428804);
}
#endif

#ifdef TEST_JSON
void testJson(void) {

    char utc[64] = "{\"utc\":1234}";
    char ip[MAX_IPLEN] = {0};
    int port = 0;
    char json[] = "{\"dir\":0,\"IP\":\"1.2.3.4\",\"port\":8888}";
    // char json1[] = "{\"dir\":1,\"IP\":\"1.2.3.4\",\"port\":8888}";
    char status[256] = {"{}"};
    char out[512] = {"{}"};
    // char ip[MAX_IPLEN] = {0};
    // uint16_t port = 0;
    char token[2 + 2*AES_LEN + 1] = {0};
    int ret = isNeedToRedirect(json, strlen(json), ip, &port);
    LELOG("isNeedToRedirect[%d] [%s:%d]", ret, ip, port);

    ret = syncUTC(utc, strlen(utc));
    LELOG("syncUTC[%d]", ret);

    token[0] = '"';
    getTerminalTokenStr(token + 1, sizeof(token) - 1);
    token[sizeof(token) - 2] = '"';
    getTerminalStatus(status, 256);

    genCompositeJson(out, sizeof(out), 2, 
        "now", status, 
        "token", token);
    LELOG("%s", out);

    // strcpy(status, "{\"status\":{\"action\":1},\"utc\":1457403654}");
    // genS2Json(status, sizeof(status), out, sizeof(out));
    // LELOG("%s", out);

    {
        char string1[64] = {"{\"key\":1,\"val\":{\"lock\":1}}"};
        char string2[64] = {"{\"key\":2, \"val\":{\"name\":\"testIA\", \"act\":1}, \"info\":{}}"};
        int ret = cloudMsgHandler(string1, strlen(string1));
        LELOG("cloudMsgHandler [%d] string1", ret);
        ret = cloudMsgHandler(string2, strlen(string2));
        LELOG("cloudMsgHandler [%d] string2", ret);
    }

    {
        // #include "misc.h"
        extern int testJsonArray(const char *json, int jsonLen);
        char json[] = "{\"sDevGetList\":[0,1,2]}";
        char json1[] = "{\"sDevGetInfo\":2,\"sDev\":{\"pid\":\"0104\",\"clu\":\"0006\",\"ept\":[[\"0000\",1],[\"0000\",2],[\"0000\",3]],\"mac\":\"7409E17E3376AF60\"}}";
        // testJsonArray(json, strlen(json))
;        testJsonArray(json1, strlen(json1));

    }

}
#endif

#ifdef TEST_MD5
#include "md5Wrapper.h"
void testMD5() {
    int i = 0;
    uint8_t uuid[MAX_UUID + 1] = {"10000100011000510005123456abcdef"};
    uint8_t out[MD5_LEN] = {0};
    md5(uuid, strlen(uuid), out);
    
    LELOG("md5: ");
    for (i = 0; i < MD5_LEN; i++) {
        LEPRINTF("%02X", out[i]);
    }
    LEPRINTF("\r\n");
    
}
#endif

#ifdef TEST_FLASH
void testFlash() {

    if (0)
    {
        static char buf[512] = {0};
        mdev_t *fl_dev = NULL;
        int ret = 0, count = 2;
        // uint32_t addr = 0x6000, pageSize = 0x1000;
        uint32_t addr = 0x1C2000, pageSize = 256 - 1;
        LELOG("open [0x%x]", fl_dev);
        while (count--) {
            fl_dev = (mdev_t *)flash_drv_open(0);
            ret = flash_drv_erase(fl_dev, addr, pageSize);
            LELOG("erase [0x%x][%d]", fl_dev, ret);
            sprintf(buf, "making %d", count);
            ret = flash_drv_write(fl_dev, buf + pageSize, strlen(buf), addr);
            LELOG("write [%d][%s]", ret, buf);

            ret = flash_drv_read(fl_dev, buf + pageSize, strlen(buf)+1, addr);
            LELOG("read [%d][%s]", ret, buf);
            flash_drv_close(fl_dev);

            halDelayms(2000);
        }
    }
    else 
    {
        PrivateCfg cfg;
        memset(&cfg, 0, sizeof(PrivateCfg));
        LELOG("sizeof(PrivateCfg) is [%d]", sizeof(PrivateCfg));

        lelinkStorageReadPrivateCfg(&cfg);
        LELOG("read 1 ssid[%s], psk[%s], configStatus[%d]", 
            cfg.data.nwCfg.config.ssid,
            cfg.data.nwCfg.config.psk, 
            cfg.data.nwCfg.configStatus);


        // cfg.data.nwCfg.configStatus = 2;
        cfg.data.nwCfg.configStatus = -1;
        strcpy(cfg.data.nwCfg.config.ssid, "account1");
        strcpy(cfg.data.nwCfg.config.psk, "psk1");
        lelinkStorageWritePrivateCfg(&cfg);

        lelinkStorageReadPrivateCfg(&cfg);
        LELOG("read 2 ssid[%s], psk[%s], configStatus[%d]", 
            cfg.data.nwCfg.config.ssid,
            cfg.data.nwCfg.config.psk, 
            cfg.data.nwCfg.configStatus);
    }
}
#endif

#ifdef TEST_SENGINE
void testSengine() {
    #include "sengine.h"
    int ret = 0, i = 0;
    const char json[256] = {"{\"ctrl\":{\"pwr\":1,\"action\":2}}"};
    // const char json2[256] = {"{\"status\":{\"pwr\":1,\"action\":1},\"utcH\":339,\"utcL\":136006835,\"uuid\":\"10000100011000510005FFFFFFFFFFFF\"}"};
    const char json2[256] = {"{\"status\":{\"pwr\":1,\"action\":1},\"uuid\":\"10000100011000510005FFFFFFFFFFFF\"}"};
    uint8_t buf[256] = {0};
    ScriptCfg2 *tmpScriptCfg2 = NULL;
    ret = sengineInit();
    if (0 != ret) {
        LELOGE("sengineInit ret[%d]", ret);
        return;
    }
    // ret = lelinkStorageReadScriptCfg(&ginScriptCfg);
    // if (ginScriptCfg.csum != crc8((uint8_t *)&(ginScriptCfg.data), sizeof(ginScriptCfg.data))) {
    //     LELOG("ScriptCfg read from flash failed");
    //     // return;
    // }

    ret = sengineCall((const char *)ginScriptCfg->data.script, ginScriptCfg->data.size, 
        "s1CvtStd2Pri", (const uint8_t *)json, strlen(json), buf, sizeof(buf));
    LELOG("testSengine sengineCall => cvtStd2Pri [%d]", ret);
    if (0 < ret) {
        LEPRINTF("result bin: ");
        for (i = 0; i < ret; i++) {
            LEPRINTF("%02X", buf[i]);
        }
    }
    LEPRINTF("\r\n");

    buf[0] = 0x12;
    buf[1] = 0x32;
    buf[2] = 0xab;
    buf[3] = 0xef;
    ret = sengineCall((const char *)ginScriptCfg->data.script, ginScriptCfg->data.size, 
        "s1CvtPri2Std", buf, 4, (uint8_t *)json, sizeof(json));
    LELOG("testSengine cvtPri2Std[%d] [%s]", ret, json);

    tmpScriptCfg2 = (void *)halCalloc(1, sizeof(ScriptCfg2));
    ret = lelinkStorageReadScriptCfg(tmpScriptCfg2, E_FLASH_TYPE_SCRIPT2, 0);
    ret = lelinkStorageWriteScriptCfg2(tmpScriptCfg2);
    // ret = lelinkStorageWriteScriptCfg2(tmpScriptCfg, E_FLASH_TYPE_SCRIPT2, 0);

    senginePollingRules((char *)json2, strlen(json2));
    halFree(tmpScriptCfg);
}
#endif

static void lelinkInfo(void) {
    char fwVer[64] = {0};
    uint8_t mac[8] = {0};
    LEPRINTF("---------lelinkInfo----------\r\n");
    getVer(fwVer, sizeof(fwVer));
    LELOG("ver: %s", fwVer);
    halGetMac(mac, sizeof(mac));
    LELOG("mac: %02x:%02x:%02x:%02x:%02x:%02x", 
        mac[0], 
        mac[1], 
        mac[2], 
        mac[3], 
        mac[4], 
        mac[5]);
    LEPRINTF("-----------------------------\r\n");
}

int lelinkInit() {
    int ret = 0;
    AuthCfg authCfg;
    // void **ioHdl = NULL;
    IOHDL *ioHdl = NULL;
    uint8_t mac[6] = {0};


    ret = halLockInit();
    if (0 != ret) {
        LELOGE("halLockInit ret[%d]", ret);
        goto failed;
    }

    ret = halAESInit();
    if (0 != ret) {
        LELOGE("halAESInit ret[%d]", ret);
        goto failed;
    }

    ret = halFlashInit();
    if (0 != ret) {
        LELOGE("halFlashInit ret[%d]", ret);
        goto failed;
    }

    ret = sengineInit();
    if (0 != ret) {
        LELOGE("sengineInit ret[%d]", ret);
        // goto failed;
    }

    lelinkInfo();

    // ioHdl = (void **)ioGetHdl(NULL);
    ioHdl = ioGetHdlExt();
    // test only
    // {
    //     #include "sengine.h"        
    //     ret = lelinkStorageReadScriptCfg(ginScriptCfg2, E_FLASH_TYPE_SCRIPT2, 0);
    //     ret = lelinkStorageWriteScriptCfg2(ginScriptCfg2);
    // }

    if (NULL == ioHdl) {
        LELOGE("ioInit ioGetHdlExt[%p]", ioHdl);
        // goto failed;
    }
    ret = lelinkStorageReadAuthCfg(&authCfg);
    if (0 > ret) {
        goto failed;
    }
    if (authCfg.csum != crc8((uint8_t *)&(authCfg.data), sizeof(authCfg.data))) {
        ret = -100;
        goto failed;
    }
    setTerminalPublicKey(authCfg.data.pubkey, authCfg.data.pubkeyLen);
    setTerminalSignature(authCfg.data.signature, authCfg.data.signatureLen);        
    setOriRemoteServer(authCfg.data.remote, strlen(authCfg.data.remote), authCfg.data.port);
    getOriRemoteServer(authCfg.data.remote, MAX_REMOTE, &(authCfg.data.port));
    if (0 == halGetMac(mac, sizeof(mac))) {
        char macStr[13] = {0};
        bytes2hexStr(mac, sizeof(mac), (uint8_t*)macStr, sizeof(macStr));
        memcpy(authCfg.data.uuid + 20, macStr, 12);
    }
    setTerminalUUID(authCfg.data.uuid, MAX_UUID);
#ifdef TEST_MD5
    testMD5();
#endif
#ifdef TEST_JSON
    testJson();
#endif
#ifdef TEST_AES
    testAES();
#endif
#ifdef TEST_RSA
    testRSA();
#endif
#ifdef TEST_FLASH
    testFlash();
#endif

#ifdef TEST_SENGINE
    testSengine();
#endif

    ginInitOk = 1;
failed:
    return ret;
}

void lelinkDeinit() {
    halDeLockInit();
    halDeAESInit();
    halFlashDeinit();
    // ioDeInit(IO_TYPE_UART);
}

int lelinkDoPollingQ2A(void *ctx) {
    CommonCtx *pCtx = COMM_CTX(ctx);
    char ipTmp[MAX_IPLEN] = {0};
    uint16_t portTmp = 0;
    int ret = 0;
    //int nwLen = 0;
    // int protocolBufLen = UDP_MTU;
    CmdHeaderInfo cmdInfo = {0};

    // do response if postphone
    qForEachfromCache(&(pCtx->cacheCmd), (int(*)(void*, void*))forEachNodeQ2ARspCB, NULL);

	qCheckForClean(&(pCtx->cacheCmd), (int(*)(void*))isNeedDelCB);

    memset(ipTmp, 0, sizeof(ipTmp));
    memset(pCtx->nwBuf, 0, sizeof(pCtx->nwBuf));
    memset(pCtx->protocolBuf, 0, sizeof(pCtx->protocolBuf));

    // TODO:  temp resolution 
    if (0 == pCtx->selfPort) {
        // R2R is in processing. R2R is more higher priority than QA. QA have to wait for R2R done
        int ret = qEmptyCache(&(pCtx->cacheCmd));
        if (!ret)
            return -1;
    }

    ret = getPackage(pCtx, ipTmp, &portTmp, &cmdInfo);
    if (0 > ret) {
        return -2;
    }

    LELOG("lelinkDoPollingQ2A doUnpack [%d-%d]", cmdInfo.cmdId, cmdInfo.subCmdId);

    ret = doQ2AProcessing(pCtx, ret, &cmdInfo, ipTmp, portTmp);


    return ret;
}

int lelinkDoPollingR2R(void *ctx) {

    CommonCtx *pCtx = COMM_CTX(ctx);
    int len = 0, isCacheEmpty = 0, isRemoteRsp = 0;
    //NodeData *node = NULL;
    //CmdRecord *ct_p = NULL;
    char ipTmp[MAX_IPLEN] = { 0 };
    uint16_t portTmp = 0;
    //CommonHeader *commonHeader = NULL;
    CmdHeaderInfo cmdInfo = {0};

    len = nwUDPRecvfrom(pCtx, pCtx->nwBuf, UDP_MTU, ipTmp, MAX_IPLEN, &portTmp);
    if ((0 < len)) {
        /* RemoteRsp or RemoteReq */
        if (isFromRemote(pCtx, ipTmp, MAX_IPLEN, portTmp)) {
            len = doUnpack(pCtx, pCtx->nwBuf, len, pCtx->protocolBuf, UDP_MTU, &cmdInfo, (void *)findTokenByUUID);
            if (0 <= len) {
                COMM_CTX(pCtx)->protocolBuf[len] = '\0';
                if (cmdInfo.cmdId % 2) { // RemoteReq
                    doQ2AProcessing(pCtx, len, &cmdInfo, ipTmp, portTmp);
                } else { // RemoteRsp
                    // start
                    // goto HandleRemoteRsp;
                    isRemoteRsp = 1;
                }
            }
        }
        else { /* local & RemoteRsp */
            memcpy(cmdInfo.ndIP, ipTmp, MAX_IPLEN);
            len = doUnpack(pCtx, pCtx->nwBuf, len, pCtx->protocolBuf, UDP_MTU, &cmdInfo, (void *)findTokenByIP);
            // start
            // goto HandleRemoteRsp;
            if (0 <= len) {
                isRemoteRsp = 1;
            }
        }

        if (isRemoteRsp) {
            CmdRecord *ct_p = getCmdRecord(cmdInfo.cmdId, cmdInfo.subCmdId);
            if (ct_p && !isCacheEmpty) {
                if (0 <= qForEachfromCache(&(pCtx->cacheCmd), (int(*)(void*, void*))forEachNodeR2RFindNode, (void *)&cmdInfo)) {
                    if(ct_p->procR2R) {
                      ((CBRemoteRsp) ct_p->procR2R)(pCtx, &cmdInfo, COMM_CTX(pCtx)->protocolBuf, len);
                    }
                }
            }
        }
    }

    /*
     * 2. POST sending
     */
     isCacheEmpty = qEmptyCache(&(pCtx->cacheCmd));
     if (!isCacheEmpty) {
        // handle post send
        qForEachfromCache(&(pCtx->cacheCmd), (int(*)(void*, void*))forEachNodeR2RPostSendCB, NULL);

        // TODO: do Q2A's response if postphone, BUT it should got R2R's request. so remote RemoteReq should no postphone
        // qForEachfromCache(&(pCtx->cacheCmd), (int(*)(void*, void*))forEachNodeQ2ARspCB, NULL);

        qCheckForClean(&(pCtx->cacheCmd), (int(*)(void*))isNeedDelCB);
    }

    
    return len;
}

static int doQ2ARemoteReq(void *ctx,
    const CmdHeaderInfo* cmdInfo, 
    const uint8_t *protocolBuf,
    int pbLen,
    uint8_t *nwBufOut,
    int nwBufLen,
    int *isRepeat) {

    int ret = -1, repeat = 0;
    // CommonCtx *cmdInfo = COMM_CTX(ctx);
    CmdRecord *remoteReqPtr, *localRspPtr = NULL;
//    UINT32 tmpType = 0;
    remoteReqPtr = getCmdRecord(cmdInfo->cmdId, cmdInfo->subCmdId);
    if (NULL == remoteReqPtr)
    {
        return ret;
    }

    // multi response. like the response(S) after a probe OR a online notice(S)
    if (0 == (cmdInfo->cmdId & 0x1))
    {
        // no need to response
        // LELOGE("R2R??? it should not be here");
        // remoteReqPtr->procR2R ?
        //         ((CBRemoteRsp) remoteReqPtr->procR2R)(ctx, cmdInfo, protocolBuf, pbLen) :
        //         0;
        return -0xA;
    }
    
    // req from outside
    // note: the data will be sent if CBRemoteReq return true. so to prepare the data in CBLocalRsp
    if ((CBRemoteReq) remoteReqPtr->procQ2A) {
        localRspPtr = getCmdRecord(cmdInfo->cmdId + 1, cmdInfo->subCmdId + 1);
        repeat = ((CBRemoteReq) remoteReqPtr->procQ2A)(ctx, cmdInfo, protocolBuf, pbLen);
        if (!localRspPtr || 0 > repeat || cmdInfo->noAck) {
            return -0xB;
        }
        ((CmdHeaderInfo *)cmdInfo)->cmdId++;
        ((CmdHeaderInfo *)cmdInfo)->subCmdId++;
        ret = ((CBLocalRsp) localRspPtr->procQ2A)(ctx, cmdInfo, protocolBuf, pbLen, nwBufOut, nwBufLen); // do sth local rsp
        if (0 >= ret) {
            LELOGE("CBLocalRsp ret 0, it is impossible [%d]", ret);
            return -0xC;
        }
        if (repeat > 1) {
            ((CmdHeaderInfo *)cmdInfo)->cmdId--;
            ((CmdHeaderInfo *)cmdInfo)->subCmdId--;
            *isRepeat = repeat - 1;
        } else {
            *isRepeat = 0;
        }


    }
    return ret;
}


int lelinkNwPostCmd(void *ctx, const void *node)
{
    CommonCtx *pCtx = (CommonCtx *)ctx;
    NodeData *node_p = (NodeData *)node;
    if (!pCtx || !node_p)
    {
        return 0;
    }

    
    node_p->pCtx = pCtx;
    node_p->needReq = 1;
    node_p->rspVal = !node_p->rspVal ? ((LELINK_CMD_DISCOVER_REQ == node_p->cmdId) || (LELINK_SUBCMD_DISCOVER_STATUS_CHANGED_REQ == node_p->subCmdId && LELINK_CMD_CTRL_REQ == node_p->cmdId) ? 0xFF : 1) : 0;
    node_p->randID = genRand();
    node_p->seqId = node_p->seqId ? node_p->seqId : genSeqId();
    if (!node_p->uuid[0])
        getTerminalUUID(node_p->uuid, MAX_UUID);
    node_p->timeStamp = halGetTimeStamp();
    if (4 > node_p->timeoutRef) {
        node_p->timeoutRef = 4;
    }
    // retry
    if (LELINK_CMD_DISCOVER_REQ == node_p->cmdId && 
        LELINK_SUBCMD_DISCOVER_STATUS_CHANGED_REQ == node_p->subCmdId) {
        node_p->timeoutRef = 1;
    }
    if (LELINK_CMD_CLOUD_MSG_CTRL_C2R_REQ == node_p->cmdId && LELINK_SUBCMD_CLOUD_MSG_CTRL_C2R_REQ == node_p->subCmdId) {
        node_p->timeoutRef = 3;
    }
    LELOG("nwPostCmd cmdId[%d], subCmdId[%d], [%s:%d] timeoutRef[%d] seqId[%d] reserved[%d]", node_p->cmdId, node_p->subCmdId, node_p->ndIP, node_p->ndPort, node_p->timeoutRef, node_p->seqId, node_p->reserved);

    // if (node_p->ndPort) {
    //     memcpy(node.ndIP, ipTmp, MAX_IPLEN);
    //     node.ndPort = portTmp;
    // }

    // // CmdHeaderInfo
    // node.status = cmdInfo.status;
    // node.cmdId = cmdInfo.cmdId;
    // node.subCmdId = cmdInfo.subCmdId;
    // node.seqId = cmdInfo.seqId;
    // node.randID = genRand();
    // node.noAck = cmdInfo.noAck;
    // node.reserved = cmdInfo.reserved;
    // node.reserved1 = cmdInfo.reserved1;
    // node.reserved2 = cmdInfo.reserved2;
    // memcpy(node.uuid, cmdInfo.uuid, sizeof(node.uuid));

    // // rest part
    // node.pCtx = pCtx;
    // node.timeStamp = halGetTimeStamp();
    // node.timeoutRef = 5;
    // node.needReq = 1;
    // node.rspVal = 0;
    // memcpy(node.ndIP, ipTmp, MAX_IPLEN);
    // node.ndPort = portTmp;


    
    if (qEnCache(&(pCtx->cacheCmd), (void *)node))
    {
        return 1;
    }

    return 0;
}

static int isFromRemote(const CommonCtx *ctx, const char *ip, uint16_t len, uint16_t port) {
    return 0 == strcmp(ip, ctx->remoteIP) ? 1 : 0;
}

// static int isCommingFromItself(const CommonCtx *ctx, const char *ip, uint16_t port) {
//     // TODO: IMPL
//     // test only
//     return 0;
//     int tmpPort = 0;
//     int ret = 0;
//     char myIP[MAX_IPLEN] = {0};
//     ret = halGetSelfAddr(myIP, MAX_IPLEN, &tmpPort);
//     if (ret > 0) {
//         if (0 == strncmp(ip, myIP, strlen(ip))) {
//             LELOG("isCommingFromItself YES");
//             return 1;
//         }
//     }
//     return 0;
// }

static int forEachNodeR2RFindTokenByUUID(NodeData *currNode, void *uData) {
    FindToken *ft = (FindToken *)uData;
    if (0 == memcmp(currNode->uuid, ft->what, MAX_UUID)) {
        memcpy(ft->token, currNode->token, ft->lenToken);
        return 1;
    }
    return 0;
}

static int findTokenByUUID(CommonCtx *ctx, const char uuid[MAX_UUID], uint8_t *token, int lenToken) {

    int ret = 0;
    int i = 0;
    FindToken ft;
    ft.what = (void *)uuid;
    ft.token = token;
    ft.lenToken = lenToken;

    ret = qForEachfromCache(&(ctx->cacheCmd), (int(*)(void*, void*))forEachNodeR2RFindTokenByUUID, (void *)&ft);
    if (0 <= ret) {
        LELOG("BY UUID TOKEN GOT => ");
        for (i = 0; i < lenToken; i++) {
            LEPRINTF("%02X", token[i]);
        }
        LEPRINTF("\r\n");
        return 1;
    }
    return 0;
}

static int forEachNodeR2RFindTokenIP(NodeData *currNode, void *uData) {
    FindToken *ft = (FindToken *)uData;
    // to void the invalid token
    if (currNode->token[0] && 
        0 == memcmp(currNode->ndIP, ft->what, MAX_IPLEN)) {
        memcpy(ft->token, currNode->token, ft->lenToken);
        return 1;
    }
    return 0;
}
static int findTokenByIP(CommonCtx *ctx, const char ip[MAX_IPLEN], uint8_t *token, int lenToken) {

    int ret = 0;
    int i = 0;
    FindToken ft;
    ft.what = (void *)ip;
    ft.token = token;
    ft.lenToken = lenToken;
    ret = qForEachfromCache(&(ctx->cacheCmd), (int(*)(void*, void*))forEachNodeR2RFindTokenIP, (void *)&ft);
    if (0 <= ret) {
        LELOG("BY IP TOKEN GOT => ");
        for (i = 0; i < lenToken; i++) {
            LEPRINTF("%02X", token[i]);
        }
        LEPRINTF("\r\n");
        return 1;
    }
    return 0;
}

static int forEachNodeR2RFindNode(NodeData *currNode, void *uData) {
    CmdHeaderInfo *cmdInfo = (CmdHeaderInfo *)uData;
    if (currNode->seqId == cmdInfo->seqId) {
        if (0 < currNode->rspVal) {
            currNode->rspVal--;
            return 1;
        }
    }
    return 0;
}

static int getPackage(void *pCtx, char ipTmp[MAX_IPLEN], uint16_t *port, CmdHeaderInfo *cmdInfo) {

    int ret;
    // memset(COMM_CTX(pCtx)->nwBuf, 0, sizeof(COMM_CTX(pCtx)->nwBuf));
    // memset(COMM_CTX(pCtx)->protocolBuf, 0, sizeof(COMM_CTX(pCtx)->protocolBuf));
    ret = nwUDPRecvfrom(pCtx, COMM_CTX(pCtx)->nwBuf, UDP_MTU, ipTmp, MAX_IPLEN, port);
    if (0 >= ret) {
        return -4;
    }

    if (0 > (ret = doUnpack(pCtx, COMM_CTX(pCtx)->nwBuf, ret, COMM_CTX(pCtx)->protocolBuf,
        sizeof(COMM_CTX(pCtx)->protocolBuf), cmdInfo, NULL)))
    {
        LELOGW("lelinkDoPolling doUnpack [%d]", ret);
        return -5;
    }
    COMM_CTX(pCtx)->protocolBuf[ret] = '\0';
    return ret;
}

static int forEachNodeR2RPostSendCB(NodeData *currNode, void *uData)
{
    int len;
    CmdRecord *ct_p;
    //int protocolBufLen = UDP_MTU;
    //uint16_t seqId = 0;
    //int ret = 0;
    char ipTmp[MAX_IPLEN] = { 0 };
    int portTmp = 0;
    // only for RemoteRsp
    //int protocolBufLen = (int)uData;

    USED(uData);
    // do req from local to remote
    if (currNode->needReq)
    {
        ct_p = getCmdRecord(currNode->cmdId, currNode->subCmdId);
        if (NULL == ct_p)
        {
            return -1;
        }

        memset(COMM_CTX(currNode->pCtx)->nwBuf, 0, sizeof(COMM_CTX(currNode->pCtx)->nwBuf));
        //len = cbGetServerListLocalReq(ctx, nwBuf, dataOut)
        len = ((CBLocalReq) ct_p->procR2R) ?
                ((CBLocalReq) ct_p->procR2R)(COMM_CTX(currNode->pCtx), (CmdHeaderInfo *)currNode, COMM_CTX(currNode->pCtx)->nwBuf, UDP_MTU) : 0; // do sth for local req
        // TODO: if POST send has not ready yet.
        if (0 >= len)
        {
            currNode->needReq = 0;
            currNode->rspVal = 0;
            return 0;
        }

        if (currNode->ndPort)
        {
            memcpy(ipTmp, currNode->ndIP, sizeof(ipTmp));
            portTmp = currNode->ndPort;
        }
        else
        {
            memcpy(ipTmp, COMM_CTX(currNode->pCtx)->remoteIP, sizeof(ipTmp));
            portTmp = COMM_CTX(currNode->pCtx)->remotePort;
        }

        // R2R send
        len = nwUDPSendto(currNode->pCtx, ipTmp, portTmp, COMM_CTX(currNode->pCtx)->nwBuf, len);
        if (0 >= len)
        {
            LELOGE("forEachNodeR2RPostSendCB nwUDPSendto [%d]", len);
            return -2;
        }

        //currNode->rspVal = 1;
        currNode->needReq = 0;
    }

    return 0;
}

static int forEachNodeQ2ARspCB(NodeData *currNode, void *uData)
{
    USED(uData);

    //char nwBufOut[UDP_MTU] =
    //{ 0 };
    //int protocolBufLenOut = 0;
    //int isReq;
    CmdRecord *ct_p;
    uint8_t nwBuf[UDP_MTU] = { 0 };
    int nwLen = 0, ret, isRepeat = 0;
    ct_p = getCmdRecord(currNode->cmdId, currNode->subCmdId);
    if (NULL == ct_p)
    {
        return 0;
    }

    /*
     isReq = ct_p->cmdId & 0x1;
     if (!isReq)
     {
     return -2;
     }
     */

    // processing again, the original req data from remote is NULL.
    // 1. req handler & req response
    // 2. pack data respectively
    if (0 < (nwLen = doQ2ARemoteReq(currNode->pCtx, (CmdHeaderInfo *)currNode, NULL, 0, nwBuf, UDP_MTU, &isRepeat)))
    {
        // QA send from second time
        ret = nwUDPSendto(currNode->pCtx, currNode->ndIP, currNode->ndPort, nwBuf, nwLen);
        if (0 >= ret)
        {
            LELOGE("forEachNodeQ2ARspCB nwUDPSendto [%d]", ret);
        }
        currNode->needReq = 0;
        return 0;
    }
    return 0;

}

static int isNeedDelCB(NodeData *currNode) {
    // timeout
    int ret = 0;
    if (currNode->timeoutRef && (halGetTimeStamp() - currNode->timeStamp) > currNode->timeoutRef) {
        LELOG("isNeedDelCB timeoutRef[%d] cmd[%d][%d] left rspVal[%d] ", 
            currNode->timeoutRef, currNode->cmdId, currNode->subCmdId, currNode->rspVal);

        // for retry
        if ((LELINK_CMD_CLOUD_HEARTBEAT_REQ == currNode->cmdId && LELINK_SUBCMD_CLOUD_STATUS_CHANGED_REQ == currNode->subCmdId) ||
            (LELINK_CMD_CTRL_REQ == currNode->cmdId && LELINK_SUBCMD_CTRL_CMD_REQ == currNode->subCmdId) ||
            (LELINK_CMD_CLOUD_MSG_CTRL_C2R_REQ == currNode->cmdId && LELINK_SUBCMD_CLOUD_MSG_CTRL_C2R_REQ == currNode->subCmdId)) {
            uint8_t bRspFlag = 0x01; // for unicast
            NodeData node = {0};
            LELOG("**************** cmd[%d] subCmd[%d], rspVal[%d] reserved2[%d]", 
                currNode->cmdId, currNode->subCmdId, currNode->rspVal, currNode->reserved2);
            // if (LELINK_SUBCMD_DISCOVER_STATUS_CHANGED_REQ == currNode->subCmdId) {
            //     bRspFlag = 0xFF; // for multicast
            // }
            if (!currNode->reserved2) {
                currNode->reserved2 = RETRY_TIMES;
                LELOG("RETRY start");
            }
            // no need to retry OR has got rsp already
            if (RETRY_TIMES < currNode->reserved2 || bRspFlag > currNode->rspVal) {
                LELOG("RETRY is finished");
            } else {
                node.cmdId = currNode->cmdId;
                node.subCmdId = currNode->subCmdId;
                node.seqId = currNode->seqId;
                strcpy(node.ndIP, currNode->ndIP);
                node.ndPort = currNode->ndPort;
                node.reserved = currNode->reserved;
                node.reserved2 = (0 == --currNode->reserved2) ? 0xFF : currNode->reserved2;
                memcpy(node.uuid, currNode->uuid, MAX_UUID);
                memcpy(node.token, currNode->token, AES_LEN);
                lelinkNwPostCmdExt(&node);
                LELOG("RETRY doing... retry[%d] uuid[%s]", node.reserved2, node.uuid);
            }
        }

        switch (currNode->cmdId) {
            case LELINK_CMD_CLOUD_ONLINE_REQ:
            case LELINK_CMD_CLOUD_HEARTBEAT_REQ: {
                if ((currNode->subCmdId == LELINK_SUBCMD_CLOUD_HEARTBEAT_REQ || LELINK_SUBCMD_CLOUD_ONLINE_REQ == currNode->subCmdId) &&
                    !(currNode->reserved)) {
                    if (!flagHeartBeatMinus()) {
                        changeStateId(E_STATE_AP_CONNECTED);
                        flagHeartBeatReset();
                    }
                }
            }
            break;
            // case LELINK_CMD_CLOUD_GET_TARGET_REQ:
            //     {
            //         changeStateId(E_STATE_AP_CONNECTED);
            //     }
            //     break;
        }
        ret = 1;
    }

    // has been sent
    if (!currNode->rspVal && !currNode->needReq) {
        ret = 1;
    }


    return ret;
}

static int doQ2AProcessing(CommonCtx *pCtx, int protocolBufLen, const CmdHeaderInfo *cmdInfo, char ip[MAX_IPLEN], uint16_t port) {
    int ret = 0, isRepeat = 0;
MULTI_LOCAL_RSP:
    ret = doQ2ARemoteReq(pCtx, cmdInfo, pCtx->protocolBuf, protocolBufLen, pCtx->nwBuf, UDP_MTU, &isRepeat);
    if (ret > 0)
    {
        int nwLen = ret;
        LELOG("lelinkDoPollingQ2A nwUDPSendto [%s:%d][%d]", ip, port, nwLen);
        // QA send imediately
        ret = nwUDPSendto(pCtx, ip, port, pCtx->nwBuf, nwLen);
        if (0 >= ret)
        {
            LELOGW("lelinkDoPollingQ2A nwUDPSendto [%s:%d][%d]", ip, port, ret);
        }
        if (isRepeat) {
            goto MULTI_LOCAL_RSP;
        }
    }
    else if (0 == ret)
    {
        NodeData node = { 0 };

        // CmdHeaderInfo
        memcpy(&node, cmdInfo, sizeof(CmdHeaderInfo));
        node.randID = genRand();
        
        // rest part
        node.pCtx = pCtx;
        node.timeStamp = halGetTimeStamp();
        node.timeoutRef = 30;
        node.needReq = 1;
        node.rspVal = 0;
        memcpy(node.ndIP, ip, MAX_IPLEN);
        node.ndPort = port;
        node.seqId = cmdInfo->seqId;

        qEnCache(&(pCtx->cacheCmd), (void *)&node);
    }
    return ret;
}

void postReboot(void) {
    NodeData node = {0};
    node.cmdId = LELINK_CMD_ASYNC_REBOOT_REQ;
    node.subCmdId = LELINK_SUBCMD_ASYNC_REBOOT_REQ;
    lelinkNwPostCmdExt(&node);
}

void postSDevRecordChanged(int index, int kind) {
    NodeData node = {0};
    node.cmdId = LELINK_CMD_CLOUD_REPORT_REQ;
    node.subCmdId = LELINK_SUBCMD_CLOUD_REPORT_SDEV_RECORD_REQ;
    node.reserved = index + 1;
    node.reserved1 = kind;
    lelinkNwPostCmdExt(&node);
}
/* hello */
static int cbHelloLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen) {
    int ret = 0;
    char helloReq[MAX_BUF] = {"{\"msg\":\"hello\""};
    int helloReqLen = strlen(helloReq);
    ret = getTerminalStatus(helloReq + helloReqLen, sizeof(helloReq) - helloReqLen);
    if (ret > 0) {
        helloReq[helloReqLen] = ',';
    } else {
        helloReq[helloReqLen] = '}';
        helloReq[helloReqLen + 1] = 0;
    }
    LELOG("cbHelloLocalReq [%s] -s", helloReq);

    ret = doPack(ctx, ENC_TYPE_STRATEGY_11, cmdInfo, (const uint8_t *)helloReq, strlen(helloReq), dataOut, dataLen);
    LELOG("cbHelloLocalReq [%d] -e", ret);
    return ret;
}

static void cbHelloRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen) {
    //int ret = 0;
    // CommonCtx *pCtx = COMM_CTX(ctx);
    LELOG("cbHelloRemoteRsp -s");
    if (0 < dataLen) 
        LELOG("[%d][%s]", dataLen, dataIn);
    LELOG("cbHelloRemoteRsp -e");
}

/* for sdk, discover a new device */
static int cbHelloRemoteReq(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen) {
    int ret = 0;
    // CommonCtx *pCtx = COMM_CTX(ctx);
    LELOG("cbHelloRemoteReq -s");
    LELOG("[%d][%s]", dataLen, dataIn);
    LELOG("cbHelloRemoteReq -e");
    ret = halCBRemoteReq(ctx, cmdInfo, dataIn, dataLen);
	return ret > 0 ? 1 : -1;
}

static int cbHelloLocalRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len, uint8_t *dataOut, int dataLen) {
    int ret = 0;
    //char rspHello[] = "{ \"msg\":\"i know u got it.\" }";
    // CommonCtx *pCtx = COMM_CTX(ctx);
    LELOG("cbHelloLocalRsp -s");
    ret = doPack(ctx, ENC_TYPE_STRATEGY_11, cmdInfo, data, ret, dataOut, dataLen);
    LELOG("cbHelloLocalRsp -e");
    return ret;
}

/* discovery */
static int cbDiscoverLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen) {
    int ret = 0;
    char reqDiscover[48] = {0};
    // CommonCtx *pCtx = COMM_CTX(ctx);
    LELOG("cbDiscoverLocalReq -s");

    ret = getJsonUTC(reqDiscover, sizeof(reqDiscover));
    if (0 >= ret) {
        ret = 0;
    }
    ret = doPack(ctx, ENC_TYPE_STRATEGY_11, cmdInfo, (const uint8_t *)reqDiscover, ret, dataOut, dataLen);
    
    LELOG("cbDiscoverLocalReq [%d] -e", ret);
    return ret;
}

static void cbDiscoverRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen) {
    //int ret = 0;
    // CommonCtx *pCtx = COMM_CTX(ctx);
    LELOG("cbDiscoverRemoteRsp -s");
    // LELOG("[%d][%s]", dataLen, dataIn);
    halCBRemoteRsp(ctx, cmdInfo, dataIn, dataLen);
    LELOG("cbDiscoverRemoteRsp -e");
}


extern int forEachNodeSDevForNumCB(SDevNode *currNode, void *uData);
static int16_t ginSDevCountsInDiscovery;
static int16_t ginSDevCurrFoundIndex;
static int forEachNodeSDevIteratorCB(SDevNode *currNode, void *uData) {
    int16_t *foundIndex = (int16_t *)uData;
    int16_t index = (((uint8_t *)currNode - (uint8_t *)sdevCache()->pBase)/sdevCache()->singleSize);
    LELOG("[SENGINE] forEachNodeSDevIteratorCB *foundIndex[%d] index[%d] isSDevInfoDone[%02x]", *foundIndex, index, currNode->isSDevInfoDone);
    if (0x08 == (0x08 & currNode->isSDevInfoDone)) {
        if (*foundIndex <= index) {
            *foundIndex = index + 1;
            return 1;
        }
    }
    return 0;
}
static int cbDiscoverRemoteReq(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen) {
    int ret = 1;
    // CommonCtx *pCtx = COMM_CTX(ctx);
    LELOG("cbDiscoverRemoteReq -s");
    LELOG("[%d][%s]", dataLen, dataIn);
    
    // it is not comming from simu
    // if (memcmp(cmdInfo->uuid, "d05bca44feb34aeca2dd", 20)) {
    //     if (isCloudOnlined() && getLock()) {
    //         return -1; // drop this req, it means no rsp
    //     }
    // }

    LELOG("cbDiscoverRemoteReq [%d] -e", ret + ginSDevCountsInDiscovery);
    return ret + ginSDevCountsInDiscovery;
}

static int cbDiscoverLocalRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len, uint8_t *dataOut, int dataLen) {
    int ret = 0, validSize = 0, encType = 0;
    // CommonCtx *pCtx = COMM_CTX(ctx);
    char rspDiscover[MAX_BUF] = {0};
    LELOG("cbDiscoverLocalRsp ======> ginSDevCountsInDiscovery[%d] -s", ginSDevCountsInDiscovery);



    // gen status
    if (0 == ginSDevCountsInDiscovery) {
        ret = getTerminalStatus(rspDiscover, sizeof(rspDiscover));
        if (0 >= ret) {
            ret = 0;
        }
    }
    else {
        // TO FIND the index for sdev(valid), and the index should > ginSDevCurrFoundIndex
        int index = 0;
        index = qForEachfromCache(sdevCache(), (int(*)(void*, void*))forEachNodeSDevIteratorCB, &ginSDevCurrFoundIndex);
        if (0 <= index) {
            ret = getSDevStatus(index, rspDiscover, sizeof(rspDiscover));
            LELOG("index[%d] ginSDevCurrFoundIndex[%d] ret[%d][%s]", index, ginSDevCurrFoundIndex, ret, rspDiscover);
        } else {
            ret = 0;
        }
    }

    encType = (LELINK_CMD_CTRL_RSP == cmdInfo->cmdId && LELINK_SUBCMD_CTRL_GET_STATUS_RSP == cmdInfo->subCmdId) ? ENC_TYPE_STRATEGY_13 : ENC_TYPE_STRATEGY_11;
	ret = doPack(ctx, encType, cmdInfo, (const uint8_t *)rspDiscover, ret, dataOut, dataLen);
    LELOG("cbDiscoverLocalRsp <====== ret[%d] [%d] cmd[%d][%d] -e", ret, ginSDevCountsInDiscovery, cmdInfo->cmdId, cmdInfo->subCmdId);

    // clear the flag
    if (0 == ginSDevCountsInDiscovery--) {
        // ginSDevCountsInDiscovery = 0;
        ginSDevCurrFoundIndex = 0;
        if (sengineHasDevs()) {
            qForEachfromCache(sdevCache(), (int(*)(void*, void*))forEachNodeSDevForNumCB, &validSize);
            ginSDevCountsInDiscovery = validSize;            
        } else {
            ginSDevCountsInDiscovery = 0;
        }
    }
    return ret;
}

static int cbDiscoverStatusChangedLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen) {
    int ret = 0;
    char status[MAX_BUF] = {0};
    // char token[2*AES_LEN + 1] = {0};
    LELOG("cbDiscoverStatusChangedLocalReq -s");

    if (!cmdInfo->reserved) {
        ret = getTerminalStatus(status, sizeof(status));
        if (0 > ret) {
            ret = 0;
        } 
    } else {
        ret = getSDevStatus(cmdInfo->reserved-1, status, sizeof(status));
    }

    LELOG("NO need token[%d][%s]", ret, status);
    // test only
    // static int mm = 0;
    // if (!mm) {
    //     strcpy(status, "{\"status\":{\"qca9531\":100,\"idx1\":1,\"idx2\":2,\"idx3\":1,\"idx4\":1},\"cloud\":0,\"uuid\":\"10000100101000010007F0B429000012\",\"ip\":\"192.168.3.109\",\"ver\":\"3-0.9.9-1-1.0\"}");
    // } else {   
    //     strcpy(status, "{\"status\":{\"qca9531\":100,\"idx1\":1,\"idx2\":1,\"idx3\":2,\"idx4\":1},\"cloud\":0,\"uuid\":\"10000100011000510005FFFFFFFFFFFF\",\"ip\":\"192.168.3.109\",\"ver\":\"3-0.9.9-1-1.0\"}");
    // }
    // mm += 1;
    ret = doPack(ctx, ENC_TYPE_STRATEGY_11, cmdInfo, (const uint8_t *)status, ret, dataOut, dataLen);
    
    LELOG("cbDiscoverStatusChangedLocalReq [%d] -e", ret);
    return ret;
}

static void cbDiscoverStatusChangedRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen) {
    LELOG("cbDiscoverStatusChangedRemoteRsp -s");
    // LELOG("[%d][%s]", dataLen, dataIn);
    LELOG("cbDiscoverStatusChangedRemoteRsp -e");
}

static int cbDiscoverStatusChangedRemoteReq(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen) {
    int ret = 1;
    // CommonCtx *pCtx = COMM_CTX(ctx);
    LELOG("======> cbDiscoverStatusChangedRemoteReq -s");
    LELOG("[%d][%s]", dataLen, dataIn);
    senginePollingRules((char *)dataIn, dataLen);
    halCBRemoteReq(ctx, cmdInfo, dataIn, dataLen);
    LELOG("cbDiscoverStatusChangedRemoteReq -e");
    return ret;
}

static int cbDiscoverStatusChangedLocalRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len, uint8_t *dataOut, int dataLen) {
    int ret = 0;
    ret = doPack(ctx, ENC_TYPE_STRATEGY_11, cmdInfo, NULL, 0, dataOut, dataLen);
    LELOG("cbDiscoverStatusChangedLocalRsp -e");
    return ret;
}

static int cbCtrlGetStatusLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen) {

    int ret = 0, encType = -1;
    char reqCtrlGetStatus[128];
    // CommonCtx *pCtx = COMM_CTX(ctx);
    LELOG("cbCtrlGetStatusLocalReq -s");

    ret = halCBLocalReq(ctx, cmdInfo, (uint8_t *)reqCtrlGetStatus, sizeof(reqCtrlGetStatus));
    encType = cmdInfo->token[0] ? ENC_TYPE_STRATEGY_13 : ENC_TYPE_STRATEGY_11;
	ret = doPack(ctx, encType, cmdInfo, (const uint8_t *)reqCtrlGetStatus, ret, dataOut, dataLen);
    
    LELOG("cbCtrlGetStatusLocalReq [%d] -e", ret);
    return ret;
}
static void cbCtrlGetStatusRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen) {
    //int ret = 0;
    // CommonCtx *pCtx = COMM_CTX(ctx);
    LELOG("cbCtrlGetStatusRemoteRsp -s");
    LELOG("[%d][%s]", dataLen, dataIn);
    LELOG("cbCtrlGetStatusRemoteRsp -e");
    return;
}

static int cbCtrlCmdLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen) {

    int ret = 0, encType = -1;
    char reqCtrlCmd[MAX_BUF] = {0};
    // CommonCtx *pCtx = COMM_CTX(ctx);
    LELOG("cbCtrlCmdLocalReq -s");

    ret = halCBLocalReq(ctx, cmdInfo, (uint8_t *)reqCtrlCmd, sizeof(reqCtrlCmd));
    encType = cmdInfo->token[0] ? ENC_TYPE_STRATEGY_13 : ENC_TYPE_STRATEGY_11;
    ret = doPack(ctx, encType, cmdInfo, (const uint8_t *)reqCtrlCmd, ret, dataOut, dataLen);
    
    LELOG("cbCtrlCmdLocalReq [%d] -e", ret);
    return ret;
}
static void cbCtrlCmdRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen) {
    //int ret = 0;
    // CommonCtx *pCtx = COMM_CTX(ctx);
    LELOG("cbCtrlCmdRemoteRsp -s");
    halCBRemoteRsp(ctx, cmdInfo, dataIn, dataLen);
    // LELOG("[%d][%s]", dataLen, dataIn);
    LELOG("cbCtrlCmdRemoteRsp -e");
    return;
}
static int cbCtrlCmdRemoteReq(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen) {
    int ret = 1;
    // uint8_t data[MAX_BUF] = {0};
    // CommonCtx *pCtx = COMM_CTX(ctx);
    LELOG("cbCtrlCmdRemoteReq -s");
    LELOG("[%d][%s]", dataLen, dataIn);
    setTerminalAction(cmdInfo, (const char *)dataIn, dataLen);
    LELOG("cbCtrlCmdRemoteReq [%d] -e", ret);
    // TODO: handle the remote ctrl
    // ret = std2pri((const char *)dataIn, dataLen, data, sizeof(data), &type, NULL);
    // if (0 < ret) {
    //     ret = ioWrite(type, data, ret);
    // }

    return 1;
}
static int cbCtrlCmdLocalRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len, uint8_t *dataOut, int dataLen) {
    int ret = 0, encType = -1;
    // CommonCtx *pCtx = COMM_CTX(ctx);
    char rspCtrlCmd[MAX_BUF] = {0};
    LELOG("cbCtrlCmdLocalRsp -s");
    ret = getTerminalStatus(rspCtrlCmd, sizeof(rspCtrlCmd));
    encType = isCloudOnlined() ? ENC_TYPE_STRATEGY_13 : ENC_TYPE_STRATEGY_11;
    ret = doPack(ctx, encType, cmdInfo, (const uint8_t *)rspCtrlCmd, strlen(rspCtrlCmd), dataOut, dataLen);
    LELOG("cbCtrlCmdLocalRsp -e");
    senginePollingSlave();
    return ret;
}

static int cbCtrlGetStatusRemoteReq(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen) {
    int ret = 1;
    // CommonCtx *pCtx = COMM_CTX(ctx);
    LELOG("cbCtrlGetStatusRemoteReq -s");
    // LELOG("[%d][%s]", dataLen, dataIn);
    // test only
    // resetConfigData();
    ret = cbDiscoverRemoteReq(ctx, cmdInfo, dataIn, dataLen);
    LELOG("cbCtrlGetStatusRemoteReq -e");
    return ret;
}
static int cbCtrlGetStatusLocalRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len, uint8_t *dataOut, int dataLen) {
    // int ret = 0, encType = -1;
    // // CommonCtx *pCtx = COMM_CTX(ctx);
    // char status[MAX_BUF] = {0};
    // char strMac[32] = {0};
    // LELOG("cbCtrlGetStatusLocalRsp [%s] cmd[%d][%d] -s", (char *)data, cmdInfo->cmdId, cmdInfo->subCmdId);

    // if (!getStrValByKey((char *)data, len, JSON_NAME_SDEV_MAC, strMac, sizeof(strMac))) {
    //     ret = qForEachfromCache(sdevCache(), (int(*)(void*, void*))forEachNodeSDevByMacCB, strMac);
    //     if (0 <= ret) {
    //         ret = getSDevStatus(ret, status, sizeof(status));
    //     } else {
    //         ret = 0;
    //     }
    // } else {
    //     ret = getTerminalStatus(status, sizeof(status));
    // }

    // encType = isCloudOnlined() ? ENC_TYPE_STRATEGY_13 : ENC_TYPE_STRATEGY_11;
    // ret = doPack(ctx, encType, cmdInfo, (const uint8_t *)status, ret > 0 ? ret : 0, dataOut, dataLen);
    // // ret = getTerminalStatus(binStatus, sizeof(binStatus));
    // LELOG("cbCtrlGetStatusLocalRsp status[%s] -e", status);
    // return ret;
    return cbDiscoverLocalRsp(ctx, cmdInfo, data, len, dataOut, dataLen);
}

static int cbCloudGetTargetLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen) {
    int ret = 0;
    uint8_t signature[MAX_BUF] = {0};
    int lenSignature = 0;

    // CommonCtx *pCtx = COMM_CTX(ctx);
    // CmdHeaderInfo cmdInfo = {0};
    LELOG("cbCloudGetTargetLocalReq -s");

    // cmdInfo.status = 0;
    // cmdInfo.cmdId = LELINK_CMD_CLOUD_GET_TARGET_REQ;
    // cmdInfo.subCmdId = LELINK_SUBCMD_CLOUD_GET_TARGET_REQ;
    // cmdInfo.seqId = 0;

    lenSignature = getTerminalSignature(signature, RSA_LEN);
    // sprintf(signature + RSA_LEN, "{\"lock\":%d}", getLock());
    // LELOG("test sig len[%d], total len[%d] [%s]", lenSignature, lenSignature + strlen(signature + RSA_LEN), signature + RSA_LEN);
    ret = doPack(ctx, ENC_TYPE_STRATEGY_12, cmdInfo, signature, lenSignature /*+ strlen(signature + RSA_LEN)*/, dataOut, dataLen);
    
    if(ret <= 0) {
        changeStateId(E_STATE_AP_CONNECTED);
    }

    LELOG("cbCloudGetTargetLocalReq [%d] -e", ret);
    return ret;
}

static void cbCloudGetTargetRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen) {
    int ret = 0;
    // CommonCtx *pCtx = COMM_CTX(ctx);
    LELOG("cbCloudGetTargetRemoteRsp -s");
    LELOG("total[%d] json[%s]", dataLen, dataIn + RSA_LEN);
    char ip[MAX_IPLEN] = {0};
    uint16_t port = 0;
    uint8_t tPubkey[256] = {0};
    int tPubkeyLen = 0;
    tPubkeyLen = getTerminalPublicKey(tPubkey, sizeof(tPubkey));

    if (0 != (ret = rsaVerify(tPubkey, tPubkeyLen, dataIn + RSA_LEN, dataLen - RSA_LEN, dataIn, RSA_LEN))) {
        LELOGW("cbCloudGetTargetRemoteRsp rsaVerify Failed[%d]", ret);
        changeStateId(E_STATE_AP_CONNECTED);
        return;
    }

    if (isNeedToRedirect((const char *)dataIn, dataLen, ip, &port)) {
        NodeData node = { 0 };
        node.cmdId = LELINK_CMD_CLOUD_AUTH_REQ;
        node.subCmdId = LELINK_SUBCMD_CLOUD_AUTH_REQ; 
        memcpy(node.ndIP, ip, MAX_IPLEN);
        node.ndPort = port;
        lelinkNwPostCmd(ctx, &node);
        changeStateId(E_STATE_CLOUD_LINKED);
    } else {
        // auth done
        syncUTC((const char *)dataIn + RSA_LEN, dataLen - RSA_LEN);
        // startHeartBeat();
        halCBRemoteRsp(ctx, cmdInfo, dataIn + RSA_LEN, dataLen - RSA_LEN);
        changeStateId(E_STATE_CLOUD_AUTHED);
    }

    LELOG("cbCloudGetTargetRemoteRsp -e");
}

static int cbCloudOnlineLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen) {
    int ret = 0;
    char token[2*AES_LEN + 1] = {0};
    char status[MAX_BUF] = {0};
    int encLen = 0;
    uint8_t pubkey[256] = {0};
    uint8_t encToken[RSA_LEN] = {0};
    int pubkeyLen = 0;
    LELOG("cbCloudOnlineLocalReq -s");
    
    getTerminalTokenStr(token, sizeof(token));
    pubkeyLen = getTerminalPublicKey(pubkey, sizeof(pubkey));
    encLen = rsaEncrypt(pubkey, pubkeyLen, (const uint8_t *)token, strlen(token), encToken, sizeof(encToken));
    LELOG("rsaEncrypt ret[%d] token[%s]", ret, token);
    if (0 >= encLen) {
        return -1;
    }
    memcpy(status, encToken, encLen);

    // TODO: token should be encrypted by T-pubkey
    if (!cmdInfo->reserved) {
        ret = getTerminalStatus(status + encLen, sizeof(status) - encLen);
    } else {
        ret = getSDevStatus(cmdInfo->reserved-1, status + encLen, sizeof(status) - encLen);
    }
    ret += encLen;
    // LELOG("No token [%d][%d] status[%s] reserved[%d]", ret, strlen(status + RSA_LEN), status + RSA_LEN, cmdInfo->reserved);
    // status[ret] = '}';
    // ret += 1;
    // ret = sprintf(status + ret - 1, ",\"token\":\"%s\"}", token);
    // LELOG("No token [%d] status[%s] reserved[%d]", ret, status + encLen, cmdInfo->reserved);

    ret = doPack(ctx, ENC_TYPE_STRATEGY_12, cmdInfo, (const uint8_t *)status, ret, dataOut, dataLen);
    
    LELOG("cbCloudOnlineLocalReq [%d] reserved[%d] -e", ret, cmdInfo->reserved);
    return ret;
}

static void cbCloudOnlineRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen) {
    //int ret = 0;
    // CommonCtx *pCtx = COMM_CTX(ctx);
    LELOG("cbCloudOnlineRemoteRsp -s");
    if (0 > cmdInfo->status) {
        changeStateId(E_STATE_AP_CONNECTED);
    } else {
        changeStateId(E_STATE_CLOUD_ONLINE);
    }
    // LELOG("Now version: %s-%s", __DATE__, __TIME__);
    // LELOG("[%d][%s]", dataLen, dataIn);
    halCBRemoteRsp(ctx, cmdInfo, dataIn, dataLen);
    LELOG("cbCloudOnlineRemoteRsp -e");
}


static int cbCloudAuthLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen) {
    int ret = 0;
    uint8_t signature[RSA_LEN] = {0};
    int lenSignature = 0;
    // CommonCtx *pCtx = COMM_CTX(ctx);
    // CmdHeaderInfo cmdInfo = {0};
    LELOG("cbCloudAuthLocalReq -s");

    // cmdInfo.status = 0;
    // cmdInfo.cmdId = LELINK_CMD_CLOUD_AUTH_REQ;
    // cmdInfo.subCmdId = LELINK_SUBCMD_CLOUD_AUTH_REQ;
    // cmdInfo.seqId = 0;

    lenSignature = getTerminalSignature(signature, RSA_LEN);
    ret = doPack(ctx, ENC_TYPE_STRATEGY_12, cmdInfo, signature, lenSignature, dataOut, dataLen);
    
    if(ret <= 0) {
        changeStateId(E_STATE_AP_CONNECTED);
    }

    LELOG("cbCloudAuthLocalReq [%d] -e", ret);
    return ret;
}

static void cbCloudAuthRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen) {
    //int ret = 0;
    // CommonCtx *pCtx = COMM_CTX(ctx);
    LELOG("cbCloudAuthRemoteRsp -s");
    LELOG("[%d][%s]", dataLen, dataIn);

    // auth done
    if (0 > cmdInfo->status) {
        changeStateId(E_STATE_AP_CONNECTED);
    } else {
        syncUTC((const char *)dataIn + RSA_LEN, dataLen - RSA_LEN);
        // startHeartBeat();
        halCBRemoteRsp(ctx, cmdInfo, dataIn + RSA_LEN, dataLen - RSA_LEN);
        changeStateId(E_STATE_CLOUD_AUTHED);
    }
    LELOG("cbCloudAuthRemoteRsp -e");
}

static int forEachNodeSDevLogCB(SDevNode *currNode, void *uData) {
    char *sdevStatus = (char *)uData;
    if (0x08 == (0x08 & currNode->isSDevInfoDone)) {
        int16_t index = (((uint8_t *)currNode - (uint8_t *)sdevCache()->pBase)/sdevCache()->singleSize);
        memset(sdevStatus, 0, MAX_BUF);
        getSDevStatus(index, sdevStatus, MAX_BUF);
        // LELOG("[SENGINE] forEachNodeSDevLogCB [%s]", sdevStatus);
    }
    return 0;
}

static int cbCloudHeartBeatLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen) {
    int ret = 0;

    LELOG("cbCloudHeartBeatLocalReq -s");
    if (1) {
        char buf[MAX_BUF] = {0};
        getTerminalStatus(buf, sizeof(buf));
        if (sengineHasDevs()) {
            qForEachfromCache(sdevCache(), (int(*)(void*, void*))forEachNodeSDevLogCB, (void *)buf);
        }
    }
    ret = doPack(ctx, ENC_TYPE_STRATEGY_11, cmdInfo, NULL, 0, dataOut, dataLen);
    
    LELOG("cbCloudHeartBeatLocalReq [%d] getProtocolVer[%d] -e", ret, getProtocolVer());
    return ret;
}

static void cbCloudHeartBeatRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen) {
    //int ret = 0;
    // CommonCtx *pCtx = COMM_CTX(ctx);
    LELOG("cbCloudHeartBeatRemoteRsp -s");
    // LELOG("Now version: %s-%s", __DATE__, __TIME__);
    // LELOG("[%d][%s]", dataLen, dataIn);
	halCBRemoteRsp(ctx, cmdInfo, dataIn, dataLen);
    LELOG("cbCloudHeartBeatRemoteRsp -e");
}


static int cbCloudStatusChangedLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen) {
    int ret = 0;
    // char token[2*AES_LEN + 1] = {0};
    // char account[128] = {0};
    char status[MAX_BUF] = {0};
    // char token[2*AES_LEN + 1] = {0};
    // CommonCtx *pCtx = COMM_CTX(ctx);
    // CmdHeaderInfo cmdInfo = {0};
    LELOG("cbCloudStatusChangedLocalReq -s");

    if (!cmdInfo->reserved) {
        ret = getTerminalStatus(status, sizeof(status));
        if (0 > ret) {
            ret = 0;
        }
    } else {
        ret = getSDevStatus(cmdInfo->reserved-1, status, sizeof(status));
    }

    // if (0 < ret) {
    //     getTerminalTokenStr(token, sizeof(token));
    //     sprintf(status + ret - 1, ",\"token\":\"%s\"}", token);
    // } 
    // LELOG("No need token [%s]", status);

    ret = doPack(ctx, ENC_TYPE_STRATEGY_13, cmdInfo, (const uint8_t *)status, ret, dataOut, dataLen);
    
    LELOG("cbCloudStatusChangedLocalReq [%d] -e", ret);
    return ret;
}

static void cbCloudStatusChangedRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen) {
    //int ret = 0;
    // CommonCtx *pCtx = COMM_CTX(ctx);
    LELOG("cbCloudStatusChangedRemoteRsp -s");
    // halCBRemoteRsp(ctx, cmdInfo, dataIn, dataLen);
    LELOG("cbCloudStatusChangedRemoteRsp -e");
}

static int cbCloudIAExeNotifyLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen) {
    int ret = 0;
    char status[128] = {0};
    LELOG("cbCloudIAExeNotifyLocalReq -s");
    if(cmdInfo->reserved >= MAX_IA) {
        LELOGE("Error IA index(%d)", cmdInfo->reserved);
        return -1;
    }
    snprintf(status, sizeof(status), "{\"name\": \"%s\"}", ginIACache.cache[cmdInfo->reserved].ruleName);
    ret = doPack(ctx, ENC_TYPE_STRATEGY_13, cmdInfo, (const uint8_t *)status, strlen(status), dataOut, dataLen);
    LELOG("cbCloudIAExeNotifyLocalReq -e");
    return ret;
}

static void cbCloudIAExeNotifyRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen) {
    LELOG("cbCloudIAExeNotifyRemoteRsp -s");
    LELOG("cbCloudIAExeNotifyRemoteRsp -e");
}

static int cbCloudMsgCtrlC2RLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen) {
    int ret = 0, len = 0;
    // CommonCtx *pCtx = COMM_CTX(ctx);  
    // CmdHeaderInfo tmpCmdInfo;
    char rmtCtrl[256] = {0};
    LELOG("cbCloudMsgCtrlC2RLocalReq -s");

    // memcpy(&tmpCmdInfo, cmdInfo, sizeof(CmdHeaderInfo));
    // getUUIDFromJson(rmtCtrl, sizeof(getUUIDFromJson), tmpCmdInfo.uuid);

    len = ret = halCBLocalReq(ctx, cmdInfo, (uint8_t *)rmtCtrl, sizeof(rmtCtrl));
    ret = doPack(ctx, ENC_TYPE_STRATEGY_233, cmdInfo, (const uint8_t *)rmtCtrl, ret, dataOut, dataLen);

    LELOG("cbCloudMsgCtrlC2RLocalReq [%d][%s] -e", len, rmtCtrl);
    return ret;
}

static void cbCloudMsgCtrlC2RRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen) {
    LELOG("cbCloudMsgCtrlC2RRemoteRsp -s");
    LELOG("[%d][%s]", dataLen, (char *)dataIn);
    halCBRemoteRsp(ctx, cmdInfo, dataIn, dataLen);
    LELOG("cbCloudMsgCtrlC2RRemoteRsp -e");
    return;
}

// // test only
// static int cbCloudMsgCtrlC2RRemoteReq(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen) {
//     return 0;
// }

// // test only
// static int cbCloudMsgCtrlC2RLocalRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len, uint8_t *dataOut, int dataLen) {
//     return 0;
// }

// // test only
// static int cbCloudMsgCtrlR2TLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen) {
//     return 0;
// }

// // test only
// static void cbCloudMsgCtrlR2TRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen) {
//     return;
// }


static int cbCloudMsgCtrlR2TRemoteReq(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen) {
    // uint8_t data[MAX_BUF] = {0};
    int ret = 0;
    LELOG("cbCloudMsgCtrlR2TRemoteReq -s");
    // LELOG("[%d][%s]", dataLen, dataIn);
    // setCurrentR2T
    // test only
    // {
    //     extern int testGetFreeHeap();
    //     extern const char *getScriptVer();
    //     LELOG("=============================>() [%d]", testGetFreeHeap(1));
    //     const char *ver = getScriptVer();
    //     LELOG("<=============================() [%d][%s]", testGetFreeHeap(0), ver);
    // }
    setTerminalAction(cmdInfo, (const char *)dataIn, dataLen);
    ret = halCBRemoteReq(ctx, cmdInfo, dataIn, dataLen);
    LELOG("cbCloudMsgCtrlR2TRemoteReq [%d] -e", ret);
    return ret > 0 ? 1 : -1;
}

static int cbCloudMsgCtrlR2TLocalRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len, uint8_t *dataOut, int dataLen) {
    int ret = 0;
    // CommonCtx *pCtx = COMM_CTX(ctx);
    char status[MAX_BUF] = {0};
    LELOG("cbCloudMsgCtrlR2TLocalRsp -s");
    // ret = halCBLocalRsp(ctx, cmdInfo, data, len, status, sizeof(status));
    ret = getTerminalStatus(status, sizeof(status));
	ret = doPack(ctx, ENC_TYPE_STRATEGY_233, cmdInfo, (const uint8_t *)status, ret, dataOut, dataLen);
    LELOG("cbCloudMsgCtrlR2TLocalRsp -e");
    senginePollingSlave();
    return ret;
}

static int cbCloudReportLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen) {
    int ret = 0;
    // CommonCtx *pCtx = COMM_CTX(ctx);  
    // CmdHeaderInfo tmpCmdInfo;
    char query[128] = {0};
    // char report[128] = {0};
    LELOG("cbCloudReportLocalReq -s");

    // memcpy(&tmpCmdInfo, cmdInfo, sizeof(CmdHeaderInfo));
    // getqueryFromJson(query, sizeof(getqueryFromJson), tmpCmdInfo.query);


    ret = halCBLocalReq(ctx, cmdInfo, (uint8_t *)query, sizeof(query));
    // sprintf(report, "{\"query\":\"%s\"}", query);
    ret = doPack(ctx, ENC_TYPE_STRATEGY_13, cmdInfo, (const uint8_t *)query, strlen(query), dataOut, dataLen);

    LELOG("cbCloudReportLocalReq -e");
    return ret;
}
static void cbCloudReportRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen) {
    LELOG("cbCloudReportRemoteRsp -s");
    // LELOG("[%d][%s]", dataLen, (char *)dataIn);
    halCBRemoteRsp(ctx, cmdInfo, dataIn, dataLen);
    LELOG("cbCloudReportRemoteRsp -e");
    return;
}

static int cbCloudReportOTAQueryLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen) {
    int ret = 0;
    // CommonCtx *pCtx = COMM_CTX(ctx);  
    // CmdHeaderInfo tmpCmdInfo;
    char query[128] = {0};
    // char uuidAndType[2*MAX_UUID] = {0};
    // char ver[32] = {0};
    // char fmt[32] = "{\"ver\":\"%s\"}";
    // int verPos = 0;
    LELOG("cbCloudReportOTAQueryLocalReq -s");

    ret = halCBLocalReq(ctx, cmdInfo, (uint8_t *)query, sizeof(query));
    // getTerminaluuidAndType((uint8_t *)uuidAndType, MAX_UUID);
    // getVer(ver, sizeof(ver));
    // if (strlen(query) > 0) {
    //     verPos = strlen(query) - 1;
    //     fmt[0] = ',';
    // }
    // sprintf(query + verPos, fmt, ver);
    LELOG("[%d][%s]", ret, query);
    ret = doPack(ctx, ENC_TYPE_STRATEGY_14, cmdInfo, (const uint8_t *)query, ret, dataOut, dataLen);

    LELOG("cbCloudReportOTAQueryLocalReq -e");
    return ret;
}
static void cbCloudReportOTAQueryRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen) {
    LELOG("cbCloudReportOTAQueryRemoteRsp -s");
    LELOG("TOTOAL[%d] json[%d][%s]", dataLen, dataLen - RSA_LEN, (char *)dataIn + RSA_LEN);
    {
    	int i = 0; 
    	LELOG("---------------signature set----------------");
    	for (i = 0; i < RSA_LEN; i++) {
    		LEPRINTF("%02x ", dataIn[i]);
    		if (0 == (i + 1)%16) {
    			LEPRINTF("\r\n");
    		}
    	}
    	LEPRINTF("\r\n");
    }
    otaSetLatestSig(dataIn);
    halCBRemoteRsp(ctx, cmdInfo, dataIn + RSA_LEN, dataLen - RSA_LEN);
    LELOG("cbCloudReportOTAQueryRemoteRsp -e");
    return;
}

static int cbCloudSDevRecordChangedLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen) {
    int ret = 0;
    int preLen = 0;
    // char token[2*AES_LEN + 1] = {0};

    char status[MAX_BUF] = {0};

    LELOG("cbCloudSDevRecordChangedLocalReq -s");
    // TODO: token should be encrypted by T-pubkey
    // getTerminalTokenStr(token, sizeof(token));
    preLen = sprintf(status, "{\"%s\":%d,\"%s\":", JSON_NAME_SDEV_KIND, cmdInfo->reserved1, JSON_NAME_SDEV_PROPERTY);
    // reset sdev
    if (!cmdInfo->reserved) {
        uint8_t tmpUUID[MAX_UUID] = {0};
        char tmpStr[MAX_UUID + 1] = {0};
        getTerminalUUID(tmpUUID, sizeof(tmpUUID));
        memcpy(tmpStr, tmpUUID, MAX_UUID);
        ret = sprintf(status + preLen, "{\"%s\":\"%s\"}", JSON_NAME_UUID, tmpStr);
    } else { // del sdev
        // check if the del pos has been re-occupied.
        if (0x08 <= sdevArray()[cmdInfo->reserved-1].isSDevInfoDone) {
            ret = getSDevStatus(cmdInfo->reserved-1, status + preLen, sizeof(status) - preLen);
        } else {
            // strcpy(status + preLen, "{}"); ret = 2;
            LELOGE("sdev has been re-occupied!!!");
            return -1;
        }
    }
    // ret = sprintf(status + ret - 1, ",\"token\":\"%s\"}", token);
    status[preLen + ret] = '}';
    ret = preLen + ret + 1;
    LELOG("appended token [%d][%s] reserved[%d] reserved1[%d]", ret, status, cmdInfo->reserved, cmdInfo->reserved1);

    ret = doPack(ctx, ENC_TYPE_STRATEGY_13, cmdInfo, (const uint8_t *)status, ret, dataOut, dataLen);
    
    LELOG("cbCloudSDevRecordChangedLocalReq [%d] reserved[%d] -e", ret, cmdInfo->reserved);
    return ret;
}

static void cbCloudSDevRecordChangedRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen) {
    LELOG("cbCloudSDevRecordChangedRemoteRsp -s");
    LELOG("cbCloudSDevRecordChangedRemoteRsp -e");
    return;
}

static int cbCloudMsgCtrlC2RDoOTALocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen) {
    int ret = 0;
    char rmtCtrl[MAX_BUF] = {0};
    LELOG("cbCloudMsgCtrlC2RDoOTALocalReq -s");

    if (otaGetLatestSig()) {
    	{
    		int i = 0; 
    		LELOG("---------------signature get----------------");
    		for (i = 0; i < RSA_LEN; i++) {
    			LEPRINTF("%02x ", otaGetLatestSig()[i]);
    			if (0 == (i + 1)%16) {
    				LEPRINTF("\r\n");
    			}
    		}
    		LEPRINTF("\r\n");
    	}
	    memcpy(rmtCtrl, otaGetLatestSig(), RSA_LEN);
    }
    ret = halCBLocalReq(ctx, cmdInfo, (uint8_t *)rmtCtrl + RSA_LEN, sizeof(rmtCtrl) - RSA_LEN);
    LELOG("cbCloudMsgCtrlC2RDoOTALocalReq halCBLocalReq [%d]", ret);
    ret = doPack(ctx, ENC_TYPE_STRATEGY_233, cmdInfo, (const uint8_t *)rmtCtrl, ret + RSA_LEN, dataOut, dataLen);

    LELOG("cbCloudMsgCtrlC2RDoOTALocalReq [%d] -e", ret);
    return ret;
}
static void cbCloudMsgCtrlC2RDoOTARemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen) {
    LELOG("cbCloudMsgCtrlC2RDoOTARemoteRsp status[%d] -s", cmdInfo->status);
    if (0 < dataLen) {
        LELOG("[%d][%s]", dataLen, (char *)dataIn);
    }
    halCBRemoteRsp(ctx, cmdInfo, dataIn, dataLen);
    otaInfoClean();
    LELOG("cbCloudMsgCtrlC2RDoOTARemoteRsp -e");
    return;
}
static int cbCloudMsgCtrlR2TDoOTARemoteReq(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen) {
    // uint8_t data[MAX_BUF] = {0};
    // int type = 0;
    // char url[MAX_BUF] = {0};
    // uint8_t sig[RSA_LEN] = {0};
    LELOG("cbCloudMsgCtrlR2TDoOTARemoteReq -s");

    LELOG("cbCloudMsgCtrlR2TDoOTARemoteReq -e");
    return 1;
}

static void intDoOTA(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len, uint8_t *dataOut, int dataLen) {
    int ret = 0, force = 0;
    int type = 0;
    char url[MAX_BUF] = {0};
    // uint8_t sig[RSA_LEN] = {0};
    CmdHeaderInfo* tmpCmdInfo = (CmdHeaderInfo *)cmdInfo;
    const char *urlPtr = otaGetLatestUrl();
    LELOG("intDoOTA url[0x%p] lenWithRSA[%d] -s", urlPtr, len);


    if (NULL != urlPtr) {
        tmpCmdInfo->status = LELINK_ERR_BUSY_ERR;
    } else if (len <= RSA_LEN) {
        tmpCmdInfo->status = LELINK_ERR_PARAM_INVALID;
        return;
    } else {
        NodeData node = {0};
        if (0 != getIntValByKey((char *)data + RSA_LEN, len - RSA_LEN, JSON_NAME_FORCE, &force)) {
            force = 0;
        }
        type = getJsonOTAType((char *)data + RSA_LEN, len - RSA_LEN, url, sizeof(url));
        LELOG("TOTOAL[%d][%d] type[%d] force[%d] json[%d][%s] url[%s]", len, data + RSA_LEN, type, force, len - RSA_LEN, (char *)data + RSA_LEN, url);

        {
            int i = 0; 
            LELOG("---------------signature remote----------------");
            for (i = 0; i < RSA_LEN; i++) {
                LEPRINTF("%02x ", data[i]);
                if (0 == (i + 1)%16) {
                    LEPRINTF("\r\n");
                }
            }
            LEPRINTF("\r\n");
        }

        switch (type) {
            case OTA_TYPE_SDEVFW:
            case OTA_TYPE_FW: {
                node.cmdId = LELINK_CMD_ASYNC_OTA_REQ;
                node.subCmdId = LELINK_SUBCMD_ASYNC_OTA_REQ;
                ret = lelinkNwPostCmd(ctx, &node);
                if (!ret) {
                    tmpCmdInfo->status = LELINK_ERR_BUSY_ERR;
                } else {
                    otaSetLatestSig(data);
                    otaSetLatestType(type);
                    otaSetLatestUrl(url, strlen(url));
                }
            }break;
            case OTA_TYPE_AUTH:
            case OTA_TYPE_PRIVATE:
            case OTA_TYPE_SDEVINFO:
            case OTA_TYPE_FW_SCRIPT:
            case OTA_TYPE_IA_SCRIPT: {
                otaSetLatestSig(data);
                otaSetLatestType(type);
                otaSetLatestUrl(url, strlen(url));
                ret = cbAsyncOTALocalReq(NULL, NULL, NULL, 0);
                switch(ret) {
                    case -1: {
                        tmpCmdInfo->status = LELINK_ERR_HTTP_UPDATE;
                    }break;
                    case -7: {
                        tmpCmdInfo->status = LELINK_ERR_WRITE_SCRIPT2;
                    }break;
                    case -8: {
                        tmpCmdInfo->status = LELINK_ERR_VERIFY_SCRIPT;
                    }break;
                    case -9: {
                        tmpCmdInfo->status = LELINK_ERR_WRITE_SCRIPT1;
                    }break;
                    default: {
                        tmpCmdInfo->status = LELINK_ERR_SUCCESS;
                    }break;
                }
            }break;
            default:
                tmpCmdInfo->status = LELINK_ERR_PARAM_INVALID;
            break;
        }    
        // OTA_TYPE_PRIVATE OTA_TYPE_AUTH OTA_TYPE_FW should trig a reboot
        if (LELINK_ERR_SUCCESS == tmpCmdInfo->status && 
            (OTA_TYPE_PRIVATE == type || OTA_TYPE_AUTH == type || OTA_TYPE_SDEVINFO == type || OTA_TYPE_FW == type || force)) {
            postReboot();
        }
    }
    LELOG("intDoOTA type[%d], status[%d]", type, tmpCmdInfo->status);
}

static int cbCloudMsgCtrlR2TDoOTALocalRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len, uint8_t *dataOut, int dataLen) {
    int ret = 0;
    intDoOTA(ctx, cmdInfo, data, len, dataOut, dataLen);
    ret = doPack(ctx, ENC_TYPE_STRATEGY_233, cmdInfo, NULL, 0, dataOut, dataLen);
    LELOG("cbCloudMsgCtrlR2TDoOTALocalRsp [%d] -e", ret);
    return ret;
}

static int cbCloudMsgCtrlC2RTellShareLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen)
{
    int ret = 0;
    char buf[MAX_BUF] = {0};

    LELOG("cbCloudMsgCtrlC2RTellShareLocalReq -s");
    ret = halCBLocalReq(ctx, cmdInfo, (uint8_t *)buf, sizeof(buf));
    ret = doPack(ctx, ENC_TYPE_STRATEGY_233, cmdInfo, (const uint8_t *)buf, ret, dataOut, dataLen);
    LELOG("cbCloudMsgCtrlC2RTellShareLocalReq -e");

    return ret;
}

static void cbCloudMsgCtrlC2RTellShareRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen)
{
    LELOG("cbCloudMsgCtrlC2RTellShareRemoteRsp -s");
    if (0 < dataLen) {
        LELOG("[%d][%s]", dataLen, (char *)dataIn);
    }
    halCBRemoteRsp(ctx, cmdInfo, dataIn, dataLen);
    LELOG("cbCloudMsgCtrlC2RTellShareRemoteRsp -e");
    return;
}

static int cbCloudMsgCtrlR2TTellShareRemoteReq(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen)
{
    LELOG("cbCloudMsgCtrlR2TTellShareRemoteReq -s");
    if (0 < dataLen) {
        LELOG("[%d][%s]", dataLen, (char *)dataIn);
    }
    halCBRemoteReq(ctx, cmdInfo, dataIn, dataLen);
    LELOG("cbCloudMsgCtrlR2TTellShareRemoteReq -e");
    return 1;
}

static int cbCloudMsgCtrlR2TTellShareLocalRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len, uint8_t *dataOut, int dataLen)
{
    int ret;

    LELOG("cbCloudMsgCtrlR2TTellShareLocalRsp -s");
    ret = doPack(ctx, ENC_TYPE_STRATEGY_233, cmdInfo, NULL, 0, dataOut, dataLen);
    LELOG("cbCloudMsgCtrlR2TTellShareLocalRsp -e");

    return ret;
}

static int cbCloudMsgCtrlC2RConfirmShareLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen)
{
    int ret = 0;
    char buf[MAX_BUF] = {0};

    LELOG("cbCloudMsgCtrlC2RConfirmShareLocalReq -s");
    ret = halCBLocalReq(ctx, cmdInfo, (uint8_t *)buf, sizeof(buf));
    ret = doPack(ctx, ENC_TYPE_STRATEGY_233, cmdInfo, (const uint8_t *)buf, ret, dataOut, dataLen);
    LELOG("cbCloudMsgCtrlC2RConfirmShareLocalReq -e");

    return ret;
}

static void cbCloudMsgCtrlC2RConfirmShareRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen)
{
    LELOG("cbCloudMsgCtrlC2RConfirmShareRemoteRsp -s");
    if (0 < dataLen) {
        LELOG("[%d][%s]", dataLen, (char *)dataIn);
    }
    halCBRemoteRsp(ctx, cmdInfo, dataIn, dataLen);
    LELOG("cbCloudMsgCtrlC2RConfirmShareRemoteRsp -e");
    return;
}

static int cbCloudMsgCtrlR2TConfirmShareRemoteReq(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen)
{
    LELOG("cbCloudMsgCtrlR2TConfirmShareRemoteReq -s");
    if (0 < dataLen) {
        LELOG("[%d][%s]", dataLen, (char *)dataIn);
    }
    halCBRemoteReq(ctx, cmdInfo, dataIn, dataLen);
    LELOG("cbCloudMsgCtrlR2TConfirmShareRemoteReq -e");
    return 1;
}

static int cbCloudMsgCtrlR2TConfirmShareLocalRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len, uint8_t *dataOut, int dataLen)
{
    int ret;

    LELOG("cbCloudMsgCtrlR2TConfirmShareLocalRsp -s");
    ret = doPack(ctx, ENC_TYPE_STRATEGY_233, cmdInfo, NULL, 0, dataOut, dataLen);
    LELOG("cbCloudMsgCtrlR2TConfirmShareLocalRsp -e");

    return ret;
}

static int cbCloudIndOTARemoteReq(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen) {
    // int ret = 0;
    // CommonCtx *pCtx = COMM_CTX(ctx);  
    // CmdHeaderInfo tmpCmdInfo;
    // char uuid[64] = {0};
    // char query[128] = {0};
    LELOG("cbCloudIndOTARemoteReq -s");
    // {"force":"0","type":2,"url":"http://g3.letv.cn/232/31/100/resolve-smart/0/le_demo.bin","uuid":"10000100091000610006111111111111"}
    LELOG("[%d][%s]", dataLen - RSA_LEN, dataIn + RSA_LEN);
    LELOG("cbCloudIndOTARemoteReq -e");
    return 1;
}

static int cbCloudIndOTALocalRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len, uint8_t *dataOut, int dataLen) {
    int ret = 0;
    LELOG("cbCloudIndOTALocalRsp -s");
    intDoOTA(ctx, cmdInfo, data, len, dataOut, dataLen);
    ret = doPack(ctx, ENC_TYPE_STRATEGY_13, cmdInfo, NULL, 0, dataOut, dataLen);
    LELOG("cbCloudIndOTALocalRsp [%d] -e", ret);
    return ret;
}

static int cbCloudIndStatusRemoteReq(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len) {
    // int ret = 0;
    // CommonCtx *pCtx = COMM_CTX(ctx);  
    // CmdHeaderInfo tmpCmdInfo;
    // char uuid[64] = {0};
    // char query[128] = {0};
    LELOG("cbCloudIndStatusRemoteReq -s");
    LELOG("[%d][%s]", len, data);
    senginePollingRules((char *)data, len);
    halCBRemoteReq(ctx, cmdInfo, data, len);
    LELOG("cbCloudIndStatusRemoteReq -e");
    return 1;
}

static int cbCloudIndStatusLocalRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len, uint8_t *dataOut, int dataLen) {
    int ret;
    LELOG("cbCloudIndStatusLocalRsp -s");
    // halCBLocalRsp(ctx, cmdInfo, dataIn, dataLen);
    ret = doPack(ctx, ENC_TYPE_STRATEGY_13, cmdInfo, NULL, 0, dataOut, dataLen);
    LELOG("cbCloudIndStatusLocalRsp [%d] -e", ret);

    //char rspHello[] = "{ \"msg\":\"i know u got it.\" }";
    // CommonCtx *pCtx = COMM_CTX(ctx);
    // LELOG("cbHelloLocalRsp -s");
    // LELOG("cbHelloLocalRsp -e");

    return ret;
}

static int cbCloudIndMsgRemoteReq(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len) {
    int ret = 1;
    LELOG("cbCloudIndMsgRemoteReq -s");
    // LELOG("[%d][%s]", len, data);
    // senginePollingRules((char *)data, len);
    // ret = setLock();
    halCBRemoteReq(ctx, cmdInfo, data, len);
    LELOG("cbCloudIndMsgRemoteReq [%d] -e", ret);
    return ret;
}
static int cbCloudIndMsgLocalRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len, uint8_t *dataOut, int dataLen) {
    int ret;
    CmdHeaderInfo* tmpCmdInfo = (CmdHeaderInfo *)cmdInfo;
    LELOG("cbCloudIndMsgLocalRsp -s");
    // halCBLocalRsp(ctx, cmdInfo, dataIn, dataLen);
    ret = cloudMsgHandler((const char *)data, len);
    if (0 > ret) {
        tmpCmdInfo->status = ret;
    }

    ret = doPack(ctx, ENC_TYPE_STRATEGY_13, cmdInfo, NULL, 0, dataOut, dataLen);

    LELOG("cbCloudIndMsgLocalRsp [%d] -e", ret);
    return ret;
}

static int cbAsyncOTALocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen) {
    int type = otaGetLatestType();
    const char *url = otaGetLatestUrl();
    const uint8_t *sig = otaGetLatestSig();
    int ret = 0;
    LELOG("cbAsyncOTALocalReq -s");
    LELOG("cbAsyncOTALocalReq type[%d] url[%s] sig[0x%p]", type, url, sig);
    if (NULL == url || OTA_TYPE_NONE >= type) {
        LELOGE("cbAsyncOTALocalReq URL NOT FOUND");
    } else {
        ret = leOTA(type, url, sig, RSA_LEN);
        // clear the ota info
        otaInfoClean();
    }

    LELOG("cbAsyncOTALocalReq -e");
    return (ret > 0) ? 0 : ret;
}

static void cbAsyncOTARemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen) {
    
}

static int cbAsyncRebootLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen) {
    halReboot();
    return 0;
}

static void cbAsyncRebootRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen) {
    
}

CmdRecord *getCmdRecord(uint32_t cmdId, uint32_t subCmdId) {

    int i = 0;
    for (;tblCmdType[i].cmdId; i++)
    {
        if (tblCmdType[i].cmdId == cmdId && tblCmdType[i].subCmdId == subCmdId)
        {
            if (cmdId % 2)
                LELOG("POST getCmdRecord cmdId[%d] subCmdId[%d]", cmdId, subCmdId);
            return &tblCmdType[i];
        }
    }
    LELOGW("getCmdRecord NULL");
    return NULL;
}

int lelinkDoConfig(const char *configInfo) {
    void *context = NULL;
    static uint32_t ts = 0;

    if(!ginInitOk) {
        LELOGE("Init error!");
        return -1;
    }
    if (30 > (halGetTimeStamp() - ts)) {
        return -1;
    }
    ts = halGetTimeStamp();

    context = airconfig_new(configInfo);
    if (NULL == context) {
        ts = 0;
        return 0;
    }
    while (1) {
        if (airconfig_do_config(context)) {
            break;
        }
    };
    airconfig_delete(context);
    return 0;
}

int lelinkVerify(uint32_t startAddr, uint32_t size) {
    int ret = 0, restSize = size;
    uint8_t tPubkey[256] = {0};
    uint8_t raw[512] = {0};
    int tPubkeyLen = 0;
    uint8_t tmp[20] = {0};
    uint8_t tmpStr[42] = {0};
    void *dev = NULL;
    dev = halFlashOpen();
    if (NULL == dev) {
        LELOGE("lelinkVerify halFlashOpen failed");
        return -0XAB;
    }
    halSha1Start();
    for (;restSize > 0;) {
        ret = halFlashRead(dev, raw, restSize < sizeof(raw) ? restSize : sizeof(raw), startAddr, size - restSize);
        if (0 >= ret) {
            LELOGE("lelinkVerify halFlashRead failed [%d]", ret);
            break;
        }
        restSize -= ret;
        halSha1Update(raw, ret);
    }
    halSha1End(tmp);
    halFlashClose(dev);
    
    bytes2hexStr(tmp, sizeof(tmp), tmpStr, sizeof(tmpStr));
    LELOG("sha1: [%s]", tmpStr);
    if (!otaGetLatestSig()) {
        LELOGE("lelinkVerify otaGetLatestSig failed");
        return -0XAC;
    }
    tPubkeyLen = getTerminalPublicKey(tPubkey, sizeof(tPubkey));
    ret = rsaVerify(tPubkey, tPubkeyLen, tmpStr, strlen(tmpStr), otaGetLatestSig(), RSA_LEN);
    LELOG("lelinkVerify rsaVerify[%d]", ret);
    otaInfoClean();
    return ret;
}

int lelinkVerifyBuf(uint8_t *buf, uint32_t size) {
    int ret = 0;
    uint8_t tPubkey[256] = {0};
    int tPubkeyLen = 0;
    uint8_t tmp[20] = {0};
    uint8_t tmpStr[42] = {0};
    if (NULL == buf) {
        return -0XAB;
    }
    halSha1Start();
    halSha1Update(buf, size);
    halSha1End(tmp);
    
    bytes2hexStr(tmp, sizeof(tmp), tmpStr, sizeof(tmpStr));
    LELOG("sha1: [%s]", tmpStr);
    if (!otaGetLatestSig()) {
        LELOGE("lelinkVerify otaGetLatestSig failed");
        return -0XAC;
    }
    tPubkeyLen = getTerminalPublicKey(tPubkey, sizeof(tPubkey));
    ret = rsaVerify(tPubkey, tPubkeyLen, tmpStr, strlen(tmpStr), otaGetLatestSig(), RSA_LEN);
    LELOG("lelinkVerify rsaVerify[%d]", ret);
    otaInfoClean();
    return ret;
}

static void flagHeartBeatReset(void) {
    ginFlagHeartBeat = RETRY_HEARTBEAT;
}

static int flagHeartBeatMinus(void) {
    LELOG("**********flagHeartBeatMinus [%d]************", ginFlagHeartBeat);
    return ginFlagHeartBeat--;
}
