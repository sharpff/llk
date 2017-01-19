#include "leconfig.h"
#include "state.h"
#include "protocol.h"
#include "sengine.h"
#include "io.h"
#include "data.h"
#include "cache.h"
#include "sengine.h"
#include "network.h"
#include "utility.h"

/*
 * it is just a test for remote ip(support in genProfile.sh) connection.
 * for standard case, ip shoudl be got from DNS only.
 */

// #define DNS_IP_TEST

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

#define TIMEOUT_BEGIN_SEC(ss, go1st) {\
    static uint32_t tsStart;\
    static uint32_t tsEnd;\
    uint8_t tmpGo1st = go1st; \
    tmpGo1st = go1st ? (cntx->stateIdCurr != cntx->from ? 1 : 0) : 0;\
    if (!tsStart) { \
        tsStart = tsEnd = halGetTimeStamp(); \
    } \
    tsEnd = halGetTimeStamp(); \
    if (tmpGo1st || ss <= (tsEnd - tsStart)) { \
        tsStart = tsEnd;

#define TIMEOUT_ELSE } else {

#define TIMEOUT_END }}

/* for softAp */
// #define SEC2LETICK(x)               ((x) * 1000 / ginMSDelay)
#define WIFI_CFG_BY_MONITOR_TIME    (60 * 3)
#define WIFI_CFG_BY_SOFTAP_TIME     (60 * 3)
static uint8_t wifiConfigByMonitor = 0;
// static uint32_t wifiConfigTime = 0;
static uint32_t wifiConfigTimeout = 0;
extern int softApStart(void);
extern int softApCheck(void);
extern int softApStop(int success);

extern PCACHE sdevCache();
extern SDevNode *sdevArray();


typedef struct {
    StateId stateIdCurr;
    int from;
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
// static int stateProcCloudOnlining(StateContext *cntx);
static int stateProcCloudOnline(StateContext *cntx);

static void uartClear();
static int sdevGetValidChannel(void);

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
    { E_STATE_CLOUD_AUTHED, stateProcCloudAuthed, E_STATE_AP_CONNECTED, stateProcApConnected, E_STATE_CLOUD_ONLINE, stateProcCloudOnline },
    // { E_STATE_CLOUD_ONLINING, stateProcCloudOnlining, E_STATE_AP_CONNECTED, stateProcApConnected, E_STATE_CLOUD_ONLINE, stateProcCloudOnline },
    { E_STATE_CLOUD_ONLINE, stateProcCloudOnline, E_STATE_AP_CONNECTED, stateProcApConnected, E_STATE_NONE, 0 },
    { E_STATE_NONE, 0, E_STATE_NONE, 0, E_STATE_NONE, 0 }
};

int isApConnected(void) {
    return ginPrivateCfg.data.nwCfg.configStatus > 1;
}

int isCloudAuthed(void) {
    return (ginStateCntx.stateIdCurr >= E_STATE_CLOUD_AUTHED);
}

int isCloudOnlined(void) {
    return (ginStateCntx.stateIdCurr >= E_STATE_CLOUD_ONLINE);
}

StateId changeStateId(StateId state) {
    ginStateId = state;
    return ginStateCntx.stateIdCurr; 
}

StateId getStateId(void) { 
    return ginStateId;
}

static int changeState(int direction, StateContext *cntx, int idx) {
    int ret = 0;

    if(ginStateId != E_STATE_NONE) {
        direction = ginStateId - ginStateCntx.stateIdCurr;
        LELOG("Protocol set %d -> %d", ginStateCntx.stateIdCurr, ginStateId);
    }
    ginStateCntx.from = ginStateCntx.stateIdCurr;
    if (0 > direction) {
        if (E_STATE_NONE != ginStateTbl[idx].stateIdPrev) {
            ginStateCntx.stateIdCurr = ginStateTbl[idx].stateIdPrev;
            LELOG("changeState to prev [%d]", ginStateCntx.stateIdCurr);
            ret = direction;
        }
    } else if (0 < direction) {
        if (E_STATE_NONE != ginStateTbl[idx].stateIdNext) {
            ginStateCntx.stateIdCurr = ginStateTbl[idx].stateIdNext;
            LELOG("changeState to next [%d]", ginStateCntx.stateIdCurr);
            ret = direction;
        }
    }
    ginStateId = E_STATE_NONE;
    if(direction) {
        switch(ginStateCntx.stateIdCurr)
        {
            case E_STATE_CONFIGURING:
                sengineQuerySlave(QUERIETYPE_WAITCONFIG);
                setResetLed(RLED_STATE_WIFI);
                break;
            case E_STATE_SNIFFER_GOT:
            //case E_STATE_AP_CONNECTING:
                sengineQuerySlave(QUERIETYPE_CONNECTING);
                setResetLed(RLED_STATE_CONNECTING);
                break;
            case E_STATE_AP_CONNECTED:
                sengineQuerySlave(QUERIETYPE_CONNECTED);
                setResetLed(RLED_STATE_RUNNING);
                break;
            case E_STATE_CLOUD_LINKED:
            //case E_STATE_CLOUD_AUTHED:
                sengineQuerySlave(QUERIETYPE_CLOUD);
                break;
            default:
                break;
        }
    }
    if (ginStateCntx.stateIdCurr == E_STATE_AP_CONNECTED && direction > 0 && sengineHasDevs()) {
        char buf[64] = {0};
        int chnl = sdevGetValidChannel();

        uartClear();
        // get ver
        snprintf(buf, sizeof(buf), "{\"sDevVer\":%d}", 1);
        sengineSetAction(buf, strlen(buf));
        halDelayms(50);
        // set channel
        snprintf(buf, sizeof(buf), "{\"sDevChnl\":%d}", chnl);
        sengineSetAction(buf, strlen(buf));
        halDelayms(50);
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
    ginMSDelay = msDelay;
    for (i = 0; ginStateTbl[i].stateIdCurr != E_STATE_NONE; i++) {
        if (ginStateTbl[i].stateIdCurr == ginStateCntx.stateIdCurr) {
            ret = ginStateTbl[i].fpStateCurr(&ginStateCntx);
            break;
        }
    }

    TIMEOUT_BEGIN(200)
    senginePollingSlave();
    TIMEOUT_END

    if(ginStateCntx.stateIdCurr >= E_STATE_AP_CONNECTED) {
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

    halDelayms(msDelay);
    return changeState(ret, &ginStateCntx, i);
}

static int reloadLatestPassport(void) {
    int ret = 0;
    memset(&ginPrivateCfg, 0, sizeof(ginPrivateCfg));
    if (0 == lelinkStorageReadPrivateCfg(&ginPrivateCfg)) {
        LELOG("reloadLatestPassport status [%d]", ginPrivateCfg.data.nwCfg.configStatus);
        if (ginPrivateCfg.csum == crc8((uint8_t *)&(ginPrivateCfg.data), sizeof(ginPrivateCfg.data))) {
            if (0 < ginPrivateCfg.data.nwCfg.config.ssid_len) {
                LELOG("reloadLatestPassport got old passport [%s:%s]", ginPrivateCfg.data.nwCfg.config.ssid, ginPrivateCfg.data.nwCfg.config.psk);
                ginPrivateCfg.data.nwCfg.configStatus = 1;
                lelinkStorageWritePrivateCfg(&ginPrivateCfg); 
                ret = 1;                   
            }
        }
    }
    return ret;
}

static int stateProcStart(StateContext *cntx) {
    int ret = 0;

    LELOG("stateProcStart -s");
    if (0 == lelinkStorageReadPrivateCfg(&ginPrivateCfg)) {
        LELOG("lelinkStorageReadPrivateCfg configStatus[%d]", ginPrivateCfg.data.nwCfg.configStatus);
        if (ginPrivateCfg.csum == crc8((uint8_t *)&(ginPrivateCfg.data), sizeof(ginPrivateCfg.data))) {
            if (0 < ginPrivateCfg.data.nwCfg.configStatus) {
                ret = ginPrivateCfg.data.nwCfg.configStatus;
                LELOG("stateProcStart ssid[%s] psk[%s]", ginPrivateCfg.data.nwCfg.config.ssid, ginPrivateCfg.data.nwCfg.config.psk);
                ginConfigStatus = 1;
            } 
        }
    }
    LELOG("stateProcStart ********** flagWiFi[%02x] initCfgWiFiMode[%02x], initCfgIfUnBind[%02x]", 
        ginPrivateCfg.data.devCfg.flagWiFi, 
        ginPrivateCfg.data.devCfg.initCfgWiFiMode,
        ginPrivateCfg.data.devCfg.initCfgIfUnBind); 
    if (0 == ret) {
        if(getDevFlag(DEV_FLAG_RESET)) {
            // TODO: wait for mt7687 flash ready?
            halDelayms(500);
            setDevFlag(DEV_FLAG_RESET, 0);
            wifiConfigByMonitor = ginPrivateCfg.data.devCfg.initCfgWiFiMode ? 1 : 0;
        } else {
            wifiConfigByMonitor = !(ginPrivateCfg.data.devCfg.initCfgWiFiMode) ? 1 : 0;
        }
        wifiConfigTimeout = wifiConfigByMonitor ? WIFI_CFG_BY_MONITOR_TIME : WIFI_CFG_BY_SOFTAP_TIME;
        if(wifiConfigByMonitor) {
            ret = halDoConfig(NULL, 0);
            LELOG("configure wifi by monitor(%d)", ret);
        } else {
            ret = !softApStart();
            LELOG("configure wifi by softAp(%d)", ret);
        }
    }
    // LELOG("stateProcStart [%d]", ret);
    LELOG("stateProcStart [%d] -e", ret);
    return ret;
}

static int stateProcConfiguring(StateContext *cntx) {
    int ret = 0;
    if (1 == ginConfigStatus) {
        ret = 1;
    }
    if (0 == ret) {
        // if(wifiConfigTime <= wifiConfigTimeout) {
        //     wifiConfigTime++;
        // }
        // if(wifiConfigTime == wifiConfigTimeout) {
        //     ret = 0;
        //     LELOG("Configure wifi timeout!!!");
        //     wifiConfigByMonitor ?  halStopConfig() : softApStop(0);
        //     setResetLed(RLED_STATE_FREE);
        //     reloadLatestPassport();
        //     halReboot();
        // } else {
        //     ret = wifiConfigByMonitor ? halDoConfiguring(NULL, 0) : !softApCheck();
        // }

        TIMEOUT_BEGIN_SEC(wifiConfigTimeout, 0)
            ret = 0;
            LELOG("Configure wifi timeout!!! start[%d] end[%d]", tsEnd, tsStart);
            wifiConfigByMonitor ?  halStopConfig() : softApStop(0);
            setResetLed(RLED_STATE_FREE);
            if (reloadLatestPassport()) {
                halReboot();
            } else {
                wifiConfigByMonitor ?  halStopConfig() : softApStop(1);
            }
        TIMEOUT_ELSE
            TIMEOUT_BEGIN_SEC(1, 1)
            ret = wifiConfigByMonitor ? halDoConfiguring(NULL, 0) : !softApCheck();
            TIMEOUT_END
        TIMEOUT_END

    }
    return ret;
}

static int stateProcSnifferGot(StateContext *cntx) {
    int ret = 0;

    if (0 == ginConfigStatus) {
		wifiConfigByMonitor ?  halStopConfig() : softApStop(1);
		LELOG("stateProcSnifferGot right");
    }
    // ginPrivateCfg.data.nwCfg.configStatus = 0;
    lelinkStorageReadPrivateCfg(&ginPrivateCfg);
    if (ginPrivateCfg.csum != crc8((uint8_t *)&(ginPrivateCfg.data), sizeof(ginPrivateCfg.data))) {
        ginPrivateCfg.data.nwCfg.configStatus = 0;
    }

    if (0 < ginPrivateCfg.data.nwCfg.configStatus) {
        setSSID((const char *)&(ginPrivateCfg.data.nwCfg.config.ssid), ginPrivateCfg.data.nwCfg.config.ssid_len);
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

    ret = halDoApConnecting(&ginPrivateCfg, sizeof(PrivateCfg));
    // TODO: do not reset to sniffer, it should be reset by button.
    // TIMEOUT_BEGIN(45000)
    //     ret = -1;
    //     LELOG("stateProcApConnecting/DHCP timeout ot[%d]", ot);
    //     resetConfigData();
    // TIMEOUT_END

    return ret;
}

static int stateProcApConnected(StateContext *cntx) {
    int sta = -1;
    int count = 3;
    NodeData node = {0};

    // LELOG("stateProcApConnected");

    if (0 == ginConfigStatus) {
        return sta;
    }
    sta = 0;

    // only for backup
    // LELOG("***** start stateProcApConnected ginPrivateCfg.data.nwCfg.configStatus[%d], ginConfigStatus[%d]", ginPrivateCfg.data.nwCfg.configStatus, ginConfigStatus);
    lelinkStorageReadPrivateCfg(&ginPrivateCfg);
    if (ginPrivateCfg.csum != crc8((uint8_t *)&(ginPrivateCfg.data), sizeof(ginPrivateCfg.data))) {
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
    
    if (ginCtxR2R) {
        int ret = 0;
        char ip[4][32];
        AuthCfg authCfg;
        memset(&node, 0, sizeof(NodeData));
        ret = lelinkStorageReadAuthCfg(&authCfg);
        if ((0 <= ret) && (authCfg.csum == crc8((uint8_t *)&(authCfg.data), sizeof(authCfg.data)))) {
            memset(ip, 0, sizeof(ip));
            TIMEOUT_BEGIN_SEC(1, 1)
            if (!halGetHostByNameNB(authCfg.data.remote, ip, 4*32)) { // dns
                int k = 0;
                for (k = 0; k < 4; k++) {
                    LELOG("DNS OK [%s]", ip[k]);
                }
                memset(COMM_CTX(ginCtxR2R)->remoteIP, 0, MAX_IPLEN);
                strcpy(COMM_CTX(ginCtxR2R)->remoteIP, ip[0]);
                COMM_CTX(ginCtxR2R)->remotePort = authCfg.data.port;
                #ifndef DNS_IP_TEST
                node.cmdId = LELINK_CMD_CLOUD_GET_TARGET_REQ;
                node.subCmdId = LELINK_SUBCMD_CLOUD_GET_TARGET_REQ;
                if (lelinkNwPostCmd(COMM_CTX(ginCtxR2R), &node)) {
                    sta = 1;
                }
                #endif
            } else { // ip
                LELOGE("DNS Failed");
                memset(COMM_CTX(ginCtxR2R)->remoteIP, 0, MAX_IPLEN);
                strcpy(COMM_CTX(ginCtxR2R)->remoteIP, authCfg.data.remote);
                COMM_CTX(ginCtxR2R)->remotePort = authCfg.data.port;
            }
            TIMEOUT_END
            #ifdef DNS_IP_TEST
            LELOG("IP TEST ON ------- to connect[%s:%d]", COMM_CTX(ginCtxR2R)->remoteIP, COMM_CTX(ginCtxR2R)->remotePort);
            // node.cmdId = LELINK_CMD_CLOUD_HEARTBEAT_REQ;
            // node.subCmdId = LELINK_SUBCMD_CLOUD_HEARTBEAT_REQ;
            // node.cmdId = LELINK_CMD_CLOUD_ONLINE_REQ;
            // node.subCmdId = LELINK_SUBCMD_CLOUD_ONLINE_REQ;
            node.cmdId = LELINK_CMD_CLOUD_GET_TARGET_REQ;
            node.subCmdId = LELINK_SUBCMD_CLOUD_GET_TARGET_REQ;
            if (lelinkNwPostCmd(COMM_CTX(ginCtxR2R), &node)) {
                sta = 1;
            }
            #endif
        }
    }
    return sta;
}

static int stateProcCloudLinked(StateContext *cntx) {
    // int ret = 1;
    if (0 == ginConfigStatus) {
        return -1;
    }
    // LELOG("stateProcCloudLinked");

    TIMEOUT_BEGIN_SEC(8, 0)
    // TIMEOUT_BEGIN(8000)
        return -1;
    TIMEOUT_END

    return 0;
}

static int forEachNodeSDevForPostOnlineCB(SDevNode *currNode, void *uData) {
    NodeData *node = (NodeData *)uData;
    if (0x08 == (0x08 & currNode->isSDevInfoDone)) {
        node->reserved = (((void *)currNode - (void *)sdevCache()->pBase)/sdevCache()->singleSize) + 1;
        lelinkNwPostCmd(ginCtxR2R, node);                            
    }
    return 0;
}

// static int stateProcCloudAuthed(StateContext *cntx) {
//     int ret = 0;
//     NodeData node = {0};
//     if (0 == ginConfigStatus) {
//         return -1;
//     }

//     LELOG("stateProcCloudAuthed");
//     ret = 1;

//     return ret ? 1 : 0;
// }

static int stateProcCloudAuthed(StateContext *cntx) {
    if (0 == ginConfigStatus) {
        return -1;
    }

    // LELOG("stateProcCloudAuthed");

    TIMEOUT_BEGIN_SEC(6, 1)
        NodeData node = {0};
        node.cmdId = LELINK_CMD_CLOUD_ONLINE_REQ;
        node.subCmdId = LELINK_SUBCMD_CLOUD_ONLINE_REQ;
        if (ginCtxR2R) {
            lelinkNwPostCmd(ginCtxR2R, &node);
            node.seqId = 0;
            if (sengineHasDevs()) {
                if (sdevArray() && sdevCache()) {
                    qForEachfromCache(sdevCache(), (int(*)(void*, void*))forEachNodeSDevForPostOnlineCB, &node);
                }
            }
        }
    TIMEOUT_END

    return 0;
}

static int stateProcCloudOnline(StateContext *cntx) {
    if (0 == ginConfigStatus) {
        return -1;
    }

    TIMEOUT_BEGIN_SEC(12, 1)
        NodeData node = {0};
        node.cmdId = LELINK_CMD_CLOUD_HEARTBEAT_REQ;
        node.subCmdId = LELINK_SUBCMD_CLOUD_HEARTBEAT_REQ;
        if (ginCtxR2R) {
            lelinkNwPostCmd(ginCtxR2R, &node);
        }
    TIMEOUT_END

    return 0;
}

int resetConfigData(int bussinessOnly) {

    int ret = 0;
    ret = lelinkStorageReadPrivateCfg(&ginPrivateCfg);
    if (0 <= ret) {
        if (!bussinessOnly) {
        // WiFi config info
            ginPrivateCfg.data.nwCfg.configStatus = 0;
            ginConfigStatus = 0;            
        }

        // user info
        if (bussinessOnly || ginPrivateCfg.data.devCfg.initCfgIfUnBind) {
            ginPrivateCfg.data.devCfg.locked = 0;
            ginPrivateCfg.data.iaCfg.num = 0;
            memset(ginPrivateCfg.data.iaCfg.arrIA, 0, sizeof(ginPrivateCfg.data.iaCfg.arrIA));
        }
        ret = lelinkStorageWritePrivateCfg(&ginPrivateCfg);
    }
    if (0 > ret) {
        LELOGE("resetConfigData [%d]", ret);
        return ret;
    }

    if (bussinessOnly || ginPrivateCfg.data.devCfg.initCfgIfUnBind) {
        if (0 > salResetConfigData()) {
            LELOGE("salResetConfigData [%d]", ret);
            return ret;
        }
    }

    return ret; 
}
int lelinkNwPostCmdExt(const void *node) {
    return lelinkNwPostCmd(ginCtxR2R, node);
}

static void uartClear() {

    IOHDL *ioHdl = NULL;
    int x = 0, ret = 0;
    uint8_t tmp = 0;
    ioHdl = ioGetHdlExt();
    LELOG("uartClear START");
    if (NULL == ioHdl) {
        return;
    }

    for (x = 0; x < ioGetHdlCounts(); x++) {
        if (IO_TYPE_UART == ioHdl[x].ioType && ioHdl[x].hdl) {
            do {
                ret = halUartRead(ioHdl[x].hdl, &tmp, sizeof(tmp));
                LELOG("uartClear Cleaning");
            } while (0 < ret);
            break;
        }
    }
    LELOG("uartClear END");
}

static int sdevGetValidChannel(void) {
    int16_t d11, d25;
    int channel, ch = 11;
    static const int16_t zbch[] = {2405, 2475}; // 11, 25
    static const int16_t wifich[] = {0, 2412, 2417, 2422, 2427, 2432, 2437, 2442, 2447, 2452, 2457, 2462}; // 0, 1 - 11
    if(ginStateCntx.stateIdCurr >= E_STATE_AP_CONNECTED) {
        channel = halGetWifiChannel();
        if(channel > 0 && channel < 12) {
            d11 = wifich[channel] - zbch[0];
            d25 = zbch[1] - wifich[channel];
            ch = (d11 > d25) ? 11 : 25;
        }
    }
    return ch;
}

extern void postReboot();
void reboot(int isAsync) {
    if (isAsync) {
        postReboot();
    } else {
        halReboot();
    }
}
