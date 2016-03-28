package com.letv.lelink;

import java.io.IOException;
import java.io.InputStream;
import java.io.UnsupportedEncodingException;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import android.content.Context;
import android.util.Base64;
import android.util.Log;

/** 
 * 
 * Lelink Android平台接入SDK接口 <br>
 * Copyright © 2004-2016 乐视网（letv.com）All rights reserved.<br>
 * 
 * @version v0.1
 * 
 * @author  feiguoyou@letv.com
 */
public class LeLink {

	private static LeLink sLeLink = null;
	private static final String TAG = "LeLinkJar";
	private static final int MAX_WAIT_CMD = 10;
	private static final int DEFAULT_TIMEOUT = 3;
	private static final int DEFAULT_AIRCONFIG_DELAY = 10;
	private static final long SEND_HEART_TIME = 1000 * 20;
	private static final long CLOUD_HEART_RESPOND_TIMEOUT = 1000 * 60;

	private int mSeqId = 0;
	private ST_t mState = ST_t.AUTH1;
	private String mCloudAddr = null;
	private boolean mIsGetDevHello = false;
	private long mSendHeartTime = 0;
	private long mGetCloudHeartRspTime = 0;
	private static String mInitInfo = null;
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

	/**
	 * 获得LeLink SDK的信息<br>
	 * 
	 * @return
	 * 		SDK信息
	 */
	public static String getSdkInfo() {
		return getSDKInfo();
	}
	
	/**
	 * 设置SDK需要的基本信息, 在调用{@link #getInstance()}后不能再更改。<br>
	 * 只有正确的设置了该信息，才能正确使用其它功能.<br>
	 * 
	 * @param context
	 * 		Application context
	 * 		Must have file: assets/lelink/auth.cfg
	 * 
	 * @return
	 * 		true success; false failed
	 */
	public static boolean setContent(Context context) {
		if(context == null){
			LOGE("Context null");
			return false;
		}
		try {
			byte buffer[] = new byte[1024 * 10];
			InputStream in = context.getAssets().open("lelink/auth.cfg");
			int rd = in.read(buffer);
			mInitInfo = Base64.encodeToString(buffer, 0, rd, Base64.NO_WRAP);
		} catch (IOException e) {
			e.printStackTrace();
			return false;
		}
		return true;
	}
	
	/**
	 * 获得LeLink的实例接口<br>
	 * 
	 * @return
	 * 		Lelink实例
	 */
	public static LeLink getInstance() {
		if (sLeLink == null) {
			sLeLink = new LeLink(mInitInfo);
		}
		return sLeLink;
	}

	/**
	 * SDK与云的连接状态.<br>
	 * 只有成功连接云才可以进行信息上报和远程控制.<br>
	 * 
	 * @return
	 * 		true - SDK成功连接到云。false - 没有成功连接到云。
	 */
	public boolean isCloud() {
		return (mState == ST_t.HEART);
	}
	
	/**
	 * 
	 * WIFI配置.<br>
	 * 必须传入参数: ssid, passwd, timeout<br>
	 * 
	 * @param jsonStr
	 * 		String ssid, String passwd, int timeout(sec)
	 * 
	 * @return
	 * 		-1 - error; 0 - success; 1 - timeout
	 */
	public int airConfig(String jsonStr) {
		int startTime, timeout, tryTimes = 1;
		int airConfigType = LeCmd.V.AIR_CONFIG_TYPE_MULTICAST;
		JSONObject sendJson = null;

		if (jsonStr == null) {
			LOGE("airConfig, string null");
			return -1;
		}
		mIsGetDevHello = false;
		startTime = (int) (System.currentTimeMillis() / 1000);
		try {
			sendJson = new JSONObject(jsonStr);
			sendJson.getString(LeCmd.K.SSID);
			sendJson.getString(LeCmd.K.PASSWD);
			sendJson.put(LeCmd.K.DELAY, DEFAULT_AIRCONFIG_DELAY); // ms
			timeout = sendJson.getInt(LeCmd.K.TIMEOUT);
			timeout = timeout < DEFAULT_TIMEOUT ? DEFAULT_TIMEOUT : timeout;
			while (((int) (System.currentTimeMillis() / 1000) - startTime < timeout) && !mIsGetDevHello) {
				String logStr = String.format("AirConfig type = %d tryTimes = %d", airConfigType, tryTimes);
				LOGI(logStr);
				sendJson.put(LeCmd.K.TYPE, airConfigType);
				airConfig(mPtr, sendJson.toString());
				if (tryTimes++ > 5) {
					tryTimes = 1;
					airConfigType = (airConfigType == LeCmd.V.AIR_CONFIG_TYPE_BROADCAST) ? LeCmd.V.AIR_CONFIG_TYPE_MULTICAST
							: LeCmd.V.AIR_CONFIG_TYPE_BROADCAST;
					try {
						Thread.sleep(5000);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}
			}
		} catch (JSONException e) {
			LOGE("Parameter json error");
			e.printStackTrace();
			return -1;
		}
		return mIsGetDevHello ? 0 : 1;
	}

	/**
	 * 设备发现 必须传入timeout<br>
	 * 进入该函数，首先发送一次发现包。然后等待timeout时间，最后返回大这timeout期间收到的发现回复的设备。<br>
	 * 
	 * @param timeout
	 * 		timeout - discover timeout. second
	 * 
	 * @return
	 * 		null - timeout return; else - device array json string
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

	/**
	 * 控制设备状态 必须传入uuid, timeout, token<br>
	 * 如果是传入addr代表通过局域网获得状态。反之，如果没有传入addr表示通过广域网获得状态<br>
	 * 如果该设备已经连接云，则必须传入token(由广域网获得{@link #getState(String)})<br>
	 * 
	 * @param jsonStr
	 * 		Json: String uuid; String addr; String token; int timeout
	 * 
	 * @param dataJson
	 * 		Json: control data
	 * 
	 * @return
	 * 		null - timeout return; else - device array json string
	 */
	public synchronized String ctrl(String jsonStr, String dataJson) {
		int timeout;
		String uuid;
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
		if (!send(cmdJson, dataJson)) {
			LOGE("ctrl send error");
			return null;
		}
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

	/**
	 * 获得设备状态 必须传入uuid, timeout<br>
	 * 如果是传入addr代表通过局域网获得状态。反之，如果没有传入addr表示通过广域网获得状态<br>
	 * 如果是该设备需要远程控制，则必须先通过该函数广域网获得到token<br>
	 * 
	 * @param jsonStr
	 * 		Json - String addr; String uuid; int timeout
	 * 
	 * @return
	 * 		null - timeout return; else - device array json string
	 */
	public String getState(String jsonStr) {
		int timeout;
		String dataStr = null;
		JSONObject cmdJson, sendJson;
		JSONArray jsonArray = new JSONArray();

		synchronized (mFindDevs) {
			mFindDevs.clear();
			cmdJson = new JSONObject();
			try {
				sendJson = new JSONObject(jsonStr);
				timeout = sendJson.getInt(LeCmd.K.TIMEOUT);
				timeout = timeout < 0 ? DEFAULT_TIMEOUT : timeout;
				cmdJson.put(LeCmd.K.TIMEOUT, timeout);
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
				if (cmdJson.has(LeCmd.K.UUID)) {
					JSONObject dataJson = new JSONObject();
					dataStr = cmdJson.getString(LeCmd.K.UUID);
					dataJson.put(LeCmd.K.UUID, dataStr);
					dataStr = dataJson.toString();
				}
			} catch (JSONException e) {
				LOGE("Json error");
				e.printStackTrace();
				return null;
			}

			if(!send(cmdJson, dataStr)){
				LOGE("getState send error");
				return null;
			}
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

	/**
	 * 发送数据<br>
	 * 
	 * @param cmdJson
	 * 		String addr, int cmdId, int subCmdId
	 * 
	 * @param dataStr
	 * 		json userdata
	 * 
	 * @hide
	 */
	private synchronized boolean send(JSONObject cmdJson, String dataStr) {
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
			return false;
		}
		return (send(mPtr, cmdJson.toString()) > 0);
	}

	/**
	 * 得到已经发送的命令的内容<br>
	 * 
	 * @param seqid
	 * 
	 * @return
	 * 
	 * @hide
	 */
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

	/**
	 * 
	 * @param type
	 * @param jsonStr
	 * @param buf
	 * @return
	 * 
	 * @hide
	 */
	private int onMessage(int type, String jsonStr, byte buf[]) {
		int ret = 0;
		int cmd, subcmd, seqId;
		String addr, uuid, dataStr;
		JSONObject cmdJson, sendCmdJson, dataJson;

		try {
			cmdJson = new JSONObject(jsonStr);
			cmd = cmdJson.getInt(LeCmd.K.CMD);
			subcmd = cmdJson.getInt(LeCmd.K.SUBCMD);
			addr = cmdJson.getString(LeCmd.K.ADDR);
			uuid = cmdJson.getString(LeCmd.K.UUID);
			seqId = cmdJson.getInt(LeCmd.K.SEQID);
		} catch (JSONException e) {
			LOGE("Json error");
			e.printStackTrace();
			return ret;
		}
		sendCmdJson = new JSONObject();
//		LOGI("type " + type + ", onMessage: " + jsonStr);
		switch (type) {
		case MSG_TYPE_LOCALREQUEST:
		case MSG_TYPE_LOCALRESPOND:
			dataStr = getAndRemoveSendCmd(seqId);
			if (dataStr != null) {
				LOGI("SendData:\n" + dataStr);
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
//				LOGI("Json:\n" + dataStr);
				if (cmd == LeCmd.CTRL_RSP || cmd == LeCmd.CLOUD_MSG_CTRL_C2R_RSP) {
					mWaitCtrlBackData = dataStr;
					synchronized (mCtrlLock) {
						mCtrlLock.notifyAll();
					}
				} else if (cmd == LeCmd.DISCOVER_RSP || cmd == LeCmd.CLOUD_REPORT_RSP) {
					dataJson = new JSONObject(dataStr);
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
					dataJson = new JSONObject(dataStr);
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

	private static void LOGD(String msg) {
		logout(Log.DEBUG, msg);
	}

	private static void LOGI(String msg) {
		logout(Log.INFO, msg);
	}

	private static void LOGE(String msg) {
		logout(Log.ERROR, msg);
	}

	private static void logout(int priority, String msg) {
		Log.println(priority, TAG, msg);
	}

	private enum ST_t {
		AUTH1, AUTH2, HEART,
	};

	/*********************************************** for native ********************************************************/
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
	private native void airConfig(long ptr, String jsonStr);

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
//        try{
//            System.loadLibrary("lelink"); 
//        }catch(Throwable ex){
//            ex.printStackTrace();
//        }
	}
}
