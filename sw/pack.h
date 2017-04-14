#ifndef __PACK_H__
#define __PACK_H__

#ifdef __cplusplus
extern "C"
{
#endif
#include "leconfig.h"
#include "protocol.h"


#define MAGIC_CODE  0x6eb8b86e

#define ENC_PIECES(encBlock, totalLen) \
    (((totalLen) - 1)/encBlock + 1)

#define ENC_SIZE(encBlock, totalLen) \
    (encBlock*ENC_PIECES(encBlock, (totalLen)))

#define PACK_GET_CMD_HEADER(cmdHeader, buf) \
    cmdHeader = (CmdHeader *)((uint8_t *)buf + sizeof(CommonHeader));

#define PACK_GET_PAYLOAD_HEADER(payloadHeader, buf, encBlockCmdHeader) \
    payloadHeader = (PayloadHeader *)((uint8_t *)buf + sizeof(CommonHeader) + ENC_SIZE(encBlockCmdHeader, sizeof(CmdHeader)));

#define PACK_GET_PAYLOAD_BODY(payloadBody, buf, encBlockCmdHeader) \
    payloadBody = ((uint8_t*)buf + sizeof(CommonHeader) + ENC_SIZE(encBlockCmdHeader, sizeof(CmdHeader) + sizeof(PayloadHeader)));

#define PACK_INIT_HEADER(buf, encBlockCmdHeader) \
{        commonHeader = (CommonHeader *)buf; \
        PACK_GET_CMD_HEADER(cmdHeader, buf); \
        PACK_GET_PAYLOAD_HEADER(payloadHeader, buf, encBlockCmdHeader); \
        PACK_GET_PAYLOAD_BODY(payloadBody, buf, encBlockCmdHeader);}


#define PACK_GEN_COMMON_HEADER(commonHeader, et, b, rawSize, encSize) \
{    ((CommonHeader *)(commonHeader))->magic = MAGIC_CODE; \
    ((CommonHeader *)(commonHeader))->ver = getProtocolVer(); \
    ((CommonHeader *)(commonHeader))->len = rawSize; \
    ((CommonHeader *)(commonHeader))->encType = et; \
    ((CommonHeader *)(commonHeader))->encsum = crc8((const uint8_t *)b, encSize);}

#define PACK_GEN_CMD_HEADER(cmdHeader, cmdInfo, b, n, newUUID) \
{    ((CmdHeader *)(cmdHeader))->status = cmdInfo->status; \
    ((CmdHeader *)(cmdHeader))->cmdId = cmdInfo->cmdId; \
    ((CmdHeader *)(cmdHeader))->seqId = cmdInfo->seqId > 0 ? cmdInfo->seqId : genSeqId(); \
    ((CmdHeader *)(cmdHeader))->randID = genRand(); \
    ((CmdHeader *)(cmdHeader))->noAck = cmdInfo->noAck; \
    ((CmdHeader *)(cmdHeader))->reserved = cmdInfo->reserved; \
    ((CmdHeader *)(cmdHeader))->reserved1 = cmdInfo->reserved1; \
    ((CmdHeader *)(cmdHeader))->reserved2 = cmdInfo->reserved2; \
    if (newUUID) { \
        memcpy((void *)(((CmdHeader *)(cmdHeader))->uuid), (void *)newUUID, MAX_UUID); \
    } else { \
        getTerminalUUID(((CmdHeader *)(cmdHeader))->uuid, MAX_UUID); \
    } \
    ((CmdHeader *)(cmdHeader))->encsum = crc8((const uint8_t *)b, n);}

#define PACK_GEN_PAYLOAD(payloadHeader, scId, b, n) \
{    ((PayloadHeader *)(payloadHeader))->subCmdId = scId; \
    ((PayloadHeader *)(payloadHeader))->len = n; \
    memcpy(((uint8_t *)payloadHeader) + sizeof(PayloadHeader), b, n);}

// #define PACK_GEN_NODE()

// #define MAX_COOKIES 16

typedef enum {
    ENC_TYPE_STRATEGY_0, 
    ENC_TYPE_STRATEGY_11 = 11, 
    ENC_TYPE_STRATEGY_12 = 12, 
    ENC_TYPE_STRATEGY_13 = 13 + 2, 
    ENC_TYPE_STRATEGY_14 = 14, 
    ENC_TYPE_STRATEGY_233 = 233, 
    ENC_TYPE_STRATEGY_213 = 213
}ENC_TYPE;


#ifdef LELINK_PACK
#pragma pack(1)
#endif
/* Common Header */
typedef struct {
    uint32_t magic;
    uint16_t ver;
    uint16_t len;
    uint8_t encType;
    uint8_t encsum;
}LELINK_ALIGNED CommonHeader; // 10 bytes

/* CMD Header */
typedef struct {
    CMD_HEADER_INFO_1ST;
    uint8_t encsum;
}LELINK_ALIGNED CmdHeader; // 45 bytes

/* Payload Header */
typedef struct {
    uint16_t subCmdId;
    uint16_t len;
}LELINK_ALIGNED PayloadHeader; // 4 bytes
#ifdef LELINK_PACK
#pragma pack()
#endif


int doUnpack(void *ctx,
    const uint8_t *nw,
    int nwLen,
    uint8_t *protocolBuf,
    int pbLen,
    CmdHeaderInfo *cmdInfo, 
    void *funcGetToken);

int doPack(void *ctx, 
    int encType,
    const CmdHeaderInfo *cmdInfo,
    const uint8_t *protocolBuf, 
    int pbLen, 
    uint8_t *nw, 
    int nwLen);


#ifdef __cplusplus
}
#endif

#endif /* __PACK_H__ */