/*
 * halCenter.h
 *
 *  Created on: Jan 7, 2016
 *      Author: fei
 */

#ifndef HALCENTER_H_
#define HALCENTER_H_

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

#include <android/log.h>
#define MYLOG_TAG "LELINK"
#define LOGI(fmt,arg...) __android_log_print(ANDROID_LOG_INFO, MYLOG_TAG, fmt, ##arg)

extern nativeContext_t gNativeContext;

int initTask(void);
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
