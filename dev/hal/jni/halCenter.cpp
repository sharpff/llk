/*
 * halCenter.cpp
 *
 *  Created on: Jan 7, 2016
 *      Author: fei
 */

#include <pthread.h>
#include "halCenter.h"
#include "jniLeLink.h"
#include "airconfig_ctrl.h"

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

	lelinkInit();
	gNativeContext.ctxR2R = nwNew(REMOTE_IP, REMOTE_PORT, PORT_ONLY_FOR_VM, 0);
	gNativeContext.ctxQ2A = nwNew(NULL, 0, NW_SELF_PORT, 0);
	if ((ret = pthread_create(&id, NULL, netTaskFun, (void *) &gNativeContext))) {
		return ret;
	}
	return ret;
}

void airConfig(void *ptr, char *json)
{
	int timeout;
	std::string s;
	Json::Value value;
	Json::Reader reader;
	char parambuf[256];
	const char *ssid, *passwd;
	const char *aes = "912EC803B2CE49E4A541068D495AB570";

	s = std::string(static_cast<char *>(json));
	if (!reader.parse(s, value)) {
		LELOGE("airConfig parse error!\n");
		return;
	}
	ssid = value["ssid"].asCString();
	passwd = value["passwd"].asCString();
	timeout = value["timeout"].asInt();

	snprintf(parambuf, sizeof(parambuf), "SSID=%s,PASSWD=%s,AES=%s,TYPE=1,DELAY=10", ssid, passwd, aes);
	LOGI("airConfig: %s\n", parambuf);
	void *context = airconfig_new(parambuf);
//	void *context = airconfig_new("SSID=TP-LINK_JJFA1,PASSWD=987654321,AES=912EC803B2CE49E4A541068D495AB570,TYPE=1,DELAY=10");

	airconfig_do_config(context);
	airconfig_delete(context);
}

void cmdSend(void *ptr, char *json)
{
	std::string s;
	Json::Value value;
	Json::Reader reader;
	NodeData node = { 0 };

	s = std::string(static_cast<char *>(json));
	if (!reader.parse(s, value)) {
		LELOGE("cmdSend parse error!\n");
		return;
	}
	node.ndPort = LOCAL_PORT;
	node.cmdId = value[PJK_CMD].asInt();
	node.subCmdId = value[PJK_SUBCMD].asInt();
	strncpy(node.ndIP, value[PJK_ADDR].asCString(), MAX_IPLEN);
	nwPostCmd(gNativeContext.ctxR2R, &node);
}

static void *netTaskFun(void *data)
{
	LOGI("LeLink Task run...\n");
	while (gNativeContext.runTask)
	{
		doPollingQ2A(gNativeContext.ctxQ2A);
		doPollingR2R(gNativeContext.ctxR2R);
		delayms(1000);
	}
	return NULL;
}

static jstring getJsonCmdHeaderInfo(JNIEnv *env, const CmdHeaderInfo* cmdInfo)
{
	Json::Value root;

	root[PJK_STATUS] = cmdInfo->status;
	root[PJK_ADDR] = cmdInfo->ndIP;
	root[PJK_UUID] = (char *) (cmdInfo->uuid);
	root[PJK_CMD] = cmdInfo->cmdId;
	root[PJK_SUBCMD] = cmdInfo->subCmdId;
	root[PJK_TOKEN] = (char *) (cmdInfo->token);
	root[PJK_SEQID] = cmdInfo->seqId;
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
	com_letv_lelink_LeLink_MSG_TYPE_REMOTERESPOND, getJsonCmdHeaderInfo(env, cmdInfo), c2bytes(env, (const char *) data, len));
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
