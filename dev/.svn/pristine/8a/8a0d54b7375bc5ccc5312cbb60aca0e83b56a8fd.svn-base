#include <jni.h>

#include "com_letv_lelive_LeWrapper.h"

#include "leconfig.h"
#include "airconfig_ctrl.h"
#include "network.h"
#include "protocol.h"

#define MAX_CONTEXT 4
#define MAX_THREAD 4
#define MAX_DESC 64
#define JNI_VER JNI_VERSION_1_4

class LeContext {
public:
	LeContext(JNIEnv *env, jobject c, jobject d) : _env (env) {
		if (c)
			_objCaller = _env->NewGlobalRef(c);
		if (d)
			_objDelegate = _env->NewGlobalRef(d);
		memset(_pt, 0, sizeof(_pt));
		LELOG("LeContext exp _env[%p] _objCaller[%p] _objDelegate[%p] \r\n", _env, _objCaller, _objDelegate);
	}
    LeContext() : _env(0), _objCaller(0), _objDelegate(0) {
		memset(_pt, 0, sizeof(_pt));
		LELOG("LeContext cmn _env[%p] _objCaller[%p] _objDelegate[%p] \r\n", _env, _objCaller, _objDelegate);
    }
    ~LeContext() {
		LELOG("~LeContext _env[%p] _objCaller[%p] _objDelegate[%p] \r\n", _env, _objCaller, _objDelegate);

    	if (_env) {
    		if (_objCaller) {
	    		_env->DeleteGlobalRef(_objCaller);
    			_objCaller = 0;
    		}
    		if (_objDelegate) {
	    		_env->DeleteGlobalRef(_objDelegate);
    			_objDelegate = 0;
    		}
    	}
    }
	LeContext &operator=(const LeContext &ctx) {
		this->_env = ctx._env;
		if (this->_objCaller) {
    		_env->DeleteGlobalRef(this->_objCaller);
    	}
		this->_objCaller = _env->NewGlobalRef(ctx._objCaller);

		if (this->_objDelegate) {
			_env->DeleteGlobalRef(this->_objDelegate);
		}
		this->_objDelegate = _env->NewGlobalRef(ctx._objDelegate);
	}
	bool operator!=(const LeContext &ctx) {
		if (_objCaller != ctx._objCaller) {
			return true;
		} else {
			return false;
		}
	}
	pthread_t *getThreadPtr() {
		int i;
		for (i = 0; i < MAX_THREAD; i++) {
			if (!_pt[i]) {
				return &_pt[i];
			}
		}
		return NULL;
	}

	jobject _objDelegate;
	jobject _objCaller;
	JNIEnv *_env;
	pthread_t _pt[MAX_THREAD];

    // protocol using
    void *_hdlRemote;
    void *_hdlLocal;
};

class LeContextMgr {
public:
	static LeContextMgr *getInstance() {
		if (!LeContextMgr::_cmgr) {
			LeContextMgr::_cmgr = new LeContextMgr();
		}
		return LeContextMgr::_cmgr;
	}

	static void delInstance() {
		LeContextMgr::_ref--;
		LELOG("delInstance LeContextMgr::_ref[%d]\r\n", LeContextMgr::_ref);
		if (0 == LeContextMgr::_ref && LeContextMgr::_cmgr) {
			delete LeContextMgr::_cmgr;
			LeContextMgr::_cmgr = NULL;
		}
	}

	static void addRef() {
		LeContextMgr::_ref++;
	}

	static int initClassHelper(JNIEnv *env, const char *path, jobject *outObj) {
	    jclass cls = env->FindClass(path);
	    if(!cls) {
	        LELOGE("initClassHelper: failed to get %s class reference", path);
	        return -1;
	    }
	    jmethodID constr = env->GetMethodID(cls, "<init>", "()V");
	    if(!constr) {
	        LELOGE("initClassHelper: failed to get %s constructor", path);
	        return -2;
	    }
	    jobject obj = env->NewObject(cls, constr);
	    if(!obj) {
	        LELOGE("initClassHelper: failed to create a %s object", path);
	        return -3;
	    }
	    (*outObj) = env->NewGlobalRef(obj);
	}

	LeContextMgr() {
		_ctx = new LeContext[MAX_CONTEXT];
	}
	~LeContextMgr() {
		delete [] _ctx;
	}
	int set(const LeContext &ctx) {
		int i = 0, ok = 0;
		for (i = 0; i < MAX_CONTEXT; i++) {
			if (_ctx[i] != ctx) {
				if (0 == _ctx[i]._objCaller) {
					_ctx[i] = ctx;
					LELOG("set _ctx[%d] = [%p]\r\n", i, _ctx[i]._objCaller);
					return i;
				}
			} else {
				LELOG("set dup _ctx[%d] = [%p]\r\n", i, _ctx[i]._objCaller);
				return i;
			}
		}
		return -1;
	}
    
	LeContext *get(JNIEnv *env, jobject obj) {
		int i = 0, ok = 0;
		for (i = 0; i < MAX_CONTEXT; i++) {
    		if (env->IsSameObject(obj, _ctx[i]._objCaller)) { 
				LELOG("get _ctx[%d] = [%p]\r\n", i, _ctx[i]._objCaller);
				return &_ctx[i];
			}
		}		
		return NULL;
	}

private:
	static LeContextMgr *_cmgr;
	static LeContext *_ctx;
	static int _ref;
public:
	static JavaVM *_jvm;
	static jobject _objCachedClsLeBin;
	static jobject _objCachedClsLeJson;
	static jobject _objCachedClsLeStatus;
};

LeContextMgr *LeContextMgr::_cmgr = NULL;
LeContext *LeContextMgr::_ctx = NULL;
int LeContextMgr::_ref = 0;
JavaVM *LeContextMgr::_jvm = NULL;
jobject LeContextMgr::_objCachedClsLeBin;
jobject LeContextMgr::_objCachedClsLeJson;
jobject LeContextMgr::_objCachedClsLeStatus;

class ThreadParam {
public:
	// ThreadParam(LeContext *ctx, jobject d) : _ctx(ctx) {
	// 	if (d) {
	// 		LELOG("ThreadParam ctor s");
	// 		_data = _ctx->_env->NewGlobalRef(d);
	// 		LELOG("ThreadParam ctor env[%p] _data[%p] e", _ctx->_env, _data);
	// 	}
	// }
	// ~ThreadParam() {
	// 	if (_data) {
	// 		LELOG("ThreadParam dtor env[%p] _data[%p] s", _ctx->_env, _data);
	// 		_ctx->_env->DeleteGlobalRef(_data);
	// 		LELOG("ThreadParam dtor e");
	// 		_data = 0;
	// 	}
	// }
	LeContext *_ctx;
	jobject _data;
};

//// 由java调用来建立JNI环境 
//JNIEXPORT void Java_com_test_JniThreadTestActivity_setJNIEnv(JNIEnv* env, jobject obj) { 
     //// 保存全局JVM以便在子线程中使用 
	//env->GetJavaVM(&LeContextMgr::_jvm); 
	//// 不能直接赋值(g_obj = obj) 
	//g_obj = env->NewGlobalRef(obj);
//}

// 当动态库被加载时这个函数被系统调用 
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) { 
	JNIEnv* env = NULL; 
	jint result = -1;
	int ret;
    
	// 获取JNI版本 
    if (vm->GetEnv((void**)&env, JNI_VER) != JNI_OK) { 
		LELOGE("GetEnv failed!"); 
		return result; 
	}
    LeContextMgr::_jvm = vm;
	
	// cached for class obtain
	ret = LeContextMgr::initClassHelper(env, "com/letv/lelive/LeBin", &LeContextMgr::_objCachedClsLeBin);
	if (ret < 0) {
		LELOGE("cached Class LeBin");
	}
	ret = LeContextMgr::initClassHelper(env, "com/letv/lelive/LeJson", &LeContextMgr::_objCachedClsLeJson);
	if (ret < 0) {
		LELOGE("cached Class LeJson");
	}
	ret = LeContextMgr::initClassHelper(env, "com/letv/lelive/LeStatus", &LeContextMgr::_objCachedClsLeStatus);
	if (ret < 0) {
		LELOGE("cached Class LeStatus");
	}
    return JNI_VER; 
}

// JNIEXPORT jstring JNICALL Java_com_letv_lelive_LeWrapper_getString(JNIEnv *env, jobject obj, jstring test) {
// 	const char *str = env->GetStringUTFChars(test, 0);
// 	LELOG("pass in jstring is [%s]\r\n", str);
// 	return env->NewStringUTF("hello, world! JNI!!");
// }

// JNIEXPORT jobject JNICALL Java_com_letv_lelive_LeWrapper_getAccountInfo(JNIEnv *env, jobject obj)
// {
// 	//ssid = env->NewStringUTF("ssid ");
// 	//passwd = env->NewStringUTF("passwd ");
// 	return 0;
// }
static void testThreadWorker(JNIEnv* env, void *arg) {
    int ret;
    ThreadParam *param = (ThreadParam *)arg;
    jobject objDelegate = param->_ctx->_objDelegate;
    jobject objCaller = param->_ctx->_objCaller;
    jclass clsLeDelegate = env->GetObjectClass(objDelegate);
    jmethodID midNotify = env->GetMethodID(clsLeDelegate, "notify", "(Ljava/lang/Object;Lcom/letv/lelive/LeData;)I"); 
    
    LELOG("thread caller[%p] delegate[%p]\r\n", objCaller, objDelegate);
    
    if (!midNotify) {
        return;
    }
    
    // LeData calling
    jobject objLeData;

    // get LeBin class
    // jclass clsLeBin = env->FindClass("com/letv/lelive/LeBin");
    jclass clsLeBin = env->GetObjectClass(LeContextMgr::_objCachedClsLeBin);
    LELOG("HERE2 env[%p] [%p]", env, clsLeBin);

    jmethodID construct = env->GetMethodID(clsLeBin,"<init>", "()V");
    LELOG("HERE");
    jfieldID fidId = env->GetFieldID(clsLeBin,"id", "I");
    LELOG("HERE");
    jfieldID fidBin = env->GetFieldID(clsLeBin,"bin", "[B");

    LELOG("fidId[%p] fidBin[%p]\r\n", fidId, fidBin);

        // new obj
    objLeData = env->NewObject(clsLeBin, construct);
    LELOG("objLeData[%p]\r\n", objLeData);

        // id
    env->SetIntField(objLeData,fidId, 21);

        // bin
    unsigned char buff[16] = {"abcdefg"};
    jbyteArray jbarray = env->NewByteArray(16);
    LELOG("jbarray[%p]\r\n", jbarray);
    env->SetByteArrayRegion(jbarray, 0, 16, (jbyte *)buff);
    env->SetObjectField(objLeData, fidBin, jbarray);

        // call java
    ret = env->CallIntMethod(objDelegate, midNotify, objCaller, objLeData);
    LELOG("CallIntMethod with LeBin ret[%d]\r\n", ret);


        // LeJson calling
    jobject objLeJson;

        // get LeJson class
        // jclass clsLeJson = env->FindClass("com/letv/lelive/LeJson");
    jclass clsLeJson = env->GetObjectClass(LeContextMgr::_objCachedClsLeJson);
    jmethodID ctorLeJson = env->GetMethodID(clsLeJson,"<init>", "()V");
    jfieldID fidStr = env->GetFieldID(clsLeJson, "str", "Ljava/lang/String;");

        // new obj
    objLeJson = env->NewObject(clsLeJson, ctorLeJson);

        // str
    char buffJson[] = "{\"pwr\":2}";
    env->SetObjectField(objLeJson, fidStr, env->NewStringUTF(buffJson));

        // call java
    ret = env->CallIntMethod(objDelegate, midNotify, objCaller, objLeJson);
    LELOG("CallIntMethod with LeJson ret[%d]\r\n", ret);
}

static void pollingQ2A(JNIEnv* env, void *arg) {
    ThreadParam *param = (ThreadParam *)arg;
    LeContext *ctx = param->_ctx;
    doPollingQ2A(ctx->_hdlLocal);
}

void *threadProcWorker(void* arg) 
{ 
    JNIEnv *env = NULL;
	ThreadParam *param = (ThreadParam *)arg;


#ifdef LINUX
    jint jret = LeContextMgr::_jvm->AttachCurrentThread((void **)&env, NULL);
#else
    jint jret = LeContextMgr::_jvm->AttachCurrentThread(&env, NULL);
#endif
    if (jret != JNI_OK) { 
        LELOGE("%s: AttachCurrentThread() failed \r\n", __FUNCTION__); 
		delete param;
        return NULL; 
    } 
    

    
    int count = 1, ret;
    while (1) {

        // testThreadWorker(env, arg);
        pollingQ2A(env, arg);
        
    	//while (1) {
        	//pollingState();		
        	//delayms(1000);
        //}
        // delayms(1000);
        //LELOG("thread doing delegate[%s] \r\n", LeContextMgr::toString(env, objDelegate));
    }    
    
error:   
     
    if (LeContextMgr::_jvm->DetachCurrentThread() != JNI_OK) { 
        LELOGE("%s: DetachCurrentThread() failed \r\n", __FUNCTION__); 
    } 

    LELOG("sub thread done \r\n"); 
	delete param;
    pthread_exit(0); 
} 

void *threadProcAirconfig(void* arg) {

	JNIEnv *env = NULL;
	ThreadParam *param = (ThreadParam *)arg;

    jobject objAccount = param->_data;
    jobject objCaller = param->_ctx->_objCaller;
    jobject objDelegate = param->_ctx->_objDelegate;
	LELOG("thread caller[%p] delegate[%p] objAccount[%p]\r\n", objCaller, objDelegate, objAccount);


#ifdef LINUX
	jint jret = LeContextMgr::_jvm->AttachCurrentThread((void **)&env, NULL);
#else
    jint jret = LeContextMgr::_jvm->AttachCurrentThread(&env, NULL);
#endif

    if (jret != JNI_OK) { 
        LELOGE("%s: AttachCurrentThread() failed \r\n", __FUNCTION__); 
        return NULL; 
    } 

	jclass clsLeAccount = env->GetObjectClass(objAccount); 
    jfieldID fidSsid = env->GetFieldID(clsLeAccount, "ssid", "Ljava/lang/String;");
    jstring ssid = (jstring)env->GetObjectField(objAccount, fidSsid);
	jfieldID fidPasswd = env->GetFieldID(clsLeAccount, "passwd", "Ljava/lang/String;");
	jstring passwd = (jstring)env->GetObjectField(objAccount, fidPasswd);
	jfieldID fidAES = env->GetFieldID(clsLeAccount, "aes", "Ljava/lang/String;");
	jstring aes = (jstring)env->GetObjectField(objAccount, fidAES);
	jfieldID fidType = env->GetFieldID(clsLeAccount, "type", "I");
	jint type = (jint)env->GetIntField(objAccount, fidType);
	jfieldID fidDelay = env->GetFieldID(clsLeAccount, "delay", "I");
	jint delay = (jint)env->GetIntField(objAccount, fidDelay);

	// get info
	const char *strSsid = env->GetStringUTFChars(ssid, 0);
	const char *strPasswd = env->GetStringUTFChars(passwd, 0);
	const char *strAES = env->GetStringUTFChars(aes, 0);
	LELOG("objAccount [%s][%s][%s][%d][%d]\r\n", strSsid, strPasswd, strAES, type, delay);


	char strParam[128] = {0};
	sprintf(strParam, "SSID=%s,PASSWD=%s,AES=%s,TYPE=%d,DELAY=%d", 
		strSsid, strPasswd, strAES, type, delay);
	void *context = airconfig_new(strParam);
	//void *context = airconfig_new("SSID=letv-office,PASSWD=abcdefghijklmnopqrstuvwxyz12345,AES=912EC803B2CE49E4A541068D495AB570,TYPE=1,DELAY=10");
    while (1) {
        if (airconfig_do_config(context)) {
            break;
        }
    };
    
    airconfig_delete(context);



	// LeStatus calling
	jobject objLeStatus;

	// get LeStatus class
	// jclass clsLeStatus = env->FindClass("com/letv/lelive/LeStatus");
	jclass clsLeStatus = env->GetObjectClass(LeContextMgr::_objCachedClsLeStatus);
	jmethodID ctor = env->GetMethodID(clsLeStatus,"<init>", "()V");
	jfieldID fidRet = env->GetFieldID(clsLeStatus, "what", "I");

	// new obj
	objLeStatus = env->NewObject(clsLeStatus, ctor);

	// int
	int ret = 32;
	env->SetIntField(objLeStatus, fidRet, ret);

	// callback java
    jclass clsLeDelegate = env->GetObjectClass(objDelegate);
    jmethodID midNotify = env->GetMethodID(clsLeDelegate, "notify", "(Ljava/lang/Object;Lcom/letv/lelive/LeData;)I"); 
 	if (midNotify) {
		ret = env->CallIntMethod(objDelegate, midNotify, objCaller, objLeStatus);
 	}

    
    if (LeContextMgr::_jvm->DetachCurrentThread() != JNI_OK) { 
        LELOGE("%s: DetachCurrentThread() failed \r\n", __FUNCTION__); 
    } 

error:   
    LELOG("sub threadProcAirconfig done1 \r\n"); 
	// env->DeleteGlobalRef(objAccount);
    LELOG("sub threadProcAirconfig done2 \r\n"); 
	delete param;
    LELOG("sub threadProcAirconfig done3 \r\n"); 
    pthread_exit(0); 

}

JNIEXPORT jint JNICALL Java_com_letv_lelive_LeWrapper_init(JNIEnv *env, jobject obj, jobject delegate) {
	LELOG("LeWrapper:init\r\n");
    
	int i = 0, ret = -1;
	LeContext ctx(env, obj, delegate);
	LELOG("caller[%p]\r\n", ctx._objCaller);
	i = LeContextMgr::getInstance()->set(ctx);
	if (i >= 0) {
		LeContextMgr::addRef();
	}

	LeContext *pCtx = LeContextMgr::getInstance()->get(env, obj);
    if (!pCtx) {
        LELOGE("LeContextMgr::getInstance NULL\r\n");
        return -2;
    }

    pCtx->_hdlLocal = nwNew(NULL, 0, NW_SELF_PORT, 0);
    if (!pCtx->_hdlLocal) {
        LELOGE("nwNew NULL\r\n");
        return -3;
    }
    
    ThreadParam *param = new ThreadParam();
    param->_ctx = pCtx;
    ret = pthread_create(pCtx->getThreadPtr(), NULL, &threadProcWorker, (void *)param); 

    // delayms(10000);
    //pthread_join(*(pCtx->getThreadPtr()), NULL);   
    if (!ret) { 
        LELOGE("pthread_create [%d]\r\n", ret);
        return ret;
    }

	return ret;
}



JNIEXPORT jint JNICALL Java_com_letv_lelive_LeWrapper_exit(JNIEnv *env, jobject obj) {
	LeContextMgr::delInstance();
	LELOG("LeWrapper:exit\r\n");
	return 0;
}

JNIEXPORT jint JNICALL Java_com_letv_lelive_LeWrapper_airconfigCtrl(JNIEnv *env, jobject obj, 
	jobject accountInfo) {
    LELOG("LeWrapper:airconfigCtrlSync\r\n");

    // get LeAccount class
	// jclass clsLeAccount = env->GetObjectClass(accountInfo); 
	// jfieldID fidSsid = env->GetFieldID(clsLeAccount, "ssid", "Ljava/lang/String;");
 //    jstring ssid = (jstring)env->GetObjectField(accountInfo, fidSsid);
	// jfieldID fidPasswd = env->GetFieldID(clsLeAccount, "passwd", "Ljava/lang/String;");
	// jstring passwd = (jstring)env->GetObjectField(accountInfo, fidPasswd);
	
	// // get info
	// const char *strSsid = env->GetStringUTFChars(ssid, 0);
	// const char *strPasswd = env->GetStringUTFChars(passwd, 0);
	// LELOG("accountInfo [%s][%s]\r\n", strSsid, strPasswd);


	 int i = 0, ret;
	LeContext *pCtx = LeContextMgr::getInstance()->get(env, obj);

	if (pCtx) {
		// ThreadParam *param = new ThreadParam(pCtx, accountInfo);
		ThreadParam *param = new ThreadParam();
		static jobject data = 0;
		if (data) {
			env->DeleteGlobalRef(data);
		}
		data = param->_data = env->NewGlobalRef(accountInfo);
		param->_ctx = pCtx;
		LELOG("NEW env[%p] data[%p]", env, param->_data);
		// LELOG("delete 1");
		// delete param;
		// LELOG("delete 2");
		// return ret;
		ret = pthread_create(pCtx->getThreadPtr(), NULL, &threadProcAirconfig, (void *)param); 

        // pthread_join(*(pCtx->getThreadPtr()), NULL);
	}
	return ret;
}

JNIEXPORT jint JNICALL Java_com_letv_lelive_LeWrapper_postDiscovery(JNIEnv *env, jobject obj, jobject data) {

	return 0;
}

JNIEXPORT jobject JNICALL Java_com_letv_lelive_LeWrapper_polling(JNIEnv *env, jobject obj) {
	return 0;
	
}
