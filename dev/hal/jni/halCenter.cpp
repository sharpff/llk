/*
 * halCenter.cpp
 *
 *  Created on: Jan 7, 2016
 *      Author: fei
 */

#include <pthread.h>
#include "halCenter.h"
#include "jniLeLink.h"

nativeContext_t gNativeContext = {
		"lelink v0.1",
		true
};

static void *netTaskFun(void *data);
static jstring getJsonCmdHeaderInfo(JNIEnv *env, const CmdHeaderInfo* cmdInfo);

int initTask(void)
{
	int ret;
	pthread_t id;

	gNativeContext.ctxR2R = nwNew(REMOTE_IP, REMOTE_PORT, PORT_ONLY_FOR_VM, 0);
	gNativeContext.ctxQ2A = nwNew(NULL, 0, NW_SELF_PORT, 0);
	if ((ret = pthread_create(&id, NULL, netTaskFun, (void *) &gNativeContext))) {
		return ret;
	}
	return ret;
}

static void *netTaskFun(void *data)
{
	LELOGW("LeLink Task run...\n");
	while (gNativeContext.runTask)
	{
//		doPollingQ2A(gNativeContext.ctxQ2A);
		doPollingR2R(gNativeContext.ctxR2R);
		delayms(100);
	}
	return NULL;
}

static jstring getJsonCmdHeaderInfo(JNIEnv *env, const CmdHeaderInfo* cmdInfo)
{
	Json::Value root;

	root["cmdId"] = cmdInfo->cmdId;
	root["subCmdId"] = cmdInfo->subCmdId;
	return c2js(env, root.toStyledString().c_str());
}
/*
 * only for C
 */
int halCBLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *data, int len)
{
	JNIEnv *env;
	int ret = 0;

	THREAD_ATTACH(gNativeContext.jvm, env);
	ret = env->CallIntMethod(gNativeContext.obj, gNativeContext.onMessage,
			com_letv_lelink_LeLink_MSG_TYPE_LOCALREQUEST, getJsonCmdHeaderInfo(env, cmdInfo), c2bytes(env, (const char *) data, len));
	THREAD_DETACH(gNativeContext.jvm);
	return ret;
}

void halCBRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *payloadBody, int len)
{
	JNIEnv *env;
	int ret = 0;

	THREAD_ATTACH(gNativeContext.jvm, env);
	ret = env->CallIntMethod(gNativeContext.obj, gNativeContext.onMessage,
			com_letv_lelink_LeLink_MSG_TYPE_REMOTERESPOND, getJsonCmdHeaderInfo(env, cmdInfo), c2bytes(env, (const char *) payloadBody, len));
	THREAD_DETACH(gNativeContext.jvm);
}

int halCBRemoteReq(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *payloadBody, int len)
{
	JNIEnv *env;
	int ret = 0;

	THREAD_ATTACH(gNativeContext.jvm, env);
	ret = env->CallIntMethod(gNativeContext.obj, gNativeContext.onMessage,
			com_letv_lelink_LeLink_MSG_TYPE_REMOTEREQUEST, getJsonCmdHeaderInfo(env, cmdInfo), c2bytes(env, (const char *) payloadBody, len));
	THREAD_DETACH(gNativeContext.jvm);
	return ret;
}

int halCBLocalRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len, char *nw, int nwLenOut)
{
	JNIEnv *env;
	int ret = 0;

	THREAD_ATTACH(gNativeContext.jvm, env);
	ret = env->CallIntMethod(gNativeContext.obj, gNativeContext.onMessage,
			com_letv_lelink_LeLink_MSG_TYPE_LOCALRESPOND, getJsonCmdHeaderInfo(env, cmdInfo), c2bytes(env, (const char *) data, len));
	THREAD_DETACH(gNativeContext.jvm);
	return ret;
}
