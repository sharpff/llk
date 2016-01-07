/*
 * jniLeLink.cpp
 *
 *  Created on: Jan 7, 2016
 *      Author: fei
 */
#include <stdio.h>
#include <string.h>
#include "halCenter.h"
#include "jniLeLink.h"

JNIEXPORT jlong JNICALL Java_com_letv_lelink_LeLink_init(JNIEnv *env, jobject jobj, jstring jstr)
{
	jclass cls;

	//保存全局JVM以便在子线程中使用
	env->GetJavaVM(&(gNativeContext.jvm));
	gNativeContext.obj = env->NewGlobalRef(jobj);
	cls = env->FindClass("com/letv/lelink/LeLink"); //	C++ 中映射类
	gNativeContext.onMessage = env->GetMethodID(cls, "onMessage", "(ILjava/lang/String;[B)I"); //	C++ 中映射非静态
	initTask();
	return (long) &gNativeContext;
}

JNIEXPORT jstring JNICALL Java_com_letv_lelink_LeLink_getSDKInfo(JNIEnv *env, jobject jobj, jlong ptr)
{
	Json::Value root;

	root["version"] = gNativeContext.version;

	return c2js(env, root.toStyledString().c_str());
}

JNIEXPORT void JNICALL Java_com_letv_lelink_LeLink_airConfig(JNIEnv *env, jobject jobj, jlong ptr, jstring jstr)
{
}

JNIEXPORT void JNICALL Java_com_letv_lelink_LeLink_send(JNIEnv *env, jobject jobj, jlong ptr, jstring jstr)
{
}

