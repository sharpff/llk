package com.letv.lelink;

public class LeLink {

	private long mPtr = 0;
	
	private LeLink() {
		mPtr = init(null);
	}

	private static final int MSG_TYPE_LOCALREQUEST = 1;
	private static final int MSG_TYPE_LOCALRESPOND = 2;
	private static final int MSG_TYPE_REMOTEREQUEST = 3;
	private static final int MSG_TYPE_REMOTERESPOND = 4;

	/*
	 * Parameter
	 * 
	 * jsonStr<json>: null
	 *  
	 * Return
	 *  
	 * native C pointer
	 */
	private native long init(String jsonStr);

	/*
	 * Parameter
	 * 
	 * ptr: 
	 *  
	 * Return
	 *  
	 * SDK information
	 */
	private native String getSDKInfo(long ptr);

	/*
	 * Parameter
	 * 
	 * jsonStr<json> - String ssid; String passwd; int delay(ms); int type(1-mutilcast, 2-broadcast)
	 * 
	 */
	private native void airConfig(long ptr, String jsonStr);

	/*
	 * Parameter
	 * 
	 * cmdJson<Json> - String addr; int cmdId; int subCmdId; int seqid
	 * 
	 * dataStr - userdata
	 */
	private native void send(long ptr, String jsonStr);

	static {
		System.loadLibrary("lelink");
	}
}
