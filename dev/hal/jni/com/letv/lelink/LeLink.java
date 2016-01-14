package com.letv.lelink;

import java.io.UnsupportedEncodingException;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import android.util.Log;

public class LeLink {

	private long mPtr = 0;
	private static LeLink sLeLink = null;
	private static final String TAG = "LeLinkJar";
	private static final long CLOUD_HEART_RESPOND_TIMEOUT = 1000 * 10;
	private static final int DEFAULT_TIMEOUT = 3;
	private static final int MAX_WAIT_CMD = 10;

	private int mSeqId = 0;
	private ST_t mState = ST_t.HEART;
	private String mCloudAddr = null;
	private boolean mIsGetDevHello = false;
	private long mGetCloudHeartRspTime = 0;
	// for get & discover
	private String mWaitGetUuid = null;
	private Map<String, JSONObject> mFindDevs = new HashMap<String, JSONObject>(); // uuid
	// for send
	private int mMinSeqId = -1;
	private Lock mGetLock = new ReentrantLock();
	private Map<String, String> mWaitSendCmds = new HashMap<String, String>(); // seqid
	// for ctrl
	private String mWaitCtrlUuid = null;
	private String mWaitCtrlBackData = null;
	private Lock mCtrlLock = new ReentrantLock();

	public static LeLink getInstance() {
		if (sLeLink == null) {
			sLeLink = new LeLink();
		}
		return sLeLink;
	}

	public String getSdkInfo() {
		return getSDKInfo(mPtr);
	}

	/*
	 * Parameter
	 * 
	 * jsonStr<json> - String ssid, String passwd, int timeout(sec)
	 * 
	 * 
	 * Return
	 * 
	 * -1 - error; 0 - success; 1 - timeout
	 */
	public int airConfig(String jsonStr) {
		int startTime, timeout;
		JSONObject json = null;

		if (jsonStr == null) {
			LOGE("airConfig, string null");
			return -1;
		}
		startTime = (int) (System.currentTimeMillis() / 1000);
		try {
			json = new JSONObject(jsonStr);
			json.getString(LeCmd.K.SSID);
			json.getString(LeCmd.K.PASSWD);
			timeout = json.getInt(LeCmd.K.TIMEOUT);
		} catch (JSONException e) {
			LOGE("Parameter json error");
			e.printStackTrace();
			return -1;
		}
		mIsGetDevHello = false;
		while (((int) (System.currentTimeMillis() / 1000) - startTime < timeout) && !mIsGetDevHello) {
			airConfig(mPtr, jsonStr);
		}
		return mIsGetDevHello ? 0 : 1;
	}

	/*
	 * Parameter
	 * 
	 * timeout - discover timeout. second
	 * 
	 * 
	 * Return
	 * 
	 * null - timeout return; else - device array json string
	 */
	public String discover(int timeout) {
		JSONObject sendJson;
		try {
			sendJson = new JSONObject();
			sendJson.put(LeCmd.K.ADDR, LeCmd.V.BROADCAST_ADDR);
			sendJson.put(LeCmd.K.TIMEOUT, timeout);
		} catch (JSONException e) {
			LOGE("Json error");
			e.printStackTrace();
			return null;
		}
		return getState(sendJson.toString());
	}

	/*
	 * Parameter
	 * 
	 * jsonStr<Json> - String uuid; String addr; String token; int timeout
	 * 
	 * 
	 * Return
	 * 
	 * null - ctrl timeout; else - device array json string
	 */
	public synchronized String ctrl(String jsonStr, String dataJson) {
		String uuid;
		int timeout;
		JSONObject cmdJson = null;

		try {
			cmdJson = new JSONObject(jsonStr);
			uuid = cmdJson.getString(LeCmd.K.UUID);
			timeout = cmdJson.getInt(LeCmd.K.TIMEOUT);
			timeout = timeout < 0 ? DEFAULT_TIMEOUT : timeout;
			cmdJson.put(LeCmd.K.TIMEOUT, timeout);
			if (cmdJson.has(LeCmd.K.ADDR)) {
				cmdJson.put(LeCmd.K.CMD, LeCmd.CTRL_REQ);
				cmdJson.put(LeCmd.K.SUBCMD, LeCmd.Sub.CTRL_CMD_REQ);
			} else if (mState == ST_t.HEART) {
				cmdJson.put(LeCmd.K.CMD, LeCmd.CLOUD_MSG_CTRL_C2R_REQ);
				cmdJson.put(LeCmd.K.SUBCMD, LeCmd.Sub.CLOUD_MSG_CTRL_C2R_REQ);
			} else {
				LOGE("Wait cloud auth!");
				return null;
			}
		} catch (JSONException e) {
			LOGE("Json error");
			e.printStackTrace();
			return null;
		}
		mWaitCtrlUuid = uuid;
		mWaitCtrlBackData = null;
		send(cmdJson, dataJson);
		synchronized (mCtrlLock) {
			try {
				mCtrlLock.wait(1000 * timeout);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
		if (mWaitCtrlBackData == null) {
			LOGE("Control timeout");
		}
		return mWaitCtrlBackData;
	}

	/*
	 * Parameter
	 * 
	 * jsonStr<Json> - String addr; String uuid; int timeout
	 * 
	 * 
	 * Return
	 * 
	 * null - timeout return; else - device array json string
	 */
	public String getState(String jsonStr) {
		int timeout;
		JSONObject cmdJson, sendJson;
		JSONArray jsonArray = new JSONArray();

		synchronized (mFindDevs) {
			mFindDevs.clear();
			cmdJson = new JSONObject();
			try {
				sendJson = new JSONObject(jsonStr);
				timeout = sendJson.getInt(LeCmd.K.TIMEOUT);
				timeout = timeout < 0 ? DEFAULT_TIMEOUT : timeout;
				sendJson.put(LeCmd.K.TIMEOUT, timeout);
				if (sendJson.has(LeCmd.K.ADDR)) {
					if (!sendJson.getString(LeCmd.K.ADDR).equals(LeCmd.V.BROADCAST_ADDR)) {
						cmdJson.put(LeCmd.K.UUID, sendJson.getString(LeCmd.K.UUID));
					}
					cmdJson.put(LeCmd.K.CMD, LeCmd.DISCOVER_REQ);
					cmdJson.put(LeCmd.K.SUBCMD, LeCmd.Sub.DISCOVER_REQ);
					cmdJson.put(LeCmd.K.ADDR, sendJson.getString(LeCmd.K.ADDR));
				} else if (mState == ST_t.HEART) {
					cmdJson.put(LeCmd.K.CMD, LeCmd.CLOUD_REPORT_REQ);
					cmdJson.put(LeCmd.K.SUBCMD, LeCmd.Sub.CLOUD_REPORT_REQ);
					cmdJson.put(LeCmd.K.UUID, sendJson.getString(LeCmd.K.UUID));
				} else {
					LOGE("Waiting cloud auth, try again!");
					return null;
				}
				mWaitGetUuid = cmdJson.has(LeCmd.K.UUID) ? cmdJson.getString(LeCmd.K.UUID) : null;
			} catch (JSONException e) {
				LOGE("Json error");
				e.printStackTrace();
				return null;
			}

			send(cmdJson, null);
			LOGI("Waiting get...");
			synchronized (mGetLock) {
				try {
					mGetLock.wait(1000 * timeout);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
			LOGI("Wait over!");
			if (mFindDevs.size() <= 0) {
				return null;
			}
			for (JSONObject v : mFindDevs.values()) {
				jsonArray.put(v);
			}
		}
		return jsonArray.toString();
	}

	/*
	 * Parameter
	 * 
	 * cmdJson<Json> - String addr, int cmdId, int subCmdId
	 * 
	 * dataStr - userdata
	 */
	private synchronized void send(JSONObject cmdJson, String dataStr) {
		synchronized (mWaitSendCmds) {
			String keyStr;
			int min = mMinSeqId;
			if (mWaitSendCmds.size() >= MAX_WAIT_CMD) {
				for (; min != mSeqId; min++) {
					keyStr = String.valueOf(min);
					if (mWaitSendCmds.get(keyStr) != null) {
						mWaitSendCmds.remove(keyStr);
						break;
					}
				}
				mMinSeqId = min + 1;
			}
			if (dataStr != null) {
				mWaitSendCmds.put(String.valueOf(mSeqId + 1), dataStr);
			}
		}
		mSeqId++;
		try {
			cmdJson.put(LeCmd.K.SEQID, mSeqId);
			if (!cmdJson.has(LeCmd.K.ADDR) && mCloudAddr != null) {
				cmdJson.put(LeCmd.K.ADDR, mCloudAddr);
			}
		} catch (JSONException e) {
			LOGE("Json error");
			e.printStackTrace();
			return;
		}
		send(mPtr, cmdJson.toString());
	}

	private String getAndRemoveSendCmd(int seqid) {
		String dataJson = null;
		String keyStr = String.valueOf(seqid);

		synchronized (mWaitSendCmds) {
			dataJson = mWaitSendCmds.get(keyStr);
			if (dataJson != null) {
				mWaitSendCmds.remove(keyStr);
			}
		}
		return dataJson;
	}

	private LeLink() {
		mPtr = init(null);
		// mThread.start();
	}

	private static final int MSG_TYPE_LOCALREQUEST = 1;
	private static final int MSG_TYPE_LOCALRESPOND = 2;
	private static final int MSG_TYPE_REMOTEREQUEST = 3;
	private static final int MSG_TYPE_REMOTERESPOND = 4;

	// private Lock lock = new ReentrantLock();
	private int onMessage(int type, String jsonStr, byte buf[]) {
		int ret = 0;
		int cmd, subcmd, seqId;
		String addr, uuid, token, dataStr;
		JSONObject cmdJson, sendCmdJson, dataJson;

		try {
			cmdJson = new JSONObject(jsonStr);
			cmd = cmdJson.getInt(LeCmd.K.CMD);
			subcmd = cmdJson.getInt(LeCmd.K.SUBCMD);
			addr = cmdJson.getString(LeCmd.K.ADDR);
			uuid = cmdJson.getString(LeCmd.K.UUID);
			token = cmdJson.getString(LeCmd.K.TOKEN);
			seqId = cmdJson.getInt(LeCmd.K.SEQID);
		} catch (JSONException e) {
			LOGE("Json error");
			e.printStackTrace();
			return ret;
		}
		sendCmdJson = new JSONObject();
		LOGI("type " + type + ", onMessage: " + jsonStr);
		switch (type) {
		case MSG_TYPE_LOCALREQUEST:
		case MSG_TYPE_LOCALRESPOND:
			dataStr = getAndRemoveSendCmd(seqId);
			if (dataStr != null) {
				byte[] bytes = dataStr.getBytes();
				System.arraycopy(bytes, 0, buf, 0, bytes.length);
				ret = bytes.length;
			}
			break;
		case MSG_TYPE_REMOTEREQUEST:
			if (cmd == LeCmd.HELLO_REQ && subcmd == LeCmd.Sub.HELLO_REQ) {
				try {
					sendCmdJson.put(LeCmd.K.CMD, LeCmd.HELLO_RSP);
					sendCmdJson.put(LeCmd.K.SUBCMD, LeCmd.Sub.HELLO_RSP);
					sendCmdJson.put(LeCmd.K.ADDR, addr);
				} catch (JSONException e) {
					LOGE("Json error");
					e.printStackTrace();
					return -1;
				}
				LOGI("Get device hello");
				send(sendCmdJson, null);
				mIsGetDevHello = true;
			}
			break;
		case MSG_TYPE_REMOTERESPOND:
			try {
				dataStr = new String(buf, "UTF-8");
				dataJson = new JSONObject(dataStr);
				if (cmd == LeCmd.CTRL_RSP || cmd == LeCmd.CLOUD_MSG_CTRL_C2R_RSP) {
					mWaitCtrlBackData = dataStr;
					synchronized (mCtrlLock) {
						mCtrlLock.notifyAll();
					}
				} else if (cmd == LeCmd.DISCOVER_RSP || cmd == LeCmd.CLOUD_REPORT_RSP) {
					if (mWaitGetUuid != null) {
						mFindDevs.clear();
						mFindDevs.put(uuid, dataJson);
						synchronized (mGetLock) {
							mGetLock.notifyAll();
						}
					} else {
						mFindDevs.put(uuid, dataJson);
					}
				} else if (cmd == LeCmd.CLOUD_GET_TARGET_RSP && subcmd == LeCmd.Sub.CLOUD_GET_TARGET_RSP) {
					if (dataJson.has(LeCmd.K.REMOTE_ADDR)) {
						mState = ST_t.AUTH2;
						mCloudAddr = dataJson.getString(LeCmd.K.REMOTE_ADDR);
					} else {
						mState = ST_t.HEART;
					}
				} else if (cmd == LeCmd.CLOUD_AUTH_RSP && subcmd == LeCmd.Sub.CLOUD_AUTH_RSP) {
					mState = ST_t.HEART;
				} else if (cmd == LeCmd.CLOUD_HEARTBEAT_RSP && subcmd == LeCmd.Sub.CLOUD_HEARTBEAT_RSP) {
					mGetCloudHeartRspTime = System.currentTimeMillis();
				}
			} catch (UnsupportedEncodingException e1) {
				LOGE("Json error");
				e1.printStackTrace();
				return ret;
			} catch (JSONException e1) {
				LOGE("Json error");
				e1.printStackTrace();
				return ret;
			}
			break;
		}
		return ret;
	}

	/*
	 * Parameter
	 * 
	 * cmdJson<Json> - String addr, int cmdId, int subCmdId
	 * 
	 * dataStr - userdata
	 */
	// private synchronized void send(JSONObject cmdJson, String dataStr) {
	private Thread mThread = new Thread(new Runnable() {
		@Override
		public void run() {
			long sleepMs = 1000;
			JSONObject cmdJson = null;

			while (true) {
				cmdJson = new JSONObject();
				try {
					if (mState == ST_t.AUTH1) {
						cmdJson.put(LeCmd.K.CMD, LeCmd.CLOUD_GET_TARGET_REQ);
						cmdJson.put(LeCmd.K.SUBCMD, LeCmd.Sub.CLOUD_GET_TARGET_REQ);
						send(cmdJson, null); // 准备进入AUTH1
						sleepMs = 3000;
					} else if (mState == ST_t.AUTH2) {
						cmdJson.put(LeCmd.K.CMD, LeCmd.CLOUD_AUTH_REQ);
						cmdJson.put(LeCmd.K.SUBCMD, LeCmd.Sub.CLOUD_AUTH_REQ);
						send(cmdJson, null);
						sleepMs = 3000;
					} else if (mState == ST_t.HEART) {
						if (mGetCloudHeartRspTime > 0
								&& System.currentTimeMillis() - mGetCloudHeartRspTime > CLOUD_HEART_RESPOND_TIMEOUT) {
							mCloudAddr = null;
							mState = ST_t.AUTH1;
							mGetCloudHeartRspTime = 0;
						} else {
							cmdJson.put(LeCmd.K.CMD, LeCmd.CLOUD_AUTH_REQ);
							cmdJson.put(LeCmd.K.SUBCMD, LeCmd.Sub.CLOUD_AUTH_REQ);
							send(cmdJson, null);
						}
						sleepMs = 3000;
					}
				} catch (JSONException e) {
					LOGE("Json error");
					e.printStackTrace();
				}
				try {
					Thread.sleep(sleepMs);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
		}
	});

	private void LOGD(String msg) {
		logout(Log.DEBUG, msg);
	}
	
	private void LOGI(String msg) {
		logout(Log.INFO, msg);
	}

	private void LOGE(String msg) {
		logout(Log.ERROR, msg);
	}

	private void logout(int priority, String msg) {
		Log.println(priority, TAG, msg);
	}

	private enum ST_t {
		AUTH1, AUTH2, HEART,
	};

	// Json: String serverAddr, short serverPort, short devPort
	private native long init(String jsonStr);

	private native String getSDKInfo(long ptr);

	private native void airConfig(long ptr, String jsonStr);

	private native void send(long ptr, String jsonStr);

	static {
		System.loadLibrary("lelink");
	}
}
