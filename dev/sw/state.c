#include "leconfig.h"
#include "state.h"
#include "protocol.h"
#include "io.h"




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
int8_t ginConfigStatus;

static int stateProcStart(StateContext *cntx);
static int stateProcConfiguring(StateContext *cntx);
static int stateProcSnifferGot(StateContext *cntx);
static int stateProcApConnecting(StateContext *cntx);
static int stateProcApConnected(StateContext *cntx);
static int stateProcCloudLinked(StateContext *cntx);
static int stateProcCloudAuthed(StateContext *cntx);

static void resetConfigData(void);
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
    { E_STATE_CLOUD_LINKED, stateProcCloudLinked, E_STATE_NONE, 0, E_STATE_CLOUD_AUTHED, stateProcCloudAuthed },
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
    int i = 0, ret = 0;
    ginCtxQ2A = q2a;
    ginCtxR2R = r2r;
    for (i = 0; ginStateTbl[i].stateIdCurr != E_STATE_NONE; i++) {
        if (ginStateTbl[i].stateIdCurr == ginStateCntx.stateIdCurr) {
            ret = ginStateTbl[i].fpStateCurr(&ginStateCntx);
            break;
        }
    }
    ginMSDelay = msDelay;
    if (0 < msDelay) {
        delayms(msDelay);
    }
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
    LELOG("stateProcSnifferGot ret[%d] ginConfigStatus[%d][%d]\r\n", ret, ginConfigStatus, cfg.data.nwCfg.configStatus);
    return ret;
}

static int stateProcApConnecting(StateContext *cntx) {
    int ret = 0;
    // PrivateCfg cfg;
    ret = halDoApConnecting(NULL, 0);
    // TODO: do not reset to sniffer, it should be reset by button.
    TIMEOUT_BEGIN(45000)
        ret = -1;
        LELOG("stateProcApConnecting/DHCP timeout ot[%d]\r\n", ot);
        resetConfigData();
    TIMEOUT_END

    return ret;
}

static int stateProcApConnected(StateContext *cntx) {
    int count = 3;
    PrivateCfg cfg;
    NodeData node = {0};
    
    if (0 == ginStateCloudLinked) {
        node.cmdId = LELINK_CMD_CLOUD_GET_TARGET_REQ;
        node.subCmdId = LELINK_SUBCMD_CLOUD_GET_TARGET_REQ;
        if (ginCtxR2R) {
            if (lelinkNwPostCmd(ginCtxR2R, &node)) {
                ginStateCloudLinked = 1;
            }

        }
    }

    lelinkStorageReadPrivateCfg(&cfg);
    if (cfg.csum != crc8(&(cfg.data), sizeof(cfg.data))) {
        cfg.data.nwCfg.configStatus = 2;
    }
    if (2 != cfg.data.nwCfg.configStatus) {
        strcpy(node.ndIP, "255.255.255.255");
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
    // ret = halDoCloudLinked(NULL, 0);
    if (0 > ginStateCloudAuthed) {
        return -1;
    } else if (0 == ginStateCloudAuthed || 1 == ginStateCloudAuthed) {
        return 0;
    } else {
        return 1;
    }
}

static int stateProcCloudAuthed(StateContext *cntx) {
    // int ret = 0;
    LELOG("stateProcCloudAuthed [%d]\r\n", ginStateCloudAuthed);
    // ret = halDoCloudAuthed(NULL, 0);

    TIMEOUT_BEGIN(15000)
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

static void resetConfigData(void) {
    PrivateCfg cfg;
    lelinkStorageReadPrivateCfg(&cfg);
    cfg.data.nwCfg.configStatus = 0;
    ginConfigStatus = 0;
    lelinkStorageWritePrivateCfg(&cfg);
}