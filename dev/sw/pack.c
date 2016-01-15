#include "leconfig.h"
#include "pack.h"
#include "protocol.h"
#include "network.h"
#include "data.h"
#include "utility.h"

// #define NO_CRYPTO
typedef int (*FP_FIND_TOKEN)(const CommonCtx *ctx, const void *what, uint8_t *token, int len);
int doUnpack(void *ctx,
    const uint8_t *nw,
    int nwLen,
    uint8_t *protocolBuf,
    int pbLen,
    CmdHeaderInfo *cmdInfo, 
    void *funcFindToken)
{
    //CommonCtx *pCtx = (CommonCtx *)ctx;
    USED(ctx);

    // 1. read the common header
    CommonHeader *commonHeader = NULL;
    CmdHeader *cmdHeader = NULL;
    PayloadHeader *payloadHeader = NULL;
    //uint8_t *payloadBody = NULL;
    int payloadLen = 0;
    static uint8_t tmpBuf[UDP_MTU] = {0};

    memset(tmpBuf, 0, sizeof(tmpBuf));
    if (nwLen + sizeof(CommonHeader) + sizeof(CmdHeader) + sizeof(PayloadHeader) > 
        UDP_MTU) {
        LELOGW("LELINK_ERR_BUFFER_SPACE_ERR [%d] \r\n", LELINK_ERR_BUFFER_SPACE_ERR);
        return LELINK_ERR_BUFFER_SPACE_ERR;
    }

    commonHeader = (CommonHeader *)(nw);
    if (MAGIC_CODE != commonHeader->magic) {
        LELOGW("LELINK_ERR_MAGIC_ERR mismatch \r\n");
        return LELINK_ERR_MAGIC_ERR;
    }
    memcpy(tmpBuf, commonHeader, sizeof(CommonHeader));

    // protocol verion

    // 1st check sum
    cmdHeader = (CmdHeader *)(nw + sizeof(CommonHeader));
    if (commonHeader->encsum != crc8((const uint8_t *)cmdHeader, nwLen - sizeof(CommonHeader))) {
        LELOGW("LELINK_ERR_CHECKSUM_1ST_ERR mismatch \r\n");
        return LELINK_ERR_CHECKSUM_1ST_ERR;
    }
    memcpy(tmpBuf + sizeof(CommonHeader), cmdHeader, nwLen - sizeof(CommonHeader));

    // unpack for cmdHeader & payload, do relink pointer
    switch (commonHeader->encType) {
    case ENC_TYPE_STRATEGY_0: {
            // payloadHeader = (PayloadHeader *)(nw + sizeof(CommonHeader) + sizeof(PayloadHeader));
            PACK_GET_PAYLOAD_HEADER(payloadHeader, tmpBuf, sizeof(CmdHeader));
            if (cmdHeader->encsum != crc8((const uint8_t *)payloadHeader, payloadLen)) {
            // if (cmdHeader->encsum != crc8((const uint8_t *)payloadHeader, sizeof(PayloadHeader) + payloadHeader->len)) {
                LELOGW("LELINK_ERR_CHECKSUM_2ND_ERR [%d] \r\n", LELINK_ERR_CHECKSUM_2ND_ERR);
                return LELINK_ERR_CHECKSUM_2ND_ERR;
            }
        }break;
    case ENC_TYPE_STRATEGY_11:
    case ENC_TYPE_STRATEGY_13: {
            uint32_t encLen = ENC_SIZE(AES_LEN, commonHeader->len + 1);
            int ret;
            uint8_t iv[AES_LEN] = { 0 };
            uint8_t key[AES_LEN] = { 0 };

            // AES decrypt for cmdHeader + payload
            if (ENC_TYPE_STRATEGY_11 == commonHeader->encType) {
                memcpy(iv, (void *)getPreSharedIV(), AES_LEN);
                memcpy(key, (void *)getPreSharedToken(), AES_LEN);
            }
            else if (ENC_TYPE_STRATEGY_13 == commonHeader->encType) {   
                memcpy(iv, (void *)getPreSharedIV(), AES_LEN);
                // if (!cmdInfo->token[0]) {// for RemoteReq 
                //     memcpy(key, (void *)getTerminalToken(), AES_LEN);
                // } else { // for RemoteRsp
                //     memcpy(key, (void *)cmdInfo->token, AES_LEN);
                // }
                if (funcFindToken && cmdInfo->ndIP[0]) { // it must be local & RemoteRsp
                    if (!((FP_FIND_TOKEN)(funcFindToken))(ctx, (const void *)cmdInfo->ndIP, key, AES_LEN)) {
                        // TODO: it maybe timeout.
                        LELOGW("LELINK_ERR_BAD_TOKEN_ERR [%d] \r\n", LELINK_ERR_BAD_TOKEN_ERR);
                        return LELINK_ERR_BAD_TOKEN_ERR;
                    }
                } else {
                    memcpy(key, (void *)getTerminalToken(), AES_LEN);
                }
            }
            {
                int i = 0;
                LEPRINTF("unpack encType [%d] :", commonHeader->encType);
                for (i = 0; i < AES_LEN; i++) {
                    LEPRINTF("%02x", key[i]);
                }
                LEPRINTF("\r\n");
            }
            ret = aes(iv, 
                key, 
                tmpBuf + sizeof(CommonHeader),
                &encLen, /* in-len/out-enc size */
                sizeof(tmpBuf) - sizeof(CommonHeader),
                0);
            if (0 > ret) {
                LELOGW("LELINK_ERR_DEC1_ERR [%d] \r\n", LELINK_ERR_DEC1_ERR);
                return LELINK_ERR_DEC1_ERR;
            }
            // cmdHeader = (CmdHeader *)(tmpBuf + sizeof(CommonHeader));
            PACK_GET_CMD_HEADER(cmdHeader, tmpBuf);
            // payloadHeader = (PayloadHeader *)(tmpBuf + sizeof(CommonHeader) + sizeof(CmdHeader));
            PACK_GET_PAYLOAD_HEADER(payloadHeader, tmpBuf, sizeof(CmdHeader));
            payloadLen = encLen - sizeof(CmdHeader);

            if (cmdHeader->encsum != crc8((const uint8_t *)payloadHeader, payloadLen)) {
            // if (cmdHeader->encsum != crc8((const uint8_t *)payloadHeader, sizeof(PayloadHeader) + payloadHeader->len)) {
                LELOGW("LELINK_ERR_CHECKSUM_2ND_ERR [%d] \r\n", LELINK_ERR_CHECKSUM_2ND_ERR);
                return LELINK_ERR_CHECKSUM_2ND_ERR;
            }
        }break;
    case ENC_TYPE_STRATEGY_233: {
            int encLen = nwLen - sizeof(CommonHeader);
            uint32_t encLenCmdHeader = 0, rawLenCmdHeader = 0;
            uint32_t encLenPayload = 0, rawLenPayload = 0; 
            encLenCmdHeader = rawLenCmdHeader = ENC_SIZE(AES_LEN, sizeof(CmdHeader) + 1);
            encLenPayload = rawLenPayload = encLen - encLenCmdHeader;
            int ret = 0;
            uint8_t iv[AES_LEN] = { 0 };
            uint8_t key[AES_LEN] = { 0 };

            memcpy(iv, (void *)getPreSharedIV(), AES_LEN);
            memcpy(key, (void *)getTerminalToken(), AES_LEN);

            /* 1st
             * always decrypt with the terminal token.
             */
            ret = aes(iv, 
                key, 
                tmpBuf + sizeof(CommonHeader),
                &rawLenCmdHeader, /* in-len/out-enc size */
                encLenPayload,
                0);
            if (0 > ret) {
                LELOGW("LELINK_ERR_DEC1_ERR [%d] \r\n", LELINK_ERR_DEC1_ERR);
                return LELINK_ERR_DEC1_ERR;
            }
            if (rawLenCmdHeader != sizeof(CmdHeader)) {
                LELOGW("LELINK_ERR_ENCINFO_ERR [%d] \r\n", LELINK_ERR_ENCINFO_ERR);
                return LELINK_ERR_ENCINFO_ERR;
            }
            cmdHeader = (CmdHeader *)(tmpBuf + sizeof(CommonHeader));
            if (cmdHeader->encsum != crc8((const uint8_t *)tmpBuf + sizeof(CommonHeader) + encLenCmdHeader, encLenPayload)) {
            // if (cmdHeader->encsum != crc8((const uint8_t *)payloadHeader, sizeof(PayloadHeader) + payloadHeader->len)) {
                LELOGW("LELINK_ERR_CHECKSUM_2ND_ERR [%d] \r\n", LELINK_ERR_CHECKSUM_2ND_ERR);
                return LELINK_ERR_CHECKSUM_2ND_ERR;
            }

            /* 2nd
             * to check the cmdId
             * RemoteReq: decrypt with terminal token
             * RemoteRsp: decrypt with the token should be got from peers 
             */
            //  
            if (cmdHeader->cmdId % 2) { // RemoteReq
                memcpy(key, (void *)getTerminalToken(), AES_LEN);
            } else { // RemoteRsp
                if (funcFindToken && cmdHeader->uuid[0]) {
                    ((FP_FIND_TOKEN)(funcFindToken))(ctx, (const void *)cmdHeader->uuid, key, AES_LEN);
                }
            }

            memcpy(iv, (void *)getPreSharedIV(), AES_LEN);
            ret = aes(iv, 
                key, 
                tmpBuf + sizeof(CommonHeader) + ENC_SIZE(AES_LEN, sizeof(CmdHeader) + 1),
                &encLenPayload, /* in-len/out-enc size */
                sizeof(tmpBuf) - sizeof(CommonHeader) - ENC_SIZE(AES_LEN, sizeof(CmdHeader) + 1),
                0);
            if (0 > ret) {
                LELOGW("LELINK_ERR_DEC2_ERR [%d] \r\n", LELINK_ERR_DEC2_ERR);
                return LELINK_ERR_DEC2_ERR;
            }

            PACK_GET_CMD_HEADER(cmdHeader, tmpBuf);
            PACK_GET_PAYLOAD_HEADER(payloadHeader, tmpBuf, ENC_SIZE(AES_LEN, sizeof(CmdHeader) + 1));
            payloadLen = encLenPayload;
        }break;
    default:
        LELOGW("LELINK_ERR_ENCTYPE_ERR [%d] \r\n", commonHeader->encType);
        return LELINK_ERR_ENCTYPE_ERR;
        break;
    }

    
    // cmdInfo 
    cmdInfo->status = cmdHeader->status;
    cmdInfo->cmdId = cmdHeader->cmdId;
    cmdInfo->seqId = cmdHeader->seqId;
    cmdInfo->randID = cmdHeader->randID;
    cmdInfo->passThru = cmdHeader->passThru;
    cmdInfo->reserved = cmdHeader->reserved;
    cmdInfo->reserved1 = cmdHeader->reserved1;
    cmdInfo->reserved2 = cmdHeader->reserved2;
    memcpy(cmdInfo->uuid, cmdHeader->uuid, sizeof(cmdInfo->uuid));
    cmdInfo->subCmdId = payloadHeader->subCmdId;

    // protocol buf
    if (pbLen < payloadHeader->len) {
        return LELINK_ERR_BUFFER_SPACE_ERR;
    }
    memcpy(protocolBuf, (uint8_t *)payloadHeader + sizeof(PayloadHeader), payloadHeader->len);



    LELOG("nwUnpack[%d] status[%d] cmdId[%u] subCmdId[%u] seqId[%u] randID[%u] \r\n\
        passThru[%d] reserved[%d] uuid[%s]\r\n", 
        payloadHeader->len, 
        cmdInfo->status, 
        cmdInfo->cmdId, 
        cmdInfo->subCmdId, 
        cmdInfo->seqId, 
        cmdInfo->randID, 
        cmdInfo->passThru, 
        cmdInfo->reserved, 
        cmdInfo->uuid);



    // 4. parse the data

    // lelinkUnpack(
    //     ctx,
    //     cmdId, // 
    //     subCmdId,
    //     nw,
    //     nwLen,
    //     protocolBuf,
    //     pbLen,
    //     cmdId,

    //     )
    return payloadHeader->len;
}

int doPack(void *ctx, 
    int encType,
    const CmdHeaderInfo *cmdInfo,
    const uint8_t *protocolBuf, 
    int pbLen, 
    uint8_t *nw, 
    int nwLen) {

    int ret = 0;
    int macro = 0;
    //CommonCtx *pCtx = (CommonCtx *)ctx;
    USED(ctx);
    CommonHeader *commonHeader = NULL;
    CmdHeader *cmdHeader = NULL;
    // CmdHeader *cmdHeader = (CmdHeader *)(nw + sizeof(CommonHeader));
    PayloadHeader *payloadHeader = NULL;
    uint8_t *payloadBody = NULL;
    uint32_t beingEncLen = 0;
    uint32_t encSize1 = 0;
    uint32_t encSize2 = 0;
    uint32_t rawSize1 = 0;
    uint32_t rawSize2 = 0;
    static uint8_t beingEncBuf[UDP_MTU] = {0};

    // body
    switch (encType) {
        case ENC_TYPE_STRATEGY_0: {
            if (nwLen < sizeof(CommonHeader) + sizeof(CmdHeader) + sizeof(PayloadHeader) + pbLen) {
                LELOGW("LELINK_ERR_BUFFER_SPACE_ERR [%d] \r\n", LELINK_ERR_BUFFER_SPACE_ERR);
                return LELINK_ERR_BUFFER_SPACE_ERR;
            }
            // payload
            payloadHeader->len = pbLen;
            payloadHeader->subCmdId = cmdInfo->subCmdId; 
            cmdHeader->encsum = crc8((const uint8_t *)payloadHeader, sizeof(PayloadHeader) + pbLen);
            payloadBody = nw + sizeof(CommonHeader) + sizeof(CmdHeader) + sizeof(PayloadHeader);
            memcpy(payloadBody, protocolBuf, pbLen);
            ret = sizeof(CommonHeader) + sizeof(CmdHeader) + sizeof(PayloadHeader) + pbLen;

            // commnad header
            cmdHeader->status = cmdInfo->status; 
            cmdHeader->cmdId = cmdInfo->cmdId; 
            cmdHeader->seqId = cmdInfo->seqId > 0 ? cmdInfo->seqId : genSeqId(); 
            cmdHeader->randID = genRand();
            cmdHeader->passThru = cmdInfo->passThru; 
            cmdHeader->reserved = cmdInfo->reserved; 
            getTerminalUUID(cmdHeader->uuid, MAX_UUID);


            // commond 
            commonHeader->magic = MAGIC_CODE;
            commonHeader->ver = getProtocolVersion();
            commonHeader->encType = ENC_TYPE_STRATEGY_0;
            commonHeader->len = sizeof(CmdHeader) + sizeof(PayloadHeader) + pbLen;
            commonHeader->encsum = crc8((const uint8_t *)cmdHeader, sizeof(CmdHeader) + sizeof(PayloadHeader) + pbLen);
        }break;
        case ENC_TYPE_STRATEGY_11:
        case ENC_TYPE_STRATEGY_13: {
            uint8_t iv[AES_LEN] = { 0 };
            uint8_t key[AES_LEN] = { 0 };
            rawSize1 = encSize1 = sizeof(CmdHeader) + sizeof(PayloadHeader) + pbLen;
            if (nwLen < sizeof(CommonHeader) + ENC_SIZE(AES_LEN, rawSize1 + 1)) {
                LELOGW("LELINK_ERR_BUFFER_SPACE_ERR [%d] \r\n", LELINK_ERR_BUFFER_SPACE_ERR);
                return LELINK_ERR_BUFFER_SPACE_ERR;
            }
            memset(beingEncBuf, 0, sizeof(beingEncBuf));

            PACK_INIT_HEADER(nw, sizeof(CmdHeader));

            // gen payload
            PACK_GEN_PAYLOAD(&beingEncBuf[sizeof(CmdHeader)], cmdInfo->subCmdId, protocolBuf, pbLen);
            beingEncLen += sizeof(PayloadHeader) + pbLen;

            // gen cmd header
            PACK_GEN_CMD_HEADER(&beingEncBuf, cmdInfo, &beingEncBuf[sizeof(CmdHeader)], (sizeof(PayloadHeader) + pbLen), macro);
            beingEncLen += sizeof(CmdHeader);

            memcpy(iv, (void *)getPreSharedIV(), AES_LEN);
            if (ENC_TYPE_STRATEGY_11 == encType) {
                memcpy(key, (void *)getPreSharedToken(), AES_LEN);
            }
            else if (ENC_TYPE_STRATEGY_13 == encType) {
                if (!cmdInfo->token[0]) {// for RemoteReq 
                    memcpy(key, (void *)getTerminalToken(), AES_LEN);
                } else { // for RemoteRsp
                    memcpy(key, (void *)cmdInfo->token, AES_LEN);
                }
            }

            ret = aes(iv, 
                key, 
                beingEncBuf,
                &encSize1, /* in-len/out-enc size */
                sizeof(beingEncBuf),
                1);
            if (0 > ret) {
                LELOGW("LELINK_ERR_DEC1_ERR [%d] \r\n", LELINK_ERR_DEC1_ERR);
                return LELINK_ERR_DEC1_ERR;
            }
            memcpy(cmdHeader, beingEncBuf, encSize1);

            // gen common header
            PACK_GEN_COMMON_HEADER(commonHeader, encType, cmdHeader, rawSize1, encSize1);
            ret = sizeof(CommonHeader) + encSize1;
            
            {
                int i = 0;
                LEPRINTF("pack encType [%d] :", commonHeader->encType);
                for (i = 0; i < AES_LEN; i++) {
                    LEPRINTF("%02x", key[i]);
                }
                LEPRINTF("\r\n");
            }

        }break;
        case ENC_TYPE_STRATEGY_12:
        case ENC_TYPE_STRATEGY_14: {
            encSize1 = ENC_SIZE(RSA_LEN, sizeof(CmdHeader) + sizeof(PayloadHeader) + pbLen);
            if (nwLen < sizeof(CommonHeader) + encSize1) {
                LELOGW("LELINK_ERR_BUFFER_SPACE_ERR [%d] \r\n", LELINK_ERR_BUFFER_SPACE_ERR);
                return LELINK_ERR_BUFFER_SPACE_ERR;
            }
            memset(beingEncBuf, 0, sizeof(beingEncBuf));

            PACK_INIT_HEADER(nw, sizeof(CmdHeader));

            // gen payload
            PACK_GEN_PAYLOAD(&beingEncBuf[sizeof(CmdHeader)], cmdInfo->subCmdId, protocolBuf, pbLen);
            beingEncLen += sizeof(PayloadHeader) + pbLen;

            // no 1st/cmdHeader crypto

            // gen cmd header
            PACK_GEN_CMD_HEADER(&beingEncBuf, cmdInfo, payloadHeader, (sizeof(PayloadHeader) + pbLen), macro);
            beingEncLen += sizeof(CmdHeader);

            // do encrypt 1nd
            {
                uint8_t pubkey[256] = {0};
                int pubkeyLen = 0;

                if (ENC_TYPE_STRATEGY_12 == encType) {
                    pubkeyLen = getPreSharedPublicKey(pubkey, sizeof(pubkey));           
                } else if (ENC_TYPE_STRATEGY_14 == encType) {
                    pubkeyLen = getTerminalPublicKey(pubkey, sizeof(pubkey));           
                }
                if (0 > pubkeyLen) {
                    LELOGW("LELINK_ERR_BUFFER_SPACE_ERR [%d] \r\n", LELINK_ERR_BUFFER_SPACE_ERR);
                    return LELINK_ERR_BUFFER_SPACE_ERR;
                }
                if (encSize1 != rsaEncrypt(pubkey, pubkeyLen, beingEncBuf, beingEncLen, cmdHeader, nwLen - sizeof(CommonHeader))) {
                    LELOGW("LELINK_ERR_ENC4_ERR [%d] \r\n", LELINK_ERR_ENC4_ERR);
                    return LELINK_ERR_ENC4_ERR;
                }
            }

            // gen common header
            PACK_GEN_COMMON_HEADER(commonHeader, encType, cmdHeader, encSize1, encSize1);
            ret = sizeof(CommonHeader) + encSize1;

            // // test only
            // {
            //     #include "data.h"
            //     uint8_t prikey[] = PRIATE_KEY_GLOBAL_PEM;
            //     int ret;
            //     uint8_t buf[256] = {0};
            //     ret = rsaDecrypt((const uint8_t *)prikey, sizeof(prikey), (const uint8_t *)cmdHeader, encSize1, buf, sizeof(buf));
            //     LELOG("TEST ret[%d]\r\n", ret);
            // }

        }break;
        case ENC_TYPE_STRATEGY_233: {
            uint8_t iv[AES_LEN] = { 0 };
            uint8_t key[AES_LEN] = { 0 };
            rawSize1 = encSize1 = sizeof(CmdHeader);
            rawSize2 = encSize2 = sizeof(PayloadHeader) + pbLen;
            if (nwLen < sizeof(CommonHeader) + ENC_SIZE(AES_LEN, rawSize1 + 1) + ENC_SIZE(AES_LEN, rawSize2 + 1)) {
                LELOGW("LELINK_ERR_BUFFER_SPACE_ERR [%d] \r\n", LELINK_ERR_BUFFER_SPACE_ERR);
                return LELINK_ERR_BUFFER_SPACE_ERR;
            }
            memset(beingEncBuf, 0, sizeof(beingEncBuf));

            PACK_INIT_HEADER(nw, ENC_SIZE(AES_LEN, sizeof(CmdHeader) + 1));

            // gen payload
            PACK_GEN_PAYLOAD(&beingEncBuf[ENC_SIZE(AES_LEN, rawSize1 + 1)], cmdInfo->subCmdId, protocolBuf, pbLen);

            
            /* 2nd
             * Q2A's token in cmdInfo is EMPTY
             * LocalRsp: encrypt with terminal token
             * LocalReq: encrypt with cached/peer token
             */
            if (!cmdInfo->token[0]) { // for LocalRsp
                memcpy(key, (void *)getTerminalToken(), AES_LEN); 
            } else { // for LocalReq
                memcpy(key, (void *)cmdInfo->token, AES_LEN); 
            }
            memcpy(iv, (void *)getPreSharedIV(), AES_LEN);
            ret = aes(iv, 
                key, 
                &beingEncBuf[ENC_SIZE(AES_LEN, rawSize1 + 1)],
                &encSize2, /* in-len/out-enc size */
                ENC_SIZE(AES_LEN, rawSize2 + 1),
                1);
            if (0 > ret) {
                LELOGW("LELINK_ERR_ENC2_ERR [%d] \r\n", LELINK_ERR_ENC2_ERR);
                return LELINK_ERR_ENC2_ERR;
            }
            memcpy(payloadHeader, &beingEncBuf[ENC_SIZE(AES_LEN, rawSize1 + 1)], encSize2);

            // gen cmd header
            PACK_GEN_CMD_HEADER(&beingEncBuf, cmdInfo, &beingEncBuf[ENC_SIZE(AES_LEN, rawSize1 + 1)], encSize2, cmdInfo->uuid);
            beingEncLen += encSize2;


            /* 1st
             * always encrypt with the terminal token.
             */
            memcpy(iv, (void *)getPreSharedIV(), AES_LEN);
            memcpy(key, (void *)getTerminalToken(), AES_LEN); // self token
            ret = aes(iv, 
                key, 
                beingEncBuf,
                &encSize1, /* in-len/out-enc size */
                ENC_SIZE(AES_LEN, rawSize1 + 1),
                1);
            if (0 > ret) {
                LELOGW("LELINK_ERR_ENC1_ERR [%d] \r\n", LELINK_ERR_ENC1_ERR);
                return LELINK_ERR_ENC1_ERR;
            }
            memcpy(cmdHeader, beingEncBuf, encSize1);
            beingEncLen += encSize1;

            // gen common header
            PACK_GEN_COMMON_HEADER(commonHeader, encType, cmdHeader, rawSize1 + rawSize2, encSize1 + encSize2);
            ret = sizeof(CommonHeader) + encSize1 + encSize2;
            // if (nwLen < (sizeof(CommonHeader) + ENC_SIZE(AES_LEN, sizeof(CmdHeader)) + ENC_SIZE(AES_LEN, sizeof(PayloadHeader) + pbLen))) {
            //     return LELINK_ERR_BUFFER_SPACE_ERR;
            // }
            // payloadHeader = (PayloadHeader *)(nw + sizeof(CommonHeader) + ENC_SIZE(AES_LEN, sizeof(CmdHeader)));

            // test only
            // {
            //     int encLen = nwLen - sizeof(CommonHeader);
            //     uint16_t encLenCmdHeader = ENC_SIZE(AES_LEN, sizeof(CmdHeader) + 1);
            //     uint16_t encLenPayload = encLen - encLenCmdHeader;
            //     int ret = 0;
            //     uint8_t iv[AES_LEN] = { 0 };
            //     uint8_t key[AES_LEN] = { 0 };

            //     memcpy(iv, (void *)getPreSharedIV(), AES_LEN);
            //     memcpy(key, (void *)getTerminalToken(), AES_LEN);

            //     /* 1st
            //      * always decrypt with the terminal token.
            //      */
            //      ret = aes(iv, 
            //         key, 
            //         beingEncBuf,
            //         &encSize1, /* in-len/out-enc size */
            //         ENC_SIZE(AES_LEN, rawSize1 + 1),
            //         0);
            //     if (0 > ret) {
            //         return LELINK_ERR_DEC1_ERR;
            //     }

            // }
        }break;

        default:
            return LELINK_ERR_ENCTYPE_ERR;
        break;
    }


    return ret;
}

// static int lelinkPackInt(
//     void *ctx, 
//     int cmdId,
//     int subCmdId, 
//     const uint8_t *in, 
//     uint32_t inLen, 
//     uint8_t *out, 
//     uint32_t outLen) {

//     return 0;
// }

// static int lelinkUnpackInt(
//     void *ctx, 
//     int cmdId,
//     int subCmdId, 
//     const char *nw, 
//     int nwLen, 
//     uint8_t *protocolBuf, 
//     int *pbLen, 
//     uint32_t *cmdId, 
//     uint16_t *subCmdId, 
//     uint16_t *seqId, 
//     const char *peerIP) {

//     return 0;
// }
