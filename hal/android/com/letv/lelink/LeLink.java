package com.letv.lelink;

public class LeLink {

	private long mPtr = 0;

	private LeLink(String info) {
		mPtr = init(info);
	}

	private static final int MSG_TYPE_LOCALREQUEST = 1;
	private static final int MSG_TYPE_LOCALRESPOND = 2;
	private static final int MSG_TYPE_REMOTEREQUEST = 3;
	private static final int MSG_TYPE_REMOTERESPOND = 4;

	/**
	 * 
	 * @return
	 * 		SDK information
	 * 
	 * @hide
	 */
	private static native String getSDKInfo();

	/**
	 * 
	 * @param jsonStr
	 * 		null
	 * 
	 * @return
	 * 		native C pointer
	 * 
	 * @hide
	 */
	private native long init(String jsonStr);

	/**
	 * 
	 * @param ptr
	 * 
	 * @param jsonStr
	 * 		String ssid; String passwd; int delay(ms); int type(1-mutilcast, 2-broadcast)
	 * 
	 * @hide
	 */
	private native int airConfig(long ptr, String jsonStr);

	/**
	 * 
	 * @param ptr
	 * 
	 * @param jsonStr
	 *		String addr; int cmdId; int subCmdId; int seqid
	 *
	 * @hide
	 */
	private native int send(long ptr, String jsonStr);

	static {
		System.loadLibrary("lelink");
	}
}
