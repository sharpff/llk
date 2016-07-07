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

extern "C" {
    int getTerminalUUID(uint8_t *uuid, int len);
    int softApDoConfig(const char *ssid, const char *passwd, unsigned int timeout);
};

nativeContext_t gNativeContext = {
    SW_VERSION,
    true
};

static void *netTaskFun(void *data);
static jstring getJsonCmdHeaderInfo(JNIEnv *env, const CmdHeaderInfo* cmdInfo);

int initTask(char *json)
{
	pthread_t id;
	Json::Value value;
	Json::Reader reader;
	int i, ret, authLen;
    std::string s, s1, s2;
    AuthCfg *authCfg = &(gNativeContext.authCfg);

    s = std::string(static_cast<char *>(json));
	if (!reader.parse(s, value)) {
		LELOGE("initTask parse error!\n");
		return -1;
	}
    s1 = value[FJK_AUTH].asString();
    s2 = value[FJK_MAC].asString();
    if(s1.length() <= 0 || s2.length() <= 0) {
		LELOGE("initTask parameter error!\n");
        return -1;
    }
    { // AuthCfg, 初始得到 public_key, signatrue, uuid. server_addr, server_port
        const char *p;
        char *mac = gNativeContext.mac;

        s1 = base64_decode(s1);
        memcpy(authCfg, s1.c_str(), s1.length());
        p = s2.c_str();
        for(i = 0; i < 6; i++) {
            mac[i] = strtol(p, (char **)&p, 16);
            p++;
        }
    }
    { // PrivateCfg 
        PrivateCfg *privateCfg = &gNativeContext.privateCfg;
        privateCfg->data.nwCfg.configStatus = 2;
        privateCfg->csum = crc8((uint8_t *)(&privateCfg->data), sizeof(privateCfg->data));
    }
    if((ret = lelinkStorageInit(0x1C2000, 0x3E000, 0x1000))) {
        LELOGE("Fialed to lelinkStorageInit\n");
        return -1;
    }
    if((ret = lelinkInit()) < 0) {
        LELOGE("Fialed to lelinkInit\n");
        return ret;
    }
    getTerminalUUID(authCfg->data.uuid, MAX_UUID);
	gNativeContext.ctxR2R = lelinkNwNew(authCfg->data.remote, authCfg->data.port, PORT_ONLY_FOR_VM, 0);
	gNativeContext.ctxQ2A = lelinkNwNew(NULL, 0, NW_SELF_PORT, 0);
	if ((ret = pthread_create(&id, NULL, netTaskFun, (void *) &gNativeContext))) {
        LELOGE("Fialed to pthread_create\n");
		return ret;
	}
	return ret;
}

int airConfig(void *ptr, char *json)
{
    int ret = 0;
	std::string s;
	Json::Value value;
	Json::Reader reader;
	int delay, type;
    char configInfo[256] = {0};
	const char *ssid, *passwd;
	const char *aes = "912EC803B2CE49E4A541068D495AB570";
    const char *configFmt = "SSID=%s,PASSWD=%s,AES=%s,TYPE=%d,DELAY=%d";

	s = std::string(static_cast<char *>(json));
	if (!reader.parse(s, value)) {
		LELOGE("airConfig parse error!\n");
		return -1;
	}
	type = value[FJK_TYPE].asInt();
	delay = value[FJK_DELAY].asInt();
	ssid = value[FJK_SSID].asCString();
	passwd = value[FJK_PASSWD].asCString();

    if(type < 3) {
        sprintf(configInfo, configFmt, ssid, passwd, "912EC803B2CE49E4A541068D495AB570", type, delay);
        ret = lelinkDoConfig(configInfo);
        if (0 > ret) {
            APPLOGW("waiting ...\n%s", configInfo);
            delayMS(1000);
        } else {
            APPLOGW("ending with [%s:%s][%d] type[%d]...", ssid, passwd, delay, type);
            type = type == 1 ? 2 : 1;
            APPLOGW("starting with [%s:%s][%d] type[%d]...", ssid, passwd, delay, type);
        }
    } else {
        softApDoConfig(ssid, passwd, delay);
    }
    return ret;
}

int cmdSend(void *ptr, char *json)
{
	std::string s;
	Json::Value value;
	Json::Reader reader;
	NodeData node = { 0 };

	s = std::string(static_cast<char *>(json));
	if (!reader.parse(s, value)) {
		LELOGE("cmdSend parse error!\n");
		return -1;
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
		APPLOG("token:[%s]n", value[PJK_TOKEN].asCString());
		hexStr2bytes(value[PJK_TOKEN].asCString(), node.token, AES_LEN);
	}
	APPLOG("send: %s.%d %d.%d-%d, timeout: %d",
			node.ndIP[0] ? node.ndIP : "default", node.ndPort, node.cmdId, node.subCmdId, node.seqId, node.timeoutRef);
	return lelinkNwPostCmd(gNativeContext.ctxR2R, &node);
}

static void *netTaskFun(void *data)
{
	APPLOG("LeLink Task run...");
	while (gNativeContext.runTask)
	{
        lelinkPollingState(200, gNativeContext.ctxR2R, gNativeContext.ctxQ2A);
        //lelinkDoPollingQ2A(gNativeContext.ctxQ2A);
        //lelinkDoPollingR2R(gNativeContext.ctxR2R);
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
	root[PJK_UUID] = std::string((char *) (cmdInfo->uuid), 0, MAX_UUID);
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
        if(cmdInfo->cmdId == LELINK_CMD_CLOUD_MSG_CTRL_C2R_REQ && cmdInfo->subCmdId == LELINK_SUBCMD_CLOUD_MSG_CTRL_C2R_DO_OTA_REQ) {
            data += RSA_LEN;
        }
		memcpy(data, jdata, ret);
        if(cmdInfo->cmdId == LELINK_CMD_CLOUD_MSG_CTRL_C2R_REQ && cmdInfo->subCmdId == LELINK_SUBCMD_CLOUD_MSG_CTRL_C2R_DO_OTA_REQ) {
            ret += RSA_LEN;
        }
		env->ReleaseByteArrayElements(jbytes, jdata, JNI_ABORT);
		//data[ret++] = '\0';
	}
	THREAD_DETACH(gNativeContext.jvm);
	return ret;
}

void halCBRemoteRsp(void *ctx, const CmdHeaderInfo* cmdInfo, const uint8_t *data, int len)
{
	int ret = 0;
	JNIEnv *env;

    if(cmdInfo->cmdId == LELINK_CMD_CLOUD_REPORT_RSP && cmdInfo->subCmdId == LELINK_SUBCMD_CLOUD_REPORT_OTA_QUERY_RSP) {
        data += RSA_LEN;
        len -= RSA_LEN;
    }
    if(len < 0) {
        LELOGE("Error len(%d)", len);
        return;
    }
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

