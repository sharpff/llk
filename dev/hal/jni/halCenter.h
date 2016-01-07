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

extern nativeContext_t gNativeContext;

int initTask(void);

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
