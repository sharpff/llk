package com.letv.lelink;

public class LeLink {

	private long mPtr = 0;
	private Listener mListener = null;
	private static LeLink sLeLink = null;

	public static LeLink getInstance() {
		if (sLeLink == null) {
			sLeLink = new LeLink();
		}
		return sLeLink;
	}

	public String getSDKInfo() {
		return getSDKInfo(mPtr);
	}

	public void setListener(Listener listener) {
		mListener = listener;
	}

	// Json: String ssid, String passwd, int timeout(sec)
	public void airConfig(String jsonStr) {
		airConfig(mPtr, jsonStr);
	}

	// Json: int cmdId, int subCmdId
	public void send(String jsonStr) {
		send(mPtr, jsonStr);
	}

	private LeLink() {
		mPtr = init(null);
	}

	private static final int MSG_TYPE_LOCALREQUEST = 1;
	private static final int MSG_TYPE_LOCALRESPOND = 2;
	private static final int MSG_TYPE_REMOTEREQUEST = 3;
	private static final int MSG_TYPE_REMOTERESPOND = 4;

	private int onMessage(int type, String jsonstr, byte buf[]) {
		int ret = -1;

		if (mListener == null) {
			return ret;
		}
		switch (type) {
		case MSG_TYPE_LOCALREQUEST:
			ret = mListener.onLocalRequest(jsonstr, buf);
			break;
		case MSG_TYPE_LOCALRESPOND:
			ret = mListener.onLocalRespond(jsonstr, buf);
			break;
		case MSG_TYPE_REMOTEREQUEST:
			ret = mListener.onRemoteRequest(jsonstr, buf);
			break;
		case MSG_TYPE_REMOTERESPOND:
			ret = mListener.onRemoteRespond(jsonstr, buf);
			break;
		}
		return ret;
	}

	// Json: String serverAddr, short serverPort, short devPort
	private native long init(String jsonStr);

	private native String getSDKInfo(long ptr);

	private native void airConfig(long ptr, String jsonStr);

	private native void send(long ptr, String jsonStr);

	// Json: int cmdId, int subCmdId
	public interface Listener {
		int onLocalRequest(String jsonstr, byte outBuf[]);

		int onLocalRespond(String jsonstr, byte outBuf[]);

		int onRemoteRequest(String jsonstr, byte inBuf[]);

		int onRemoteRespond(String jsonstr, byte inBuf[]);
	}

	static {
		System.loadLibrary("lelink");
	}
}
