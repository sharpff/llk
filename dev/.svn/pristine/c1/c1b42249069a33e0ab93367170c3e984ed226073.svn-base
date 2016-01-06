#include "leconfig.h"
#include "state.h"
#include "protocol.h"

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

// 0. nothing, 1. req done, 2. rsp done
int8_t ginStateCloudLinked;
int8_t ginStateCloudAuthed;

static int stateProcStart(StateContext *cntx);
static int stateProcConfiguring(StateContext *cntx);
static int stateProcSnifferGot(StateContext *cntx);
static int stateProcApConnecting(StateContext *cntx);
static int stateProcApConnected(StateContext *cntx);
static int stateProcCloudLinked(StateContext *cntx);
static int stateProcCloudAuthed(StateContext *cntx);
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
    { E_STATE_AP_CONNECTING, stateProcApConnecting, E_STATE_SNIFFER_GOT, stateProcSnifferGot, E_STATE_AP_CONNECTED, stateProcApConnected },
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


int pollingState(int msDelay, void *r2r, void *q2a) {
    int i = 0, ret = 0;
    ginCtxQ2A = q2a;
    ginCtxR2R = r2r;
    for (i = 0; ginStateTbl[i].stateIdCurr != E_STATE_NONE; i++) {
        if (ginStateTbl[i].stateIdCurr == ginStateCntx.stateIdCurr) {
            ret = ginStateTbl[i].fpStateCurr(&ginStateCntx);
            break;
        }
    }
    if (0 < msDelay) {
        delayms(msDelay);
    }
    return changeState(ret, &ginStateCntx, i);
}



static int stateProcStart(StateContext *cntx) {
    int ret = 0;
    ret = halDoConfig(NULL, 0);
    // LELOG("stateProcStart [%d]\r\n", ret);
    return ret;
}

static int stateProcConfiguring(StateContext *cntx) {
    int ret;
    ret = halDoConfiguring(NULL, 0);
    return ret;
}

static int stateProcSnifferGot(StateContext *cntx) {
    int ret = 0;
    ret = halDoApConnect(NULL, 0);
    // LELOG("stateProcSnifferGot [%d]\r\n", ret);
    return ret;
}

static int stateProcApConnecting(StateContext *cntx) {
    int ret = 0;
    ret = halDoApConnecting(NULL, 0);
    // LELOG("stateProcSnifferGot [%d]\r\n", ret);
    return ret;
}

static int stateProcApConnected(StateContext *cntx) {
    // int ret = 0;
    // static int flag = 0;
    if (0 == ginStateCloudLinked) {
        NodeData node = {0};
        node.timeStamp = halGetTimeStamp();
        node.timeoutRef = 3;
        node.cmdId = LELINK_CMD_CLOUD_GET_TARGET_REQ;
        node.subCmdId = LELINK_SUBCMD_CLOUD_GET_TARGET_REQ;
        if (ginCtxR2R) {
            if (nwPostCmd(ginCtxR2R, &node)) {
                ginStateCloudLinked = 1;
            }
        }
    }
	LELOG("stateProcApConnected ginStateCloudLinked[%d]\r\n", ginStateCloudLinked);
    // ret = halDoApConnected(NULL, 0);
    return (2 == ginStateCloudLinked) ? 1 : 0;
}

static int stateProcCloudLinked(StateContext *cntx) {
    int ret = 1;
	// LELOG("stateProcCloudLinked [%d]\r\n", ret);
    // ret = halDoCloudLinked(NULL, 0);
    return ret;
}
static int stateProcCloudAuthed(StateContext *cntx) {
    int ret = 1;
	LELOG("stateProcCloudAuthed [%d]\r\n", ret);
    // ret = halDoCloudAuthed(NULL, 0);
    return ret;
}
