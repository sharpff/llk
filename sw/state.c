#include "leconfig.h"
#include "state.h"
#include "protocol.h"
#include "sengine.h"
#include "io.h"
#include "sengine.h"

#ifndef LOG_STATE
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

#define TIMEOUT_BEGIN(ms) {\
    static uint32_t ot;\
    if (0 == ot) {\
        ot = 1;\
    } else {\
        ot++;\
    }\
    if (0 < ((ot * ginMSDelay) / ms)) {\
        ot = 0;

#define TIMEOUT_END }}

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
PrivateCfg ginPrivateCfg;
static int8_t ginConfigStatus;
static StateId ginStateId = E_STATE_NONE;

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
//     LELOG("default halDoConfig [%d]", ret);
//     return ret;
// }
// static int halDoApConnecting(void *ptr, int ptrLen) {
//     int ret = 0;
//     LELOG("default halDoApConnecting [%d]", ret);
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


int isApConnected(void)
{
    return ginPrivateCfg.data.nwCfg.configStatus > 1;
}

int isCloudAuthed(void)
{
    return (ginStateCntx.stateIdCurr == E_STATE_CLOUD_AUTHED);
}

StateId changeStateId(StateId state) {
    ginStateId = state;
    return ginStateCntx.stateIdCurr; 
}

static int changeState(int direction, StateContext *cntx, int idx) {
    int ret = 0;

    if(ginStateId != E_STATE_NONE) {
        direction = ginStateId - ginStateCntx.stateIdCurr;
        LELOGE("Protocol set %d -> %d", ginStateCntx.stateIdCurr, ginStateId);
    }
    if (0 > direction) {
        if (E_STATE_NONE != ginStateTbl[idx].stateIdPrev) {
            ginStateCntx.stateIdCurr = ginStateTbl[idx].stateIdPrev;
            LELOG("changeState to prev");
            ret = direction;
        }
    } else if (0 < direction) {
        if (E_STATE_NONE != ginStateTbl[idx].stateIdNext) {
            ginStateCntx.stateIdCurr = ginStateTbl[idx].stateIdNext;
            LELOG("changeState to next");
            ret = direction;
        }
    }
    ginStateId = E_STATE_NONE;
    if(direction) {
        switch(ginStateCntx.stateIdCurr)
        {
            case E_STATE_CONFIGURING:
                sengineQuerySlave(QUERIETYPE_WAITCONFIG);
                break;
            case E_STATE_SNIFFER_GOT:
            //case E_STATE_AP_CONNECTING:
                sengineQuerySlave(QUERIETYPE_CONNECTING);
                break;
            case E_STATE_AP_CONNECTED:
                sengineQuerySlave(QUERIETYPE_CONNECTED);
                break;
            case E_STATE_CLOUD_LINKED:
            //case E_STATE_CLOUD_AUTHED:
                sengineQuerySlave(QUERIETYPE_CLOUD);
                break;
            default:
                break;
        }
    }
    return ret;
}


int lelinkPollingState(uint32_t msDelay, void *r2r, void *q2a) {
    int i = 0, ret = 0;
    // char status[MAX_BUF] = {0};
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
    
    TIMEOUT_BEGIN(100)
    senginePollingSlave();
    TIMEOUT_END

    if(ginStateCntx.stateIdCurr >= E_STATE_AP_CONNECTED && ginStateCntx.stateIdCurr <= E_STATE_CLOUD_AUTHED) {
        TIMEOUT_BEGIN(1000)
        senginePollingRules(NULL, 0);
        TIMEOUT_END

        TIMEOUT_BEGIN(300)
        sengineQuerySlave(QUERIETYPE_STATE);
        TIMEOUT_END

        TIMEOUT_BEGIN(100)
        lelinkDoPollingQ2A(ginCtxQ2A);
        lelinkDoPollingR2R(ginCtxR2R);
        TIMEOUT_END
    }

    delayms(msDelay);
    return changeState(ret, &ginStateCntx, i);
}

static int stateProcStart(StateContext *cntx) {
    int ret = 0;

    LELOG("stateProcStart [%d] -s", ret);
    if (0 == lelinkStorageReadPrivateCfg(&ginPrivateCfg)) {
        LELOG("lelinkStorageReadPrivateCfg [%d]", ginPrivateCfg.data.nwCfg.configStatus);
        if (ginPrivateCfg.csum == crc8(&(ginPrivateCfg.data), sizeof(ginPrivateCfg.data))) {
            LELOG("csum [0x%02x]", ginPrivateCfg.csum);
            if (0 < ginPrivateCfg.data.nwCfg.configStatus) {
                ret = ginPrivateCfg.data.nwCfg.configStatus;
                ginConfigStatus = 1;
            }
        }
    }

    if (0 == ret) {
        ret = halDoConfig(NULL, 0);
    }
    // LELOG("stateProcStart [%d]", ret);
    LELOG("stateProcStart [%d] -e", ret);
    return ret;
}
static int stateProcConfiguring(StateContext *cntx) {
    int ret = 0;

    // LELOG("stateProcConfiguring [%d] -s", ret);
    if (1 == ginConfigStatus) {
        ret = 1;
    }
    // LELOG("stateProcConfiguring configStatus[%d] -s", ginConfigStatus);
    if (0 == ret) {
        ret = halDoConfiguring(NULL, 0);
    }
    // LELOG("stateProcConfiguring [%d] -s", ret);
    return ret;
}
static int stateProcSnifferGot(StateContext *cntx) {
    int ret = 0;

    // ginPrivateCfg.data.nwCfg.configStatus = 0;

    lelinkStorageReadPrivateCfg(&ginPrivateCfg);
    if (ginPrivateCfg.csum != crc8(&(ginPrivateCfg.data), sizeof(ginPrivateCfg.data))) {
        ginPrivateCfg.data.nwCfg.configStatus = 0;
    }

    if (0 < ginPrivateCfg.data.nwCfg.configStatus) {
        ret = halDoApConnect(&ginPrivateCfg, sizeof(PrivateCfg));
    } else {
        ret = halDoApConnect(NULL, 0);
    }
    ginConfigStatus = 1;
    LELOG("stateProcSnifferGot ret[%d] ginConfigStatus[%d][%d]", ret, ginConfigStatus, ginPrivateCfg.data.nwCfg.configStatus);
    return ret;
}

static int stateProcApConnecting(StateContext *cntx) {
    int ret = 0;

    if (0 == ginConfigStatus) {
        return -1;
    }

    ret = halDoApConnecting(NULL, 0);
    // TODO: do not reset to sniffer, it should be reset by button.
    // TIMEOUT_BEGIN(45000)
    //     ret = -1;
    //     LELOG("stateProcApConnecting/DHCP timeout ot[%d]", ot);
    //     resetConfigData();
    // TIMEOUT_END

    return ret;
}

static int s_first_heart = 1;
static int stateProcApConnected(StateContext *cntx) {
    int ret = -1;
    int count = 3;

    NodeData node = {0};

    if (0 == ginConfigStatus) {
        return ret;
    }
    
    node.cmdId = LELINK_CMD_CLOUD_GET_TARGET_REQ;
    node.subCmdId = LELINK_SUBCMD_CLOUD_GET_TARGET_REQ;
    if (ginCtxR2R) {
        if (lelinkNwPostCmd(ginCtxR2R, &node)) {
            ret = 1;
        }
    }

    // only for backup
    // LELOG("***** start stateProcApConnected ginPrivateCfg.data.nwCfg.configStatus[%d], ginConfigStatus[%d]", ginPrivateCfg.data.nwCfg.configStatus, ginConfigStatus);
    lelinkStorageReadPrivateCfg(&ginPrivateCfg);
    if (ginPrivateCfg.csum != crc8(&(ginPrivateCfg.data), sizeof(ginPrivateCfg.data))) {
        ginPrivateCfg.data.nwCfg.configStatus = 2;
    }
    // LELOG("***** end stateProcApConnected ginPrivateCfg.data.nwCfg.configStatus[%d], ginConfigStatus[%d]", ginPrivateCfg.data.nwCfg.configStatus, ginConfigStatus);

    if (2 != ginPrivateCfg.data.nwCfg.configStatus && (1 == ginConfigStatus)) {
        char br[32] = {0};
        int ret = 0;
        ret = halGetBroadCastAddr(br, sizeof(br));
            if (0 >= ret) {
            LELOGE("halGetBroadCastAddr error");
            return 0;
        }

        strcpy(node.ndIP, br);
        node.ndPort = NW_SELF_PORT;
        node.cmdId = LELINK_CMD_HELLO_REQ;
        node.subCmdId = LELINK_SUBCMD_HELLO_REQ;
        while (count--) {
            lelinkNwPostCmd(ginCtxR2R, &node);
        }

        ginPrivateCfg.data.nwCfg.configStatus = 2;
        lelinkStorageWritePrivateCfg(&ginPrivateCfg);
    }
    LELOG("stateProcApConnected");
    s_first_heart = 1;
    // ret = halDoApConnected(NULL, 0);
    return ret;
}

static int stateProcCloudLinked(StateContext *cntx) {
    // int ret = 1;
    if (0 == ginConfigStatus) {
        return -1;
    }
    LELOG("stateProcCloudLinked");

    TIMEOUT_BEGIN(8000)
        LELOGW("stateProcCloudLinked timeout");
        return -1;
    TIMEOUT_END

    return 0;
}

static int stateProcCloudAuthed(StateContext *cntx) {
    if (0 == ginConfigStatus) {
        return -1;
    }

    if(s_first_heart) {
        NodeData node = {0};
        node.cmdId = LELINK_CMD_CLOUD_HEARTBEAT_REQ;
        node.subCmdId = LELINK_SUBCMD_CLOUD_HEARTBEAT_REQ;
        if (ginCtxR2R) {
            if (lelinkNwPostCmd(ginCtxR2R, &node)) {
            }
        }
        s_first_heart = 0;
        return 0;
    }
    TIMEOUT_BEGIN(15000)
        NodeData node = {0};
        node.cmdId = LELINK_CMD_CLOUD_HEARTBEAT_REQ;
        node.subCmdId = LELINK_SUBCMD_CLOUD_HEARTBEAT_REQ;
        if (ginCtxR2R) {
            if (lelinkNwPostCmd(ginCtxR2R, &node)) {
            }
        }
    TIMEOUT_END

    return 0;
}

int resetConfigData(void) {

    int ret = 0;
    ret = lelinkStorageReadPrivateCfg(&ginPrivateCfg);
    if (0 <= ret) {
        ginPrivateCfg.data.nwCfg.configStatus = 0;
        ginConfigStatus = 0;
        ginPrivateCfg.data.devCfg.locked = 1;
        ret = lelinkStorageWritePrivateCfg(&ginPrivateCfg);
    }
    return ret; 
}
int lelinkNwPostCmdExt(const void *node) {
    return lelinkNwPostCmd(ginCtxR2R, node);
}
