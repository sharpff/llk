/*
 * halCenter.cpp
 *
 *  Created on: Jan 7, 2016
 *      Author: fei
 */

#include <pthread.h>
#include "version.h"
#include "json.h"
#include "state.h"
#include "base64.h"
#include "utility.h"
#include "leconfig.h"
#include "halHeader.h"
#include "jniLeLink.h"
#include "halCenter.h"
#include "airconfig_ctrl.h"

nativeContext_t gNativeContext = {
    SW_VERSION __DATE__ " " __TIME__,
    true
};

static void *netTaskFun(void *data);
static jstring getJsonCmdHeaderInfo(JNIEnv *env, const CmdHeaderInfo* cmdInfo);

int initTask(char *json)
{
	int ret;
	pthread_t id;
    AuthCfg *authCfg = &gNativeContext.authCfg;

    { // 初始得到 public_key, signatrue, uuid. server_addr, server_port
        std::string s;
        Json::Value value;
        Json::Reader reader;

        s = std::string(static_cast<char *>(json));
        if (!reader.parse(s, value)) {
            LELOGE("Failed to parse json!\n");
            return -1;
        }
        //	LOGI("info:\n%s", json);
        // public key
        s = base64_decode(value[FJK_PUBLIC_KEY].asString());
        authCfg->data.pubkeyLen = s.length();
        memcpy(authCfg->data.pubkey, s.c_str(), s.length());
        // signature
        s = base64_decode(value[FJK_SIGNATURE].asString());
        authCfg->data.signatureLen = s.length();
        memcpy(authCfg->data.signature, s.c_str(), s.length());
        // uuid
        strncpy((char *) authCfg->data.uuid, value[PJK_UUID].asCString(), MAX_UUID);
        // server addr
        strncpy(authCfg->data.remote, "10.204.28.134", MAX_REMOTE);
        // server port
        authCfg->data.port = 5546;
        authCfg->csum = crc8((uint8_t *)(&authCfg->data), sizeof(authCfg->data));
    }
    if((ret = lelinkStorageInit(0x1C2000, 0x3E000, 0x1000))) {
        LELOGE("Fialed to lelinkStorageInit\n");
        return -1;
    }
	lelinkInit();
	//lelinkInit(&gNativeContext.auth);
	gNativeContext.ctxR2R = lelinkNwNew(authCfg->data.remote, authCfg->data.port, PORT_ONLY_FOR_VM, 0);
	gNativeContext.ctxQ2A = lelinkNwNew(NULL, 0, NW_SELF_PORT, 0);
	if ((ret = pthread_create(&id, NULL, netTaskFun, (void *) &gNativeContext))) {
		return ret;
	}
	return ret;
}

void airConfig(void *ptr, char *json)
{
	std::string s;
	Json::Value value;
	Json::Reader reader;
	int delay, type;
	char parambuf[256];
	const char *ssid, *passwd;
	const char *aes = "912EC803B2CE49E4A541068D495AB570";

	s = std::string(static_cast<char *>(json));
	if (!reader.parse(s, value)) {
		LELOGE("airConfig parse error!\n");
		return;
	}
	type = value[FJK_TYPE].asInt();
	delay = value[FJK_DELAY].asInt();
	ssid = value[FJK_SSID].asCString();
	passwd = value[FJK_PASSWD].asCString();

	snprintf(parambuf, sizeof(parambuf), "SSID=%s,PASSWD=%s,AES=%s,TYPE=%d,DELAY=%d",
			ssid, passwd, aes, type, delay);
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
	node.timeoutRef = value[FJK_TIMEOUT].asInt();
	node.cmdId = value[PJK_CMD].asInt();
	node.subCmdId = value[PJK_SUBCMD].asInt();
	node.seqId = value[PJK_SEQID].asInt();
	if (!value[PJK_ADDR].isNull()) {
		node.ndPort = LOCAL_PORT;
		strncpy(node.ndIP, value[PJK_ADDR].asCString(), MAX_IPLEN);
	}
	if (!value[PJK_UUID].isNull()) {
		strncpy((char *) node.uuid, value[PJK_UUID].asCString(), MAX_UUID);
	}
	if (!value[PJK_TOKEN].isNull()) {
		LOGI("token:[%s]\r\n", value[PJK_TOKEN].asCString());
		hexStr2bytes(value[PJK_TOKEN].asCString(), node.token, AES_LEN);
	}
	LOGI("send: %s.%d %d.%d-%d, timeout: %d\n",
			node.ndIP[0] ? node.ndIP : "default", node.ndPort, node.cmdId, node.subCmdId, node.seqId, node.timeoutRef);
	lelinkNwPostCmd(gNativeContext.ctxR2R, &node);
}

static void *netTaskFun(void *data)
{
	LOGI("LeLink Task run...\n");
	while (gNativeContext.runTask)
	{
        // lelinkPollingState(200, gNativeContext.ctxR2R, gNativeContext.ctxQ2A);
        lelinkDoPollingQ2A(gNativeContext.ctxQ2A);
        lelinkDoPollingR2R(gNativeContext.ctxR2R);
	}
    lelinkNwDelete(gNativeContext.ctxQ2A);
    lelinkNwDelete(gNativeContext.ctxR2R);
	return NULL;
}

static jstring getJsonCmdHeaderInfo(JNIEnv *env, const CmdHeaderInfo* cmdInfo)
{
	Json::Value root;
	std::string base64Token;

	root[PJK_CMD] = cmdInfo->cmdId;
	root[PJK_SUBCMD] = cmdInfo->subCmdId;
	root[PJK_ADDR] = cmdInfo->ndIP;
	root[PJK_UUID] = (char *) (cmdInfo->uuid);
	root[PJK_SEQID] = cmdInfo->seqId;
	root[PJK_STATUS] = cmdInfo->status;
	if (cmdInfo->token[0]) {
		char hexbuf[AES_LEN * 2 + 1] = { 0 };
		bytes2hexStr(cmdInfo->token, AES_LEN, (uint8_t *) hexbuf, sizeof(hexbuf));
		root[PJK_TOKEN] = (char *) (hexbuf);
	}
	return c2js(env, root.toStyledString().c_str());
}
/*
 * only for C
 */
int halCBLocalReq(void *ctx, const CmdHeaderInfo* cmdInfo, uint8_t *data, int len)
{
	int ret = 0;
	JNIEnv *env;
	jbyte *jdata;
	jbyteArray jbytes;

	THREAD_ATTACH(gNativeContext.jvm, env);
	jbytes = c2bytes(env, (const char *) data, len);
	ret = env->CallIntMethod(gNativeContext.obj, gNativeContext.onMessage,
	com_letv_lelink_LeLink_MSG_TYPE_LOCALREQUEST, getJsonCmdHeaderInfo(env, cmdInfo), jbytes);
	if (ret > 0 && ret < len) {
		jdata = env->GetByteArrayElements(jbytes, NULL);
		memcpy(data, jdata, ret);
		env->ReleaseByteArrayElements(jbytes, jdata, JNI_ABORT);
		data[ret++] = '\0';
	}
	THREAD_DETACH(gNativeContext.jvm);
	return ret;
}

void halCBRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len)
{
	int ret = 0;
	JNIEnv *env;

	THREAD_ATTACH(gNativeContext.jvm, env);
	ret = env->CallIntMethod(gNativeContext.obj, gNativeContext.onMessage,
	com_letv_lelink_LeLink_MSG_TYPE_REMOTERESPOND, getJsonCmdHeaderInfo(env, cmdInfo), c2bytes(env, (const char *) data, len));
	THREAD_DETACH(gNativeContext.jvm);
}

int halCBRemoteReq(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len)
{
	int ret = 0;
	JNIEnv *env;

	THREAD_ATTACH(gNativeContext.jvm, env);
	ret = env->CallIntMethod(gNativeContext.obj, gNativeContext.onMessage,
	com_letv_lelink_LeLink_MSG_TYPE_REMOTEREQUEST, getJsonCmdHeaderInfo(env, cmdInfo), c2bytes(env, (const char *) data, len));
	THREAD_DETACH(gNativeContext.jvm);
	return ret;
}

int halCBLocalRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len, char *nw, int nwLenOut)
{
	int ret = 0;
	JNIEnv *env;
	jbyte *jdata;
	jbyteArray jbytes;

	THREAD_ATTACH(gNativeContext.jvm, env);
	jbytes = c2bytes(env, (const char *) data, len > nwLenOut ? len : nwLenOut);
	ret = env->CallIntMethod(gNativeContext.obj, gNativeContext.onMessage,
	com_letv_lelink_LeLink_MSG_TYPE_LOCALRESPOND, getJsonCmdHeaderInfo(env, cmdInfo), jbytes);
	if (ret > 0 && ret < len) {
		jdata = env->GetByteArrayElements(jbytes, NULL);
		memcpy(nw, jdata, ret);
		env->ReleaseByteArrayElements(jbytes, jdata, JNI_ABORT);
		nw[ret++] = '\0';
	}
	THREAD_DETACH(gNativeContext.jvm);
	return ret;
}

// because of llex.c:224
extern "C" char android_getlocaledecpoint()
{
    return '.';
}

