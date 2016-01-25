/*
 * halCenter.h
 *
 *  Created on: Jan 7, 2016
 *      Author: fei
 */

#ifndef HALCENTER_H_
#define HALCENTER_H_

#include <android/log.h>
#include "json.h"
#include "leconfig.h"
#include "protocol.h"
#include "jnitls.h"
#include "network.h"

typedef struct _nativeContext {
	const char *version;
	bool runTask;
	void *ctxR2R;
	void *ctxQ2A;
	JavaVM *jvm;
	jobject obj;
	jmethodID onMessage;
} nativeContext_t;

#define PORT_ONLY_FOR_VM 0 // (NW_SELF_PORT + 100) // the port for r2r should be 0,
#define MYLOG_TAG "LELINK"
#define LOGI(fmt,arg...) __android_log_print(ANDROID_LOG_INFO, MYLOG_TAG, fmt, ##arg)

/* function's json key */
#define FJK_TYPE		"type"
#define FJK_SSID		"ssid"
#define FJK_PASSWD		"passwd"
#define FJK_DELAY		"delay" // ms
#define FJK_TIMEOUT		"timeout" // sec
#define FJK_PUBLIC_KEY	"public_key"
#define FJK_SIGNATURE	"signature"

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
void airConfig(void *ptr, char *json);
void cmdSend(void *ptr, char *json);

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
