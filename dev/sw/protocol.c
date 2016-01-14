#include "leconfig.h"
#include "protocol.h"
#include "network.h"
#include "pack.h"
#include "convertor.h"
#include "io.h"

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

// for state
extern int8_t ginStateCloudLinked;
extern int8_t ginStateCloudAuthed;

// static CmdRecord tblCmdType[];

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

static int cbCloudAuthLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen);
static void cbCloudAuthRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen);

static int cbCloudHeartBeatLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen);
static void cbCloudHeartBeatRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen);

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


static int getPackage(void *pCtx, char ipTmp[MAX_IPLEN], uint16_t *port, CmdHeaderInfo *cmdInfo);
static int isNeedDelCB(CACHE_NODE_TYPE *currNode);
static int forEachNodeQ2ARspCB(CACHE_NODE_TYPE *currNode, void *uData);
static int forEachNodeR2RPostSendCB(CACHE_NODE_TYPE *currNode, void *uData);
static int forEachNodeR2RFindTokenByUUID(CACHE_NODE_TYPE *currNode, void *uData);
static int forEachNodeR2RFindTokenIP(CACHE_NODE_TYPE *currNode, void *uData);
static int forEachNodeR2RFindNode(CACHE_NODE_TYPE *currNode, void *uData);
static int isFromRemote(const CommonCtx *ctx, const char *ip, uint16_t len, uint16_t port);
static int isCommingFromItself(const CommonCtx *ctx, const char *ip, uint16_t port);
static int findTokenByUUID(CommonCtx *ctx, const char uuid[MAX_UUID], uint8_t *token, int lenToken);
static int findTokenByIP(CommonCtx *ctx, const char ip[MAX_IPLEN], uint8_t *token, int lenToken);
extern CmdRecord *getCmdRecord(uint32_t cmdId, uint32_t subCmdId);
static int doQ2ARemoteReq(void *ctx,
    const CmdHeaderInfo* cmdInfo, 
    const uint8_t *protocolBuf,
    int pbLen,
    uint8_t *nwBufOut,
    int nwBufLen);
static int doQ2AProcessing(CommonCtx *pCtx, 
    int protocolBufLen, 
    const CmdHeaderInfo *cmdInfo, 
    char ip[MAX_IPLEN], 
    uint16_t port);


static CmdRecord tblCmdType[] = {
    { LELINK_CMD_HELLO_REQ, LELINK_SUBCMD_HELLO_REQ, cbHelloLocalReq, cbHelloRemoteReq },
    { LELINK_CMD_HELLO_RSP, LELINK_SUBCMD_HELLO_RSP, cbHelloRemoteRsp, cbHelloLocalRsp },
    { LELINK_CMD_DISCOVER_REQ, LELINK_SUBCMD_DISCOVER_REQ, cbDiscoverLocalReq, cbDiscoverRemoteReq },
    { LELINK_CMD_DISCOVER_RSP, LELINK_SUBCMD_DISCOVER_RSP, cbDiscoverRemoteRsp, cbDiscoverLocalRsp },
    { LELINK_CMD_CTRL_REQ, LELINK_SUBCMD_CTRL_CMD_REQ, cbCtrlCmdLocalReq, cbCtrlCmdRemoteReq },
    { LELINK_CMD_CTRL_RSP, LELINK_SUBCMD_CTRL_CMD_RSP, cbCtrlCmdRemoteRsp, cbCtrlCmdLocalRsp },
    { LELINK_CMD_CTRL_REQ, LELINK_SUBCMD_CTRL_GET_STATUS_REQ, cbCtrlGetStatusLocalReq, cbCtrlGetStatusRemoteReq },
    { LELINK_CMD_CTRL_RSP, LELINK_SUBCMD_CTRL_GET_STATUS_RSP, cbCtrlGetStatusRemoteRsp, cbCtrlGetStatusLocalRsp },
    { LELINK_CMD_CLOUD_GET_TARGET_REQ, LELINK_SUBCMD_CLOUD_GET_TARGET_REQ, cbCloudGetTargetLocalReq, NULL },
    { LELINK_CMD_CLOUD_GET_TARGET_RSP, LELINK_SUBCMD_CLOUD_GET_TARGET_RSP, cbCloudGetTargetRemoteRsp, NULL },
    { LELINK_CMD_CLOUD_AUTH_REQ, LELINK_SUBCMD_CLOUD_AUTH_REQ, cbCloudAuthLocalReq, NULL },
    { LELINK_CMD_CLOUD_AUTH_RSP, LELINK_SUBCMD_CLOUD_AUTH_RSP, cbCloudAuthRemoteRsp, NULL },
    { LELINK_CMD_CLOUD_HEARTBEAT_REQ, LELINK_SUBCMD_CLOUD_HEARTBEAT_REQ, cbCloudHeartBeatLocalReq, NULL },
    { LELINK_CMD_CLOUD_HEARTBEAT_RSP, LELINK_SUBCMD_CLOUD_HEARTBEAT_RSP, cbCloudHeartBeatRemoteRsp, NULL },
    { LELINK_CMD_CLOUD_MSG_CTRL_C2R_REQ, LELINK_SUBCMD_CLOUD_MSG_CTRL_C2R_REQ, cbCloudMsgCtrlC2RLocalReq, NULL },
    { LELINK_CMD_CLOUD_MSG_CTRL_C2R_RSP, LELINK_SUBCMD_CLOUD_MSG_CTRL_C2R_RSP, cbCloudMsgCtrlC2RRemoteRsp, NULL },
    { LELINK_CMD_CLOUD_MSG_CTRL_R2T_REQ, LELINK_SUBCMD_CLOUD_MSG_CTRL_R2T_REQ, NULL, cbCloudMsgCtrlR2TRemoteReq },
    { LELINK_CMD_CLOUD_MSG_CTRL_R2T_RSP, LELINK_SUBCMD_CLOUD_MSG_CTRL_R2T_RSP, NULL, cbCloudMsgCtrlR2TLocalRsp },
    { LELINK_CMD_CLOUD_REPORT_REQ, LELINK_SUBCMD_CLOUD_REPORT_REQ, cbCloudReportLocalReq, NULL },
    { LELINK_CMD_CLOUD_REPORT_RSP, LELINK_SUBCMD_CLOUD_REPORT_RSP, cbCloudReportRemoteRsp, NULL },


    { 0, 0, 0, 0 }
};

// #define TEST_RSA
// #define TEST_MD5
// #define TEST_AES
// #define TEST_JSON

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
    
    LELOG("lelinkAES encrypting encLen[%d]\r\n", encLen);
    memcpy(iv, (void *)getPreSharedIV(), AES_LEN);
    memcpy(key, (void *)getPreSharedToken(), AES_LEN);
    ret = aes(iv, 
        key, 
        buf,
        &encLen, /* in-len/out-enc size */
        sizeof(buf),
        1);

    LELOG("lelinkAES encrypted [%d] encLen[%d]\r\n", ret, encLen);
    
    memcpy(iv, (void *)getPreSharedIV(), AES_LEN);
    //memcpy(key, getPreSharedToken(), AES_LEN);
    LELOG("lelinkAES decrypting decLen[%d]\r\n", decLen);
    ret = aes(iv, 
        key, 
        buf,
        &decLen, /* in-dec size/outs */
        sizeof(buf),
        0);
    buf[decLen] = '\0';
    
    LELOG("lelinkAES decrypted [%d][%s] decLen[%d]\r\n", ret, buf, decLen);
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
    const char pukkeyDevKind[] = "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDCyQMwvQauI1/PbtQ2FwVTZwDcPcDXI1nEUIvqsz+tlmQzwpCWGUOwHbZF3AVw8b1zvF5nW/UU0aF8z2KKCqtm6gB4jSblbJZDUlvMhASiGnCUGg2lHf3MDtiMFOeGy2XmvBLDLZVP3uU8gDLfTfCqW+JWzTqoEBZrEK5IPQbi+wIDAQAB";
    // ret = rsaEncrypt((const uint8_t *)ginStrPubkeyPem, ginStrPubkeyPemLen, (const uint8_t *)raw, sizeof(raw), encBuf, sizeof(encBuf));
    // LELOG("rsaEncrypted pem len[%d]\r\n", ret);

    // ret = rsaDecrypt((const uint8_t *)ginStrPrikeyPem, ginStrPrikeyPemLen, (const uint8_t *)encBuf, ret, raw, sizeof(raw));
    // LELOG("rsaDecrypted pem [%d] => [%s]\r\n", ret, raw);
    // LELOG("raw is [%s]\r\n", raw);

    // // LELOG("pem prikey[%d], pubkey[%d]\r\n", ginStrPrikeyPemLen, ginStrPubkeyPemLen);

    // ret = rsaEncrypt((const uint8_t *)ginPubkeyGlobalDer, ginPubkeyGlobalDerLen, (const uint8_t *)raw, sizeof(raw), encBuf, sizeof(encBuf));
    // LELOG("rsaEncrypted global len[%d]\r\n", ret);

    // ret = rsaDecrypt((const uint8_t *)ginPrikeyGlobalDer, ginPrikeyGlobalDerLen, (const uint8_t *)encBuf, ret, raw, sizeof(raw));
    // LELOG("rsaDecrypted global [%d] => [%s]\r\n", ret, raw);
    // LELOG("raw is [%s]\r\n", raw);

    ret = rsaEncrypt((const uint8_t *)ginPubkeyDevKindDer, ginPubkeyDevKindDerLen, (const uint8_t *)raw, sizeof(raw), encBuf, sizeof(encBuf));
    LELOG("rsaEncrypted devkind len[%d]\r\n", ret);

    ret = rsaDecrypt((const uint8_t *)ginPrikeyDevKindDer, ginPrikeyDevKindDerLen, (const uint8_t *)encBuf, ret, raw, sizeof(raw));
    LELOG("rsaDecrypted devkind [%d] => [%s]\r\n", ret, raw);
    LELOG("raw is [%s]\r\n", raw);
    LELOG("der prikey[%d], pubkey[%d]\r\n", ginPrikeyGlobalDerLen, ginPubkeyGlobalDerLen);

    // verify 1
    // if( ( f = fopen( filename, "rb" ) ) == NULL )
    // {
    //     LELOG("fopen failed\r\n");
    //     return;
    // }
    // ret = fread( signature, 1, sizeof(signature), f );
    // fclose( f );
    // // ret = getTerminalSignature(signature, sizeof(signature));
    // LELOG("signatrue [%d]\r\n", ret);
    // ret = rsaVerify((const uint8_t *)pubkeyTestVerify, sizeof(pubkeyTestVerify), 
    //     rawTest, strlen(rawTest), signature, ret);
    // LELOG("rsaVerify [%d]\r\n", ret);

    // verify 2
    ret = getTerminalSignature(signature, sizeof(signature));
    LELOG("signatrue [%d]\r\n", ret);
    ret = rsaVerify((const uint8_t *)ginPubkeyDEV1Der, ginPubkeyDEV1DerLen, 
        pukkeyDevKind, strlen(pukkeyDevKind), signature, ret);
    LELOG("rsaVerify [%d]\r\n", ret);

}
#endif

#ifdef TEST_JSON
void testJson(void) {

    // char json[] = "{\"dir\":0,\"IP\":\"1.2.3.4\",\"port\":8888}";
    // char json1[] = "{\"dir\":1,\"IP\":\"1.2.3.4\",\"port\":8888}";
    char status[256] = {0};
    char out[512] = {"{}"};
    // char ip[MAX_IPLEN] = {0};
    // uint16_t port = 0;
    char token[2 + 2*AES_LEN + 1] = {0};
    // int ret = isNeedToRedirect(json, strlen(json), ip, &port);

    // LELOG("isNeedToRedirect[%d] [%s:%d]\r\n", ret, ip, port);

    token[0] = '"';
    getTerminalTokenStr(token + 1, sizeof(token) - 1);
    token[sizeof(token) - 2] = '"';
    getTerminalStatus(status, 256);
    genCompositeJson(out, sizeof(out), 2, 
        "now", status, 
        "token", token);
    LELOG("%s\r\n", out);
}
#endif

#ifdef TEST_MD5
#include "md5Wrapper.h"
void testMD5() {
    int i = 0;
    uint8_t uuid[MAX_UUID] = {"10000100011000510005123456abcdef"};
    uint8_t out[MD5_LEN] = {0};
    md5(uuid, strlen(uuid), out);
    
    LELOG("md5: ");
    for (i = 0; i < MD5_LEN; i++) {
        LEPRINTF("%02x", out[i]);
    }
    LEPRINTF("\r\n");
    
}
#endif

int lelinkInit() {
    int ret = 0;
    ret = halLockInit();
    if (0 != ret) {
        LELOGE("halLockInit ret[%d]\r\n", ret);
        goto failed;
    }

    ret = halAESInit();
    if (0 != ret) {
        LELOGE("halAESInit ret[%d]\r\n", ret);
        goto failed;
    }

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


failed:
    return ret;
}

void lelinkDeinit() {
    
}

int doPollingQ2A(void *ctx) {
    CommonCtx *pCtx = COMM_CTX(ctx);
    char ipTmp[MAX_IPLEN] = {0};
    uint16_t portTmp = 0;
    int ret = 0;
    //int nwLen = 0;
    // int protocolBufLen = UDP_MTU;
    CmdHeaderInfo cmdInfo = {0};

    // do response if postphone
    MUTEX_LOCK;
    qForEachfromCache(&(pCtx->cacheCmd), (int(*)(void*, void*))forEachNodeQ2ARspCB, NULL);
    MUTEX_UNLOCK;

    MUTEX_LOCK;
	qCheckForClean(&(pCtx->cacheCmd), (int(*)(void*))isNeedDelCB);
    MUTEX_UNLOCK;
/*
    if (pCtx->cacheCmd.currsize)
        LELOG("doPollingQ2A cache[%d/%d]\r\n", pCtx->cacheCmd.currsize, pCtx->cacheCmd.maxsize);
*/
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
        // if (pCtx->ctx) {
        //     ret = getPackage(pCtx->ctx, ipTmp, &portTmp, &cmdInfo);
        //     if (0 > ret) {
        //         return;
        //     }
        // } else {
        //     return;
        // }
    }
    // ret = nwUDPRecvfrom(pCtx, pCtx->nwBuf, UDP_MTU, ipTmp, MAX_IPLEN, &portTmp);
    // if (ret <= 0)
    // {
    //     //LELOG("doPollingQ2A nwUDPRecvfrom [%d] QA cache[%d/%d]\r\n", ret, pCtx->cacheCmd.currsize, pCtx->cacheCmd.maxsize);
    //     return;
    // }

    // if (0 > (ret = lelinkUnpack(pCtx, pCtx->nwBuf, ret, pCtx->protocolBuf, sizeof(pCtx->protocolBuf), &cmdInfo)))
    // {
    //     LELOGW("doPollingQ2A lelinkUnpack [%d] [%s:%d]\r\n", ret, ipTmp, portTmp);
    //     return;
    // }


    // TODO: drop/ignore the discovery from itself
    // if (LELINK_CMD_DISCOVER_REQ == cmdInfo.cmdId) {
    if (isCommingFromItself(pCtx, ipTmp, portTmp)) {
        return -3;
    }
    LELOG("doPollingQ2A lelinkUnpack [%d-%d]\r\n", cmdInfo.cmdId, cmdInfo.subCmdId);

    /*
    if (0 == (cmdId & 0x1))
    {
        LELOGW("doPollingQ2A drop cmdId[%d] subCmdId[%d]\r\n", cmdId, subCmdId);
        return;
    }*/

    ret = doQ2AProcessing(pCtx, ret, &cmdInfo, ipTmp, portTmp);


    return ret;
}

int doPollingR2R(void *ctx) {

    CommonCtx *pCtx = COMM_CTX(ctx);
    int len = 0, isCacheEmpty = 0, isRemoteRsp = 0;
    //CACHE_NODE_TYPE *node = NULL;
    //CmdRecord *ct_p = NULL;
    char ipTmp[MAX_IPLEN] = { 0 };
    uint16_t portTmp = 0;
    //CommonHeader *commonHeader = NULL;
    CmdHeaderInfo cmdInfo = {0};

    len = nwUDPRecvfrom(pCtx, pCtx->nwBuf, UDP_MTU, ipTmp, MAX_IPLEN, &portTmp);
    if ((0 < len)) {
        /* RemoteRsp or RemoteReq */
        if (isFromRemote(pCtx, ipTmp, MAX_IPLEN, portTmp)) {
            len = lelinkUnpack(pCtx, pCtx->nwBuf, len, pCtx->protocolBuf, UDP_MTU, &cmdInfo, (void *)findTokenByUUID);
            if (0 <= len) {
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
            len = lelinkUnpack(pCtx, pCtx->nwBuf, len, pCtx->protocolBuf, UDP_MTU, &cmdInfo, (void *)findTokenByIP);
            // start
            // goto HandleRemoteRsp;
            if (0 <= len) {
                isRemoteRsp = 1;
            }
        }

        if (isRemoteRsp) {
            CmdRecord *ct_p = getCmdRecord(cmdInfo.cmdId, cmdInfo.subCmdId);
            if (ct_p && !isCacheEmpty) {
                MUTEX_LOCK;
                if (qForEachfromCache(&(pCtx->cacheCmd), (int(*)(void*, void*))forEachNodeR2RFindNode, (void *)&cmdInfo)) {
                    ct_p->procR2R ?
                    ((CBRemoteRsp) ct_p->procR2R)(pCtx, &cmdInfo, COMM_CTX(pCtx)->protocolBuf, len) : 0;
                }
                MUTEX_UNLOCK;
            }
        }
    }

    /*
     * 2. POST sending
     */
     MUTEX_LOCK;
     isCacheEmpty = qEmptyCache(&(pCtx->cacheCmd));
     MUTEX_UNLOCK;
     if (!isCacheEmpty) {
        // handle post send
        MUTEX_LOCK;
        qForEachfromCache(&(pCtx->cacheCmd), (int(*)(void*, void*))forEachNodeR2RPostSendCB, (void *)len);
        MUTEX_UNLOCK;

        // TODO: do Q2A's response if postphone, BUT it should got R2R's request. so remote RemoteReq should no postphone
        // MUTEX_LOCK;
        // qForEachfromCache(&(pCtx->cacheCmd), (int(*)(void*, void*))forEachNodeQ2ARspCB, NULL);
        // MUTEX_UNLOCK;

        MUTEX_LOCK;
        qCheckForClean(&(pCtx->cacheCmd), (int(*)(void*))isNeedDelCB);
        MUTEX_UNLOCK;
    }

    
    return len;
}

static int doQ2ARemoteReq(void *ctx,
    const CmdHeaderInfo* cmdInfo, 
    const uint8_t *protocolBuf,
    int pbLen,
    uint8_t *nwBufOut,
    int nwBufLen) {

    int ret = -1;
    // CommonCtx *cmdInfo = COMM_CTX(ctx);
    CmdRecord *ct_p;
//    UINT32 tmpType = 0;
    ct_p = getCmdRecord(cmdInfo->cmdId, cmdInfo->subCmdId);
    if (NULL == ct_p)
    {
        return ret;
    }

    // multi response. like the response(S) after a probe OR a online notice(S)
    if (0 == (cmdInfo->cmdId & 0x1))
    {
        // no need to response
        // LELOGE("R2R??? it should not be here\r\n");
        // ct_p->procR2R ?
        //         ((CBRemoteRsp) ct_p->procR2R)(ctx, cmdInfo, protocolBuf, pbLen) :
        //         0;
        return -0xE;
    }
    
    // req from outside
    // note: the data will be sent if CBRemoteReq return true. so to prepare the data in CBLocalRsp
    if (((CBRemoteReq) ct_p->procQ2A) && ((CBRemoteReq) ct_p->procQ2A)(ctx, cmdInfo, protocolBuf, pbLen)) // do sth remote req
    {
        ct_p = getCmdRecord(cmdInfo->cmdId + 1, cmdInfo->subCmdId + 1);
        if (ct_p) {
            // TODO: change cmdInfo to local. [why?]
            // memset((void *)(cmdInfo->uuid), 0, sizeof(cmdInfo->uuid));
            // getTerminalUUID((uint8_t *)(cmdInfo->uuid));
            ((CmdHeaderInfo *)cmdInfo)->cmdId++;
            ((CmdHeaderInfo *)cmdInfo)->subCmdId++;
            ret = ((CBLocalRsp) ct_p->procQ2A)(ctx, cmdInfo, protocolBuf, pbLen, nwBufOut, nwBufLen); // do sth local rsp
            if (0 == ret) {
                LELOGE("CBLocalRsp ret 0, it is impossible\r\n");
                // ret = strlen(DEF_JSON);
                // memcpy(nwBufOut, DEF_JSON, ret);
            }
        }
        else 
            ret = 0;
    }
    return ret;
}


int nwPostCmd(void *ctx, const void *node)
{
    CommonCtx *pCtx = (CommonCtx *)ctx;
    CACHE_NODE_TYPE *node_p = (CACHE_NODE_TYPE *)node;
    if (!pCtx || !node_p)
    {
        return 0;
    }
    
    node_p->pCtx = pCtx;
    node_p->needReq = 1;
    node_p->needRsp = (LELINK_CMD_DISCOVER_REQ == node_p->cmdId) ? 0xFF : 1;
    node_p->randID = genRand();
	node_p->timeStamp = halGetTimeStamp();
    if (!node_p->uuid[0])
        getTerminalUUID(node_p->uuid);

    // LELOG("nwPostCmd cmdId[%d], subCmdId[%d]\r\n", node_p->cmdId, node_p->subCmdId);

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
    // node.passThru = cmdInfo.passThru;
    // node.reserved = cmdInfo.reserved;
    // node.reserved1 = cmdInfo.reserved1;
    // node.reserved2 = cmdInfo.reserved2;
    // memcpy(node.uuid, cmdInfo.uuid, sizeof(node.uuid));

    // // rest part
    // node.pCtx = pCtx;
    // node.timeStamp = halGetTimeStamp();
    // node.timeoutRef = 5;
    // node.needReq = 1;
    // node.needRsp = 0;
    // memcpy(node.ndIP, ipTmp, MAX_IPLEN);
    // node.ndPort = portTmp;


    
    MUTEX_LOCK;
    if (qEnCache(&(pCtx->cacheCmd), (void *)node))
    {
        MUTEX_UNLOCK;
        return 1;
    }
    MUTEX_UNLOCK;

    return 0;
}

static int isFromRemote(const CommonCtx *ctx, const char *ip, uint16_t len, uint16_t port) {
    return 0 == memcmp(ip, ctx->remoteIP, len) ? 1 : 0;
}

static int isCommingFromItself(const CommonCtx *ctx, const char *ip, uint16_t port) {
    // TODO: IMPL
    // test only
    return 0;
    int tmpPort = 0;
    int ret = 0;
    char myIP[MAX_IPLEN] = {0};
    ret = halGetSelfAddr(myIP, MAX_IPLEN, &tmpPort);
    if (ret > 0) {
        if (0 == strncmp(ip, myIP, strlen(ip))) {
            LELOG("isCommingFromItself YES\r\n");
            return 1;
        }
    }
    return 0;
}

static int forEachNodeR2RFindTokenByUUID(CACHE_NODE_TYPE *currNode, void *uData) {
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

    MUTEX_LOCK;
    ret = qForEachfromCache(&(ctx->cacheCmd), (int(*)(void*, void*))forEachNodeR2RFindTokenByUUID, (void *)&ft);
    MUTEX_UNLOCK;
    if (ret) {
        LELOG("BY UUID TOKEN GOT => ");
        for (i = 0; i < lenToken; i++) {
            LEPRINTF("%02x", token[i]);
        }
        LEPRINTF("\r\n");
        return 1;
    }
    return 0;
}

static int forEachNodeR2RFindTokenIP(CACHE_NODE_TYPE *currNode, void *uData) {
    FindToken *ft = (FindToken *)uData;
    if (0 == memcmp(currNode->ndIP, ft->what, MAX_IPLEN)) {
        memcpy(ft->token, currNode->token, ft->lenToken);
        return 1;
    }
    return 0;
}
static int findTokenByIP(CommonCtx *ctx, const char ip[MAX_UUID], uint8_t *token, int lenToken) {

    int ret = 0;
    int i = 0;
    FindToken ft;
    ft.what = (void *)ip;
    ft.token = token;
    ft.lenToken = lenToken;
    MUTEX_LOCK;
    ret = qForEachfromCache(&(ctx->cacheCmd), (int(*)(void*, void*))forEachNodeR2RFindTokenIP, (void *)&ft);
    MUTEX_UNLOCK;
    if (ret) {
        LELOG("BY IP TOKEN GOT => ");
        for (i = 0; i < lenToken; i++) {
            LEPRINTF("%02x", token[i]);
        }
        LEPRINTF("\r\n");
        return 1;
    }
    return 0;
}

static int forEachNodeR2RFindNode(CACHE_NODE_TYPE *currNode, void *uData) {
    CmdHeaderInfo *cmdInfo = (CmdHeaderInfo *)uData;
    if (currNode->seqId == cmdInfo->seqId) {
        if (0 < currNode->needRsp) {
            currNode->needRsp--;
        }
        return 1;
    }
    return 0;
}

static int getPackage(void *pCtx, char ipTmp[MAX_IPLEN], uint16_t *port, CmdHeaderInfo *cmdInfo) {

    int ret;
    // memset(COMM_CTX(pCtx)->nwBuf, 0, sizeof(COMM_CTX(pCtx)->nwBuf));
    // memset(COMM_CTX(pCtx)->protocolBuf, 0, sizeof(COMM_CTX(pCtx)->protocolBuf));
    ret = nwUDPRecvfrom(pCtx, COMM_CTX(pCtx)->nwBuf, UDP_MTU, ipTmp, MAX_IPLEN, port);
    if (0 >= ret)
    {
        // LELOG("doPollingR2R nwUDPRecvfrom [%d] R2R queue[%d/%d] [%d-%d]\r\n", 
        //     ret, COMM_CTX(pCtx)->cacheCmd.currsize, COMM_CTX(pCtx)->cacheCmd.maxsize,
        //     currNode->cmdId, currNode->subCmdId);
        return -4;
    }

    if (0 > (ret = lelinkUnpack(pCtx, COMM_CTX(pCtx)->nwBuf, ret, COMM_CTX(pCtx)->protocolBuf,
        sizeof(COMM_CTX(pCtx)->protocolBuf), cmdInfo, NULL)))
    {
        LELOGW("doPolling lelinkUnpack [%d]\r\n", ret);
        return -5;
    }

    return ret;
}

static int forEachNodeR2RPostSendCB(CACHE_NODE_TYPE *currNode, void *uData)
{
    int len;
    CmdRecord *ct_p;
    //int protocolBufLen = UDP_MTU;
    //uint16_t seqId = 0;
    //int ret = 0;
    char ipTmp[MAX_IPLEN] =
    { 0 };
    int portTmp = 0;
    // only for RemoteRsp
    //int protocolBufLen = (int)uData;

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
            currNode->needRsp = 0;
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
            LELOGE("forEachNodeR2RPostSendCB nwUDPSendto [%d]\r\n", len);
            return -2;
        }

        //currNode->needRsp = 1;
        currNode->needReq = 0;
    }

    return 0;
}

static int forEachNodeQ2ARspCB(CACHE_NODE_TYPE *currNode, void *uData)
{
    USED(uData);

    //char nwBufOut[UDP_MTU] =
    //{ 0 };
    //int protocolBufLenOut = 0;
    //int isReq;
    CmdRecord *ct_p;
    uint8_t nwBuf[UDP_MTU] = { 0 };
    int nwLen = 0, ret;
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
    if (0 < (nwLen = doQ2ARemoteReq(currNode->pCtx, (CmdHeaderInfo *)currNode, NULL, 0, nwBuf, UDP_MTU)))
    {
        // QA send from second time
        ret = nwUDPSendto(currNode->pCtx, currNode->ndIP, currNode->ndPort, nwBuf, nwLen);
        if (0 >= ret)
        {
            LELOGE("forEachNodeQ2ARspCB nwUDPSendto [%d]\r\n", ret);
        }
        currNode->needReq = 0;
        return 0;
    }
    return 0;

}

static int isNeedDelCB(CACHE_NODE_TYPE *currNode)
{
    //return 1;
    // timeout
    if (currNode->timeoutRef && (halGetTimeStamp() - currNode->timeStamp) > currNode->timeoutRef)
    {
        LELOG("isNeedDelCB timeoutRef[%d] cmd[%d][%d]-%d left needRsp[%d] \r\n",
            currNode->timeoutRef, currNode->cmdId, currNode->subCmdId, currNode->seqId, currNode->needRsp);
        return 1;
    }

    // has been sent
    if (!currNode->needRsp && !currNode->needReq)
    {
        return 1;
    }


    return 0;
}

static int doQ2AProcessing(CommonCtx *pCtx, int protocolBufLen, const CmdHeaderInfo *cmdInfo, char ip[MAX_IPLEN], uint16_t port) {
    int ret = 0;
    ret = doQ2ARemoteReq(pCtx, cmdInfo, pCtx->protocolBuf, protocolBufLen, pCtx->nwBuf, UDP_MTU);
    if (ret > 0)
    {
        int nwLen = ret;
        LELOG("doPollingQ2A nwUDPSendto [%s:%d][%d]\r\n", ip, port, nwLen);
        // QA send imediately
        ret = nwUDPSendto(pCtx, ip, port, pCtx->nwBuf, nwLen);
        if (0 >= ret)
        {
            LELOGW("doPollingQ2A nwUDPSendto [%s:%d][%d]\r\n", ip, port, ret);
        }
    }
    else if (0 == ret)
    {
        CACHE_NODE_TYPE node =
        { 0 };

        // CmdHeaderInfo
        memcpy(&node, cmdInfo, sizeof(CmdHeaderInfo));
        node.randID = genRand();
        
        // rest part
        node.pCtx = pCtx;
        node.timeStamp = halGetTimeStamp();
        node.timeoutRef = 30;
        node.needReq = 1;
        node.needRsp = 0;
        memcpy(node.ndIP, ip, MAX_IPLEN);
        node.ndPort = port;

        MUTEX_LOCK;
        qEnCache(&(pCtx->cacheCmd), (void *) &node);
        MUTEX_UNLOCK;
    }
    return ret;
}

/* hello */
static int cbHelloLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen) {
    int ret = 0;
    char helloReq[32] = {"{\"msg\":\"hello\"}"};
    // CommonCtx *pCtx = COMM_CTX(ctx);
    LELOG("cbHelloLocalReq -s\r\n");
    ret = lelinkPack(ctx, ENC_TYPE_STRATEGY_11, cmdInfo, (const uint8_t *)helloReq, strlen(helloReq), dataOut, dataLen);
    LELOG("cbHelloLocalReq [%d] -e\r\n", ret);
    return ret;
}

static void cbHelloRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen) {
    //int ret = 0;
    // CommonCtx *pCtx = COMM_CTX(ctx);
    LELOG("cbHelloRemoteRsp -s\r\n");
    LELOG("[%d][%s]\r\n", dataLen, dataIn);
    LELOG("cbHelloRemoteRsp -e\r\n");
}

/* for sdk, discover a new device */
static int cbHelloRemoteReq(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen) {
    int ret = 0;
    // CommonCtx *pCtx = COMM_CTX(ctx);
    LELOG("cbHelloRemoteReq -s\r\n");
    LELOG("[%d][%s]\r\n", dataLen, dataIn);
    LELOG("cbHelloRemoteReq -e\r\n");
    ret = halCBRemoteReq(ctx, cmdInfo, dataIn, dataLen);
	return ret;
}

static int cbHelloLocalRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len, uint8_t *dataOut, int dataLen) {
    int ret = 0;
    //char rspHello[] = "{ \"msg\":\"i know u got it.\" }";
    // CommonCtx *pCtx = COMM_CTX(ctx);
    LELOG("cbHelloLocalRsp -s\r\n");
    ret = lelinkPack(ctx, ENC_TYPE_STRATEGY_11, cmdInfo, data, ret, dataOut, dataLen);
    LELOG("cbHelloLocalRsp -e\r\n");
    return ret;
}

/* discovery */
static int cbDiscoverLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen) {
    int ret = 0;
    char reqDiscover[48] = {0};
    // CommonCtx *pCtx = COMM_CTX(ctx);
    LELOG("cbDiscoverLocalReq -s\r\n");

    ret = halGetJsonUTC(reqDiscover, sizeof(reqDiscover));
    if (0 >= ret) {
        ret = 0;
    }
    ret = lelinkPack(ctx, ENC_TYPE_STRATEGY_11, cmdInfo, (const uint8_t *)reqDiscover, ret, dataOut, dataLen);
    
    LELOG("cbDiscoverLocalReq [%d] -e\r\n", ret);
    return ret;
}

static void cbDiscoverRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen) {
    //int ret = 0;
    // CommonCtx *pCtx = COMM_CTX(ctx);
    LELOG("cbDiscoverRemoteRsp -s\r\n");
    LELOG("[%d][%s]\r\n", dataLen, dataIn);
    halCBRemoteRsp(ctx, cmdInfo, dataIn, dataLen);
    LELOG("cbDiscoverRemoteRsp -e\r\n");
}

static int cbDiscoverRemoteReq(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen) {
    int ret = 1;
    // CommonCtx *pCtx = COMM_CTX(ctx);
    LELOG("cbDiscoverRemoteReq -s\r\n");
    LELOG("[%d][%s]\r\n", dataLen, dataIn);
    LELOG("cbDiscoverRemoteReq -e\r\n");
    return ret;
}

static int cbDiscoverLocalRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len, uint8_t *dataOut, int dataLen) {
    int ret = 0;
    // CommonCtx *pCtx = COMM_CTX(ctx);
    char rspDiscover[1024] = {0};
    LELOG("cbDiscoverLocalRsp -s\r\n");
    // gen status
    ret = getTerminalStatus(rspDiscover + strlen(rspDiscover), sizeof(rspDiscover) - strlen(rspDiscover));

	ret = lelinkPack(ctx, ENC_TYPE_STRATEGY_11, cmdInfo, (const uint8_t *)rspDiscover, strlen(rspDiscover), dataOut, dataLen);
    LELOG("cbDiscoverLocalRsp -e\r\n");
    return ret;
}

static int cbCtrlGetStatusLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen) {

    int ret = 0;
    // char reqCtrlGetStatus[128];
    // CommonCtx *pCtx = COMM_CTX(ctx);
    LELOG("cbCtrlGetStatusLocalReq -s\r\n");

    // ret = halCBLocalReq(ctx, cmdInfo, reqCtrlGetStatus, sizeof(reqCtrlGetStatus));
	ret = lelinkPack(ctx, ENC_TYPE_STRATEGY_13, cmdInfo, (const uint8_t *)NULL, 0, dataOut, dataLen);
    
    LELOG("cbCtrlGetStatusLocalReq [%d] -e\r\n", ret);
    return ret;
}
static void cbCtrlGetStatusRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen) {
    //int ret = 0;
    // CommonCtx *pCtx = COMM_CTX(ctx);
    LELOG("cbCtrlGetStatusRemoteRsp -s\r\n");
    LELOG("[%d][%s]\r\n", dataLen, dataIn);
    LELOG("cbCtrlGetStatusRemoteRsp -e\r\n");
    return;
}
static int cbCtrlCmdRemoteReq(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen) {
    int ret = 1, type = 0;
    uint8_t data[1024] = {0};
    // CommonCtx *pCtx = COMM_CTX(ctx);
    LELOG("cbCtrlCmdRemoteReq -s\r\n");
    LELOG("[%d][%s]\r\n", dataLen, dataIn);
    LELOG("cbCtrlCmdRemoteReq -e\r\n");
    ret = std2pri((const char *)dataIn, dataLen, data, sizeof(data), &type, NULL);
    if (0 < ret) {
        ret = ioWrite(type, data, ret);
    }

    return 1;
}
static int cbCtrlCmdLocalRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len, uint8_t *dataOut, int dataLen) {
    int ret = 0;
    // CommonCtx *pCtx = COMM_CTX(ctx);
    char rspCtrlCmd[1024] = {0};
    LELOG("cbCtrlCmdLocalRsp -s\r\n");
    ret = getTerminalStatus(rspCtrlCmd, sizeof(rspCtrlCmd));
	ret = lelinkPack(ctx, ENC_TYPE_STRATEGY_13, cmdInfo, (const uint8_t *)rspCtrlCmd, strlen(rspCtrlCmd), dataOut, dataLen);
    LELOG("cbCtrlCmdLocalRsp -e\r\n");
    return ret;
}

static int cbCtrlCmdLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen) {

    int ret = 0;
    char reqCtrlCmd[128];
    // CommonCtx *pCtx = COMM_CTX(ctx);
    LELOG("cbCtrlCmdLocalReq -s\r\n");

    ret = halCBLocalReq(ctx, cmdInfo, reqCtrlCmd, sizeof(reqCtrlCmd));
    ret = lelinkPack(ctx, ENC_TYPE_STRATEGY_13, cmdInfo, (const uint8_t *)reqCtrlCmd, ret, dataOut, dataLen);
    
    LELOG("cbCtrlCmdLocalReq [%d] -e\r\n", ret);
    return ret;
}
static void cbCtrlCmdRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen) {
    //int ret = 0;
    // CommonCtx *pCtx = COMM_CTX(ctx);
    LELOG("cbCtrlCmdRemoteRsp -s\r\n");
    LELOG("[%d][%s]\r\n", dataLen, dataIn);
    halCBRemoteRsp(ctx, cmdInfo, dataIn, dataLen);
    LELOG("cbCtrlCmdRemoteRsp -e\r\n");
    return;
}
static int cbCtrlGetStatusRemoteReq(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen) {
    int ret = 1;
    // CommonCtx *pCtx = COMM_CTX(ctx);
    LELOG("cbCtrlGetStatusRemoteReq -s\r\n");
    // LELOG("[%d][%s]\r\n", dataLen, dataIn);
    LELOG("cbCtrlGetStatusRemoteReq -e\r\n");
    return ret;
}
static int cbCtrlGetStatusLocalRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len, uint8_t *dataOut, int dataLen) {
    int ret = 0, type = 0;
    // CommonCtx *pCtx = COMM_CTX(ctx);
    uint8_t status[1024] = {0};
    LELOG("cbCtrlGetStatusLocalRsp -s\r\n");
    ret = std2pri((const char *)data, len, status, sizeof(status), &type, NULL);
    if (0 < ret) {
        ioWrite(type, status, ret);
        ret = ioRead(type, status, sizeof(status));
        if (0 < ret) {
            ret = pri2std(status, ret, (char *)dataOut, dataLen, type, NULL);
            if (0 < ret) {
            }
        }
    }
    ret = lelinkPack(ctx, ENC_TYPE_STRATEGY_13, cmdInfo, (const uint8_t *)status, ret > 0 ? ret : 0, dataOut, dataLen);
    // ret = getTerminalStatus(binStatus, sizeof(binStatus));
    LELOG("cbCtrlGetStatusLocalRsp -e\r\n");
    return ret;
}

static int cbCloudGetTargetLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen) {
    int ret = 0;
    uint8_t signature[RSA_LEN] = {0};
    int lenSignature = 0;
    // CommonCtx *pCtx = COMM_CTX(ctx);
    // CmdHeaderInfo cmdInfo = {0};
    LELOG("cbCloudGetTargetLocalReq -s\r\n");

    // cmdInfo.status = 0;
    // cmdInfo.cmdId = LELINK_CMD_CLOUD_GET_TARGET_REQ;
    // cmdInfo.subCmdId = LELINK_SUBCMD_CLOUD_GET_TARGET_REQ;
    // cmdInfo.seqId = 0;

    lenSignature = getTerminalSignature(signature, RSA_LEN);
    ret = lelinkPack(ctx, ENC_TYPE_STRATEGY_12, cmdInfo, signature, lenSignature, dataOut, dataLen);
    
    ginStateCloudLinked = ret > 0 ? 1 : 0;

    LELOG("cbCloudGetTargetLocalReq [%d] -e\r\n", ret);
    return ret;
}

static void cbCloudGetTargetRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen) {
    int ret = 0;
    // CommonCtx *pCtx = COMM_CTX(ctx);
    LELOG("cbCloudGetTargetRemoteRsp -s\r\n");
    LELOG("total[%d] json[%s]\r\n", dataLen, dataIn + RSA_LEN);
    char ip[MAX_IPLEN] = {0};
    uint16_t port = 0;
    uint8_t tPubkey[256] = {0};
    int tPubkeyLen = 0;
    tPubkeyLen = getTerminalPublicKey(tPubkey, sizeof(tPubkey));

    if (0 != (ret = rsaVerify(tPubkey, tPubkeyLen, dataIn + RSA_LEN, dataLen - RSA_LEN, dataIn, RSA_LEN))) {
        LELOGW("cbCloudGetTargetRemoteRsp rsaVerify Failed[%d]\r\n", ret);
        return;
    }

    if (isNeedToRedirect(dataIn, dataLen, ip, &port)) {
        CACHE_NODE_TYPE node = { 0 };
        node.cmdId = LELINK_CMD_CLOUD_AUTH_REQ;
        node.subCmdId = LELINK_SUBCMD_CLOUD_AUTH_REQ; 
        memcpy(node.ndIP, ip, MAX_IPLEN);
        node.ndPort = port;
        nwPostCmd(ctx, &node);
        ginStateCloudLinked = 2;
    } else {
        // auth done
        syncUTC(dataIn, dataLen);
        startHeartBeat();
        ginStateCloudLinked = 2;
        ginStateCloudAuthed = 2;
    }

    LELOG("cbCloudGetTargetRemoteRsp -e\r\n");
}


static int cbCloudAuthLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen) {
    int ret = 0;
    uint8_t signature[RSA_LEN] = {0};
    int lenSignature = 0;
    // CommonCtx *pCtx = COMM_CTX(ctx);
    // CmdHeaderInfo cmdInfo = {0};
    LELOG("cbCloudAuthLocalReq -s\r\n");

    // cmdInfo.status = 0;
    // cmdInfo.cmdId = LELINK_CMD_CLOUD_AUTH_REQ;
    // cmdInfo.subCmdId = LELINK_SUBCMD_CLOUD_AUTH_REQ;
    // cmdInfo.seqId = 0;

    lenSignature = getTerminalSignature(signature, RSA_LEN);
    ret = lelinkPack(ctx, ENC_TYPE_STRATEGY_12, cmdInfo, signature, lenSignature, dataOut, dataLen);
    
    ginStateCloudAuthed = ret > 0 ? 1 : 0;

    LELOG("cbCloudAuthLocalReq [%d] -e\r\n", ret);
    return ret;
}

static void cbCloudAuthRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen) {
    //int ret = 0;
    // CommonCtx *pCtx = COMM_CTX(ctx);
    LELOG("cbCloudAuthRemoteRsp -s\r\n");
    LELOG("[%d][%s]\r\n", dataLen, dataIn);

    // auth done
    syncUTC(dataIn, dataLen);
    startHeartBeat();
    ginStateCloudLinked = 2;
    ginStateCloudAuthed = 2;
    LELOG("cbCloudAuthRemoteRsp -e\r\n");
}

static int cbCloudHeartBeatLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen) {
    int ret = 0;
    char token[2 + 2*AES_LEN + 1] = {0};
    char account[128] = {0};
    char status[256] = {0};
    char out[1024] = {"{}"};
    // CommonCtx *pCtx = COMM_CTX(ctx);
    // CmdHeaderInfo cmdInfo = {0};
    LELOG("cbCloudHeartBeatLocalReq -s\r\n");

    // cmdInfo.status = 0;
    // cmdInfo.cmdId = LELINK_CMD_CLOUD_HEARTBEAT_REQ;
    // cmdInfo.subCmdId = LELINK_SUBCMD_CLOUD_HEARTBEAT_REQ;
    // cmdInfo.seqId = 0;
    
    token[0] = '"';
    getTerminalTokenStr(token + 1, sizeof(token) - 1);
    token[sizeof(token) - 2] = '"';
    getTerminalStatus(status, sizeof(status));
    ret = halCBLocalReq(ctx, cmdInfo, account, sizeof(account));
    if (ret > 0) {
        genCompositeJson(out, sizeof(out), 3, 
            "now", status, 
            "token", token, 
            "account", account);
    } else {
        genCompositeJson(out, sizeof(out), 2, 
            "now", status, 
            "token", token);
    }
    LELOG("%s\r\n", out);

    ret = lelinkPack(ctx, ENC_TYPE_STRATEGY_14, cmdInfo, (const uint8_t *)out, strlen(out), dataOut, dataLen);
    
    LELOG("cbCloudHeartBeatLocalReq [%d] -e\r\n", ret);
    return ret;
}

static void cbCloudHeartBeatRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen) {
    //int ret = 0;
    // CommonCtx *pCtx = COMM_CTX(ctx);
    LELOG("cbCloudHeartBeatRemoteRsp -s\r\n");
    LELOG("[%d]\r\n", dataLen);
    LELOG("cbCloudHeartBeatRemoteRsp -e\r\n");
}


static int cbCloudMsgCtrlC2RLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen) {
    int ret = 0;
    // CommonCtx *pCtx = COMM_CTX(ctx);  
    // CmdHeaderInfo tmpCmdInfo;
    char rmtCtrl[256] = {0};
    LELOG("cbCloudMsgCtrlC2RLocalReq -s\r\n");

    // memcpy(&tmpCmdInfo, cmdInfo, sizeof(CmdHeaderInfo));
    // getUUIDFromJson(rmtCtrl, sizeof(getUUIDFromJson), tmpCmdInfo.uuid);

    ret = halCBLocalReq(ctx, cmdInfo, rmtCtrl, sizeof(rmtCtrl));
    ret = lelinkPack(ctx, ENC_TYPE_STRATEGY_233, cmdInfo, (const uint8_t *)rmtCtrl, ret, dataOut, dataLen);

    LELOG("cbCloudMsgCtrlC2RLocalReq -e\r\n");
    return ret;
}

static void cbCloudMsgCtrlC2RRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen) {
    LELOG("cbCloudMsgCtrlC2RRemoteRsp -s\r\n");
    LELOG("[%d][%s]\r\n", dataLen, (char *)dataIn);
    halCBRemoteRsp(ctx, cmdInfo, dataIn, dataLen);
    LELOG("cbCloudMsgCtrlC2RRemoteRsp -e\r\n");
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
    LELOG("cbCloudMsgCtrlR2TRemoteReq -s\r\n");
    LELOG("[%d][%s]\r\n", dataLen, dataIn);
    // setCurrentR2T
    LELOG("cbCloudMsgCtrlR2TRemoteReq -e\r\n");
    return halCBRemoteReq(ctx, cmdInfo, dataIn, dataLen);
}

static int cbCloudMsgCtrlR2TLocalRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len, uint8_t *dataOut, int dataLen) {
    int ret = 0;
    // CommonCtx *pCtx = COMM_CTX(ctx);
    char status[1024] = {0};
    LELOG("cbCloudMsgCtrlR2TLocalRsp -s\r\n");
    ret = halCBLocalRsp(ctx, cmdInfo, data, len, status, sizeof(status));
    if (!ret) {
        ret = getTerminalStatus(status, sizeof(status));
    }
	ret = lelinkPack(ctx, ENC_TYPE_STRATEGY_233, cmdInfo, (const uint8_t *)status, ret, dataOut, dataLen);
    LELOG("cbCloudMsgCtrlR2TLocalRsp -e\r\n");
    return ret;
}

static int cbCloudReportLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *dataOut, int dataLen) {
    int ret = 0;
    // CommonCtx *pCtx = COMM_CTX(ctx);  
    // CmdHeaderInfo tmpCmdInfo;
    char uuid[64] = {0};
    LELOG("cbCloudReportLocalReq -s\r\n");

    // memcpy(&tmpCmdInfo, cmdInfo, sizeof(CmdHeaderInfo));
    // getUUIDFromJson(uuid, sizeof(getUUIDFromJson), tmpCmdInfo.uuid);


    ret = halCBLocalReq(ctx, cmdInfo, uuid, sizeof(uuid));
    ret = lelinkPack(ctx, ENC_TYPE_STRATEGY_14, cmdInfo, (const uint8_t *)uuid, ret, dataOut, dataLen);

    LELOG("cbCloudReportLocalReq -e\r\n");
    return ret;
}

static void cbCloudReportRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *dataIn, int dataLen) {
    LELOG("cbCloudReportRemoteRsp -s\r\n");
    LELOG("[%d][%s]\r\n", dataLen, (char *)dataIn);
    // halCBRemoteRsp(ctx, cmdInfo, dataIn, dataLen);
    LELOG("cbCloudReportRemoteRsp -e\r\n");
    return;
}

CmdRecord *getCmdRecord(uint32_t cmdId, uint32_t subCmdId) {

    int i = 0;
    for (;tblCmdType[i].cmdId; i++)
    {
        if (tblCmdType[i].cmdId == cmdId && tblCmdType[i].subCmdId == subCmdId)
        {
            if (cmdId % 2)
                LELOG("POST getCmdRecord cmdId[%d] subCmdId[%d]\r\n", cmdId, subCmdId);
            return &tblCmdType[i];
        }
    }
    LELOGE("getCmdRecord NULL\r\n");
    return NULL;
}
