/*
 * jnitls.cpp
 *
 *  Created on: Jan 7, 2016
 *      Author: fei
 */
#include "leconfig.h"
#include <string.h>
#include "jnitls.h"

jbyteArray c2bytes(JNIEnv *env, const char *ptr, int size)
{
	jbyteArray bytes = env->NewByteArray(size);
	env->SetByteArrayRegion(bytes, 0, size, (jbyte*) ptr);
	return bytes;
}

/*******************************************************************
 *char*----->jstring
 *******************************************************************/
jstring c2js(JNIEnv *env, const char *str)
{
	jclass strClass = env->FindClass("java/lang/String");
	jmethodID ctorID = env->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
	jbyteArray bytes = env->NewByteArray(strlen(str));
	env->SetByteArrayRegion(bytes, 0, strlen(str), (jbyte*) str);
	jstring encoding = env->NewStringUTF("utf-8");
	return (jstring) env->NewObject(strClass, ctorID, bytes, encoding);
}
/******************************************************************
 *jstring ----->char*, 需要free返回值
 ********************************************************************/
char* js2c(JNIEnv *env, jstring jstr)
{
	char* rtn = NULL;
	jclass clsstring = env->FindClass("java/lang/String");
	jstring strencode = env->NewStringUTF("utf-8");
	jmethodID mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");
	jbyteArray barr = (jbyteArray) env->CallObjectMethod(jstr, mid, strencode);
	jsize alen = env->GetArrayLength(barr);
	jbyte* b = env->GetByteArrayElements(barr, JNI_FALSE);
	if (alen > 0) {
		rtn = (char*) halMalloc(alen + 1);
		memcpy(rtn, b, alen);
		rtn[alen] = 0;
	}
	env->ReleaseByteArrayElements(barr, b, 0);
	return rtn;
}

