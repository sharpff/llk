#include "leconfig.h"
#include "state.h"
#include "protocol.h"
#include "io.h"


#define TIMEOUT_BEGIN(ms) {\
    static uint32_t ot;\
    if (0 == ot) {\
        ot = 1;\
    } else {\
        ot++;\
    }\
    if (0 < ((ot * ginMSDelay) / ms)) {

#define TIMEOUT_END ot = 0;}}
int resetConfigData(void);


typedef struct {
    StateId stateIdCurr;
    int worked;
}StateContext;


typedef int (*FPState)(StateContext *cntx);
typedef struct {
    int stateIdCurr;
    FPState fpStateCurr;
    int stateIdPrev;
    FPState fpStatePrev;
    int stateIdNext;
    FPState fpStateNext;
}StateRecord;

static StateContext ginStateCntx;

static void *ginCtxR2R;
static void *ginCtxQ2A;
static uint32_t ginMSDelay;
// static PrivateCfg cfg;

// 0. nothing, 1. req done, 2. rsp done
int8_t ginStateCloudLinked;
int8_t ginStateCloudAuthed;
static int8_t ginConfigStatus;

static int stateProcStart(StateContext *cntx);
static int stateProcConfiguring(StateContext *cntx);
static int stateProcSnifferGot(StateContext *cntx);
static int stateProcApConnecting(StateContext *cntx);
static int stateProcApConnected(StateContext *cntx);
static int stateProcCloudLinked(StateContext *cntx);
static int stateProcCloudAuthed(StateContext *cntx);

// static void resetConfigData(void);
// static int halDoConfig(void *ptr, int ptrLen) {
//     int ret = 0;
//     LELOG("default halDoConfig [%d]\r\n", ret);
//     return ret;
// }
// static int halDoApConnecting(void *ptr, int ptrLen) {
//     int ret = 0;
//     LELOG("default halDoApConnecting [%d]\r\n", ret);
//     return ret;
// }
StateRecord ginStateTbl[] = {
    { E_STATE_START, stateProcStart, E_STATE_NONE, 0, E_STATE_CONFIGURING, stateProcConfiguring },
    { E_STATE_CONFIGURING, stateProcConfiguring, E_STATE_START, stateProcStart, E_STATE_SNIFFER_GOT, stateProcSnifferGot },
    { E_STATE_SNIFFER_GOT, stateProcSnifferGot, E_STATE_START, stateProcStart, E_STATE_AP_CONNECTING, stateProcApConnecting },
    { E_STATE_AP_CONNECTING, stateProcApConnecting, E_STATE_START, stateProcStart, E_STATE_AP_CONNECTED, stateProcApConnected },
    { E_STATE_AP_CONNECTED, stateProcApConnected, E_STATE_AP_CONNECTING, stateProcApConnecting, E_STATE_CLOUD_LINKED, stateProcCloudLinked },
    { E_STATE_CLOUD_LINKED, stateProcCloudLinked, E_STATE_AP_CONNECTED, stateProcApConnected, E_STATE_CLOUD_AUTHED, stateProcCloudAuthed },
    { E_STATE_CLOUD_AUTHED, stateProcCloudAuthed, E_STATE_AP_CONNECTED, stateProcApConnected, E_STATE_NONE, 0 },
    { E_STATE_NONE, 0, E_STATE_NONE, 0, E_STATE_NONE, 0 }
};

static int changeState(int direction, StateContext *cntx, int idx) {
    int ret = 0;
    if (0 > direction) {
        if (E_STATE_NONE != ginStateTbl[idx].stateIdPrev) {
            cntx->stateIdCurr = ginStateTbl[idx].stateIdPrev;
            LELOG("changeState to prev\r\n");
            ret = direction;
        }
    }
    else if (0 < direction) {
        if (E_STATE_NONE != ginStateTbl[idx].stateIdNext) {
            cntx->stateIdCurr = ginStateTbl[idx].stateIdNext;
            LELOG("changeState to next\r\n");
            ret = direction;
        }
    }
    return ret;
}


int lelinkPollingState(uint32_t msDelay, void *r2r, void *q2a) {
    int i = 0, ret = 0, len = 0;
    char status[MAX_BUF] = {0};
    if (NULL == r2r || NULL == q2a) {
        return -100;
    }
    ginCtxQ2A = q2a;
    ginCtxR2R = r2r;
    for (i = 0; ginStateTbl[i].stateIdCurr != E_STATE_NONE; i++) {
        if (ginStateTbl[i].stateIdCurr == ginStateCntx.stateIdCurr) {
            ret = ginStateTbl[i].fpStateCurr(&ginStateCntx);
            break;
        }
    }
    ginMSDelay = msDelay;
    sengineQuerySlave();
    if (0 < msDelay) {
        delayms(msDelay);
    }
    senginePollingSlave();
    len = getTerminalStatus(status, sizeof(status));
    if (0 < len) {
        senginePollingRules(status, len);
    }
    lelinkDoPollingQ2A(ginCtxQ2A);
    lelinkDoPollingR2R(ginCtxR2R);
    return changeState(ret, &ginStateCntx, i);
}

static int stateProcStart(StateContext *cntx) {
    int ret = 0;
    PrivateCfg cfg;
    LELOG("stateProcStart [%d] -s\r\n", ret);
    if (0 == lelinkStorageReadPrivateCfg(&cfg)) {
        LELOG("lelinkStorageReadPrivateCfg [%d]\r\n", cfg.data.nwCfg.configStatus);
        if (cfg.csum == crc8(&(cfg.data), sizeof(cfg.data))) {
            LELOG("csum [0x%02x]\r\n", cfg.csum);
            if (0 < cfg.data.nwCfg.configStatus) {
                ret = cfg.data.nwCfg.configStatus;
                ginConfigStatus = 1;
            }
        }
    }

    if (0 == ret) {
        ret = halDoConfig(NULL, 0);
    }
    // LELOG("stateProcStart [%d]\r\n", ret);
    LELOG("stateProcStart [%d] -e\r\n", ret);
    return ret;
}
static int stateProcConfiguring(StateContext *cntx) {
    int ret = 0;

    // LELOG("stateProcConfiguring [%d] -s\r\n", ret);
    if (1 == ginConfigStatus) {
        ret = 1;
    }
    // LELOG("stateProcConfiguring configStatus[%d] -s\r\n", ginConfigStatus);
    if (0 == ret) {
        ret = halDoConfiguring(NULL, 0);
    }
    // LELOG("stateProcConfiguring [%d] -s\r\n", ret);
    return ret;
}
static int stateProcSnifferGot(StateContext *cntx) {
    int ret = 0;
    PrivateCfg cfg;
    cfg.data.nwCfg.configStatus = 0;

    lelinkStorageReadPrivateCfg(&cfg);
    if (cfg.csum != crc8(&(cfg.data), sizeof(cfg.data))) {
        cfg.data.nwCfg.configStatus = 0;
    }

    if (0 < cfg.data.nwCfg.configStatus) {
        ret = halDoApConnect(&cfg, sizeof(PrivateCfg));
    } else {
        ret = halDoApConnect(NULL, 0);
    }
    ginConfigStatus = 1;
    LELOG("stateProcSnifferGot ret[%d] ginConfigStatus[%d][%d]\r\n", ret, ginConfigStatus, cfg.data.nwCfg.configStatus);
    return ret;
}

static int stateProcApConnecting(StateContext *cntx) {
    int ret = 0;
    // PrivateCfg cfg;
    if (0 == ginConfigStatus) {
        return -1;
    }

    ret = halDoApConnecting(NULL, 0);
    // TODO: do not reset to sniffer, it should be reset by button.
    // TIMEOUT_BEGIN(45000)
    //     ret = -1;
    //     LELOG("stateProcApConnecting/DHCP timeout ot[%d]\r\n", ot);
    //     resetConfigData();
    // TIMEOUT_END

    return ret;
}

static int stateProcApConnected(StateContext *cntx) {
    int count = 3;
    PrivateCfg cfg;
    NodeData node = {0};

    if (0 == ginConfigStatus) {
        return -1;
    }
    
    if (0 == ginStateCloudLinked) {
        node.cmdId = LELINK_CMD_CLOUD_GET_TARGET_REQ;
        node.subCmdId = LELINK_SUBCMD_CLOUD_GET_TARGET_REQ;
        if (ginCtxR2R) {
            if (lelinkNwPostCmd(ginCtxR2R, &node)) {
                ginStateCloudLinked = 1;
            }

        }
    }

    // only for backup
    lelinkStorageReadPrivateCfg(&cfg);
    if (cfg.csum != crc8(&(cfg.data), sizeof(cfg.data))) {
        cfg.data.nwCfg.configStatus = 2;
    }

    if (2 != cfg.data.nwCfg.configStatus && (1 == ginConfigStatus)) {
        char br[32] = {0};
        int ret = 0;
        ret = halGetBroadCastAddr(br, sizeof(br));
            if (0 >= ret) {
            LELOGE("halGetBroadCastAddr error\r\n");
            return 0;
        }

        strcpy(node.ndIP, br);
        node.ndPort = NW_SELF_PORT;
        node.cmdId = LELINK_CMD_HELLO_REQ;
        node.subCmdId = LELINK_SUBCMD_HELLO_REQ;
        while (count--) {
            lelinkNwPostCmd(ginCtxR2R, &node);
        }

        cfg.data.nwCfg.configStatus = 2;
        lelinkStorageWritePrivateCfg(&cfg);
    }
    LELOG("stateProcApConnected ginStateCloudLinked[%d]\r\n", ginStateCloudLinked);
    // ret = halDoApConnected(NULL, 0);
    return (2 == ginStateCloudLinked) ? 1 : 0;
}

static int stateProcCloudLinked(StateContext *cntx) {
    // int ret = 1;
    LELOG("stateProcCloudLinked [%d]\r\n", ginStateCloudAuthed);
    if (0 == ginConfigStatus) {
        return -1;
    }


    // ret = halDoCloudLinked(NULL, 0);
    if (0 > ginStateCloudAuthed) {
        return -1;
    } else if (0 == ginStateCloudAuthed || 1 == ginStateCloudAuthed) {
        return 0;
    } else {
        NodeData node = {0};
        node.cmdId = LELINK_CMD_CLOUD_HEARTBEAT_REQ;
        node.subCmdId = LELINK_SUBCMD_CLOUD_HEARTBEAT_REQ;
        if (ginCtxR2R) {
            if (lelinkNwPostCmd(ginCtxR2R, &node)) {

            }
        }
        return 1;
    }
}

// #include <header.h>
// #include <misc.h>
// #include <rfget.h>
// #include <flash.h>
// #include <httpc.h>
// #include <ftfs.h>
// #include <partition.h>
static int stateProcCloudAuthed(StateContext *cntx) {
    // int ret = 0;
    // LELOG("stateProcCloudAuthed [%d]\r\n", ginStateCloudAuthed);
    // ret = halDoCloudAuthed(NULL, 0);
    if (0 == ginConfigStatus) {
        return -1;
    }
    /*{*/
        /*int status;                                                                                                                                                          */
        /*http_session_t handle;*/
        /*http_resp_t *resp = NULL;*/
        /*const httpc_cfg_t *cfg = NULL;*/
        /*[>const char *url_str = "http://g3.letv.cn/190/24/107/resolve-smart/0/gujian_ceshi.bin";<]*/
        /*const char *url_str = "http://115.182.63.167/fei/le_demo.bin";*/

        /*rfget_init();*/
        /*status = httpc_get(url_str, &handle, &resp, cfg);*/
        /*if (status != 0) {*/
            /*rf_e("Unable to connect to server");*/
            /*return -1;*/
        /*}   */
        /*if (resp->status_code != 200) {*/
            /*rf_e("HTTP Error %d", resp->status_code);*/
            /*return -1;*/
        /*}   */
        /*if (resp->chunked) {*/
            /*rf_e("HTTP chunked fs update is not supported");*/
            /*return -1;*/
        /*}  */
        /*LELOGE("url:%s\r\n", url_str);*/
        /*LELOGE("filesize:%d\r\n", resp->content_length);*/
        /*delayms(1000 * 10);*/
    /*}*/

    TIMEOUT_BEGIN(12000)
        NodeData node = {0};
        node.cmdId = LELINK_CMD_CLOUD_HEARTBEAT_REQ;
        node.subCmdId = LELINK_SUBCMD_CLOUD_HEARTBEAT_REQ;
        if (ginCtxR2R) {
            if (lelinkNwPostCmd(ginCtxR2R, &node)) {

            }
        }
        LELOG("stateProcCloudAuthed timeout\r\n");
    TIMEOUT_END

    return ginStateCloudAuthed;
}

int resetConfigData(void) {
    PrivateCfg cfg;
    int ret = 0;
    ret = lelinkStorageReadPrivateCfg(&cfg);
    if (0 <= ret) {
        cfg.data.nwCfg.configStatus = 0;
        ginConfigStatus = 0;
        ret = lelinkStorageWritePrivateCfg(&cfg);
    }
    return ret; 
}
int lelinkNwPostCmdExt(const void *node) {
    return lelinkNwPostCmd(ginCtxR2R, node);
}
