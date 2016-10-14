/*
 * halCenter.h
 *
 *  Created on: Jan 7, 2016
 *      Author: fei
 */

#ifndef HALCENTER_H_
#define HALCENTER_H_

// #include <android/log.h>
#include "halHeader.h"
#include "io.h"
#include "leconfig.h"
#include "protocol.h"
#include "jnitls.h"
#include "network.h"

typedef struct _nativeContext {
	const char *version;
	int runTask;
	void *ctxR2R;
	void *ctxQ2A;
	JavaVM *jvm;
	jobject obj;
	jmethodID onMessage;
    AuthCfg authCfg;
    PrivateCfg privateCfg;
    char mac[6];
} nativeContext_t;

#define PORT_ONLY_FOR_VM 0 // (NW_SELF_PORT + 100) // the port for r2r should be 0,

/* function's json key */
#define FJK_TYPE		"type"
#define FJK_SSID		"ssid"
#define FJK_PASSWD		"passwd"
#define FJK_DELAY		"delay" // ms
#define FJK_TIMEOUT		"timeout" // sec
#define FJK_AUTH        "auth"
#define FJK_MAC         "mac"

/* protocol's json key */
#define PJK_STATUS		"status"
#define PJK_ADDR		"addr"
#define PJK_UUID		"uuid"
#define PJK_CMD			"cmdId"
#define PJK_SUBCMD		"subCmdId"
#define PJK_TOKEN		"token"
#define PJK_SEQID		"seqId"

extern nativeContext_t gNativeContext;

int initTask(char *json);
int airConfig(void *ptr, char *json);
int cmdSend(void *ptr, char *json);

#ifdef __cplusplus
extern "C" {
#endif
int halCBLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *data, int len);
void halCBRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *payloadBody, int len);
int halCBRemoteReq(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *payloadBody, int len);
int halCBLocalRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len, char *nw, int nwLenOut);
#ifdef __cplusplus
}
#endif

#endif /* HALCENTER_H_ */
