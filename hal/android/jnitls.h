/*
 * jnitls.h
 *
 *  Created on: Jan 7, 2016
 *      Author: fei
 */

#ifndef JNITLS_H_
#define JNITLS_H_

#include <jni.h>

//Attach主线程
#define THREAD_ATTACH(JVM, ENV) \
	(JVM->AttachCurrentThread((void **)&ENV, NULL) != JNI_OK)
//Detach主线程
#define THREAD_DETACH(JVM) \
	(JVM->DetachCurrentThread() != JNI_OK)

jbyteArray c2bytes(JNIEnv *env, const char *ptr, int size);
jstring c2js(JNIEnv *env, const char *str);
char* js2c(JNIEnv *env, jstring jstr); // jstring ----->char*, 需要free返回值

#endif /* JNITLS_H_ */
