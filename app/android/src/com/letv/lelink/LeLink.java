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
 * @version 0.1
 * 
 * @author feiguoyou@le.com
 */
public class LeLink {

	private static final String VERSION = "0.1"; // 与以上的注释一致
	private static LeLink sLeLink = null;
	private static final String TAG = "LeLinkJar";
	private static final int MAX_WAIT_CMD = 10;
	private static final int DEFAULT_TIMEOUT = 3;
	private static final int DEFAULT_AIRCONFIG_DELAY = 10;
	private static final long SEND_HEART_TIME = 1000 * 20;
	private static final long CLOUD_HEART_RESPOND_TIMEOUT = 1000 * 60;

	private int mSeqId = 0;
	private ST_t mState = ST_t.AUTH1;
	private boolean mIsGetDevHello = false;
	private long mSendHeartTime = 0;
	private long mGetCloudHeartRspTime = 0;
	private static String mInitInfo = null;
	private static String mSdkInfo = null;
	private static Listener mListener = null;
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
	 * @return SDK信息
	 */
	public static String getSdkInfo() {
		if (mSdkInfo == null) {
			mSdkInfo = getSDKInfo();
			if (mSdkInfo != null) {
				JSONObject infoJson = null;
				try {
					infoJson = new JSONObject(mSdkInfo);
					infoJson.put(LeCmd.K.JARVER, VERSION);
					mSdkInfo = infoJson.toString();
				} catch (JSONException e) {
					e.printStackTrace();
				}
			}
		}
		return mSdkInfo;
	}

	/**
	 * 设置SDK需要的基本信息, 在调用{@link #getInstance()}后不能再更改。<br>
	 * 只有正确的设置了该信息，才能正确使用其它功能.<br>
	 * 
	 * @param context
	 *            Application context<br>
	 *            要求认证文件存在: assets/lelink/auth.cfg<br>
	 *            
	 * @param listener
	 * 			  LeLink.Listener <br>
	 * 			  SDK 状态通知监听器. <br>
	 * 
	 * @return true - 设置正确; false - 设置失败 
	 */
	public static boolean setContext(Context context, Listener listener) {
		if (context == null) {
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
		mListener = listener;
		return true;
	}

	/**
	 * @hide
	 */
	public static Listener getListener()
	{
		return mListener;
	}
	
	/**
	 * 获得LeLink的实例接口<br>
	 * 
	 * @return Lelink实例
	 */
	public static LeLink getInstance() {
		if (sLeLink == null) {
			sLeLink = new LeLink(mInitInfo);
			mSdkInfo = getSDKInfo();
		}
		return sLeLink;
	}

	/**
	 * SDK与云的连接状态.<br>
	 * 只有成功连接云才可以进行信息上报和远程控制.<br>
	 * 
	 * @return true - SDK成功连接到云。false - 没有成功连接到云。
	 */
	public boolean isCloud() {
		if (mState == ST_t.HEART && System.currentTimeMillis() - mGetCloudHeartRspTime > CLOUD_HEART_RESPOND_TIMEOUT) {
			mState = ST_t.AUTH1;
			if (mListener != null) {
				mListener.onCloudStateChange(false);
			}
		}
		return (mState == ST_t.HEART);
	}

	/**
	 * 得到SDK对应的UUID.<br>
	 * 必须在成功执行setContext()后才可以用.<br>
	 * 
	 * @return String - SDK UUID
	 */
	public String getSdkUUID() {
		String uuid = null;
		// LOGI(mSdkInfo);
		try {
			JSONObject obj = new JSONObject(mSdkInfo);
			uuid = obj.getString(LeCmd.K.UUID);
		} catch (JSONException e) {
			e.printStackTrace();
			return null;
		}
		return uuid;
	}

	/**
	 * 
	 * WIFI配置.<br>
	 * 必须传入参数: ssid, passwd, timeout<br>
	 * 
	 * @param jsonStr
	 *            String ssid, String passwd, int timeout(sec)
	 * 
	 * @return -1 - 错误; 0 - 成功; 1 - 超时
	 */
	public int airConfig(String jsonStr) {
		int startTime, timeout, tryTimes = 1;
		int airConfigType = LeCmd.V.AIR_CONFIG_TYPE_MULTICAST;
		// int airConfigType = LeCmd.V.AIR_CONFIG_TYPE_BROADCAST;
		// int airConfigType = LeCmd.V.AIR_CONFIG_TYPE_SOFTAP;
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
			sendJson.put(LeCmd.K.TIMEOUT, timeout);
			if (sendJson.has(LeCmd.K.TYPE)) {
				airConfigType = sendJson.getInt(LeCmd.K.TYPE);
			}
			while (((int) (System.currentTimeMillis() / 1000) - startTime < timeout) && !mIsGetDevHello) {
				String logStr = String.format("AirConfig type = %d tryTimes = %d", airConfigType, tryTimes);
				LOGI(logStr);
				sendJson.put(LeCmd.K.TYPE, airConfigType);
				airConfig(mPtr, sendJson.toString());
				tryTimes++;
			}
		} catch (JSONException e) {
			LOGE("Parameter json error");
			e.printStackTrace();
			return -1;
		}
		LOGW("airConfig " + (mIsGetDevHello ? "ok" : "timeout"));
		return mIsGetDevHello ? 0 : 1;
	}

	/**
	 * 设备发现 必须传入timeout<br>
	 * 进入该函数，首先发送一次发现包。然后等待timeout时间，最后返回大这timeout期间收到的发现回复的设备。<br>
	 * 
	 * @param timeout
	 *            timeout - 超时时间，单位秒
	 * 
	 * @return 发现的设备JSON数组
	 */
	public String discover(int timeout) {
		JSONObject cmdJson;
		try {
			cmdJson = new JSONObject();
			cmdJson.put(LeCmd.K.SUBCMD, LeCmd.Sub.GET_STATE_CMD);
			cmdJson.put(LeCmd.K.ADDR, LeCmd.V.BROADCAST_ADDR);
			cmdJson.put(LeCmd.K.TIMEOUT, timeout);
		} catch (JSONException e) {
			LOGE("Json error");
			e.printStackTrace();
			return null;
		}
		return getState(cmdJson.toString(), null);
	}

	/**
	 * 获得状态<br>
	 * 
	 * @param cmdStr
	 *            详细参考下述说明
	 * @param dataStr
	 *            详细参考下述说明<br>
	 * <br>
	 * 
	 *            cmdStr 中必须要有键值 LeCmd.K.SUBCMD(int),
	 *            根据该值的不同，cmdStr/dataStr包含的内容不同, 返回值也不同<br>
	 *            cmdStr 中必须要有键值 LeCmd.K.UUID(String), 设备的UUID<br>
	 *            cmdStr 中必须要有键值 LeCmd.K.TIMEOUT(int), 设备超时时间, 单位秒<br>
	 * <br>
	 *            1, LeCmd.K.SUBCMD == LeCmd.Sub.GET_STATE_CMD, 得到设备状态或者上报设备状态<br>
	 *            cmdStr:<br>
	 *            a, LeCmd.K.ADDR(String), 设置的地址，如果设置则为局域网得到设备状态<br>
	 *            dataStr:<br>
	 *            a, LeCmd.K.UUID(String), 设备的UUID<br>
	 *            return<br>
	 *            a, 出错返回null<br>
	 *            b, 设备列表的Json字符串<br>
	 *            2, LeCmd.K.SUBCMD == LeCmd.Sub.CLOUD_REPORT_OTA_QUERY_REQ,
	 *            OTA查询<br>
	 *            cmdStr:<br>
	 *            不包括其它键值<br>
	 *            dataStr:<br>
	 *            a, LeCmd.K.UUID(String), 设备的UUID<br>
	 *            b, LeCmd.K.VERSION(String), 设备的当前版本号<br>
	 *            c, LeCmd.K.TYPE(int), 查询的类型(OTA_TYPE_FIRMWARE-固件,
	 *            OTA_TYPE_FW_SCRIPT-固件脚本, OTA_TYPE_IA_SCRIPT-联动脚本)<br>
	 *            d, LeCmd.K.IAID(String), 如果LeCmd.K.TYPE的值是5(联动脚本)的时候需要填充该值<br>
	 *            return<br>
	 *            a, 出错返回null<br>
	 *            b, 查询信息列表的Json字符串, 键值有LeCmd.K.URL(String)<br>
	 * 
	 * @return 详细参考上述说明
	 */
	public String getState(String cmdStr, String dataStr) {
		JSONObject cmdJson;
		int timeout, subcmd;
		String retData = null;
		boolean isDiscover = false;

		try {
			cmdJson = new JSONObject(cmdStr);
			subcmd = cmdJson.getInt(LeCmd.K.SUBCMD);
		} catch (JSONException e) {
			e.printStackTrace();
			return null;
		}
		synchronized (mFindDevs) {
			mFindDevs.clear();
			try {
				if (cmdJson.getInt(LeCmd.K.SUBCMD) == LeCmd.Sub.GET_STATE_CMD && cmdJson.has(LeCmd.K.ADDR)) { // 特别处理本地发现
					cmdJson.put(LeCmd.K.CMD, LeCmd.DISCOVER_REQ);
					cmdJson.put(LeCmd.K.SUBCMD, LeCmd.Sub.DISCOVER_REQ);
					if (cmdJson.getString(LeCmd.K.ADDR).equals(LeCmd.V.BROADCAST_ADDR)) {
						isDiscover = true;
					}
				} else {
					cmdJson.put(LeCmd.K.CMD, LeCmd.CLOUD_REPORT_REQ);
				}
				timeout = cmdJson.getInt(LeCmd.K.TIMEOUT);
				timeout = timeout < 0 ? DEFAULT_TIMEOUT : timeout;
				cmdJson.put(LeCmd.K.TIMEOUT, timeout);
				mWaitGetUuid = cmdJson.has(LeCmd.K.UUID) ? cmdJson.getString(LeCmd.K.UUID) : null;
			} catch (JSONException e) {
				LOGE("Json error");
				e.printStackTrace();
				return null;
			}
			if (!send(cmdJson, dataStr)) {
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
				if(!isDiscover && mListener != null) {
					mListener.onGetStateBack(subcmd, mWaitGetUuid, null);
				}
				return null;
			}
			if (isDiscover) {
				JSONArray jsonArray = new JSONArray();
				for (JSONObject v : mFindDevs.values()) {
					jsonArray.put(v);
				}
				retData = jsonArray.toString();
			} else {
				for (JSONObject v : mFindDevs.values()) {
					retData = v.toString();
					break;
				}
			}
		}
		return retData;
	}

	/**
	 * 控制设备<br>
	 * 
	 * @param cmdStr
	 *            详细参考下述说明
	 * @param dataStr
	 *            详细参考下述说明<br>
	 * <br>
	 * 
	 *            cmdStr 中必须要有键值 LeCmd.K.SUBCMD(int),
	 *            根据该值的不同，cmdStr/dataStr包含的内容不同, 返回值也不同<br>
	 *            cmdStr 中必须要有键值 LeCmd.K.UUID(String), 设备的UUID<br>
	 *            cmdStr 中必须要有键值 LeCmd.K.TIMEOUT(int), 设备超时时间, 单位秒<br>
	 * <br>
	 * 
	 *            1, LeCmd.K.SUBCMD == LeCmd.Sub.CTRL_DEV_CMD, 控制设备<br>
	 *            cmdStr:<br>
	 *            a, LeCmd.K.ADDR(String), 设置的地址，如果设置则为局域网控制设备<br>
	 *            b, LeCmd.K.TOKEN(String), 设备的token, 如果不是局域网控制，则必须传入该值<br>
	 *            dataStr:<br>
	 *            a, 控制设备的Json字符串<br>
	 *            return<br>
	 *            a, 出错返回null<br>
	 *            b, 设备列表的Json字符串<br>
	 *            2, LeCmd.K.SUBCMD == LeCmd.Sub.CLOUD_MSG_CTRL_C2R_DO_OTA_REQ,
	 *            要求设备进行OTA升级<br>
	 *            cmdStr:<br>
	 *            a, LeCmd.K.TOKEN(String), 设备的token<br>
	 *            dataStr:<br>
	 *            a, 填充由OTA查询得到的Json字符串<br>
	 *            return<br>
	 *            a, 成功返回字符串"ok"<br>
	 *            b, 失败返回失败说明字符串<br>
	 * 
	 * @return 详细参考上述说明
	 */
	public synchronized String ctrl(String cmdStr, String dataStr) {
		int timeout, subcmd;
		JSONObject cmdJson = null;

		try {
			cmdJson = new JSONObject(cmdStr);
			subcmd = cmdJson.getInt(LeCmd.K.SUBCMD);
			if (cmdJson.getInt(LeCmd.K.SUBCMD) == LeCmd.Sub.CTRL_DEV_CMD && cmdJson.has(LeCmd.K.ADDR)) {
				cmdJson.put(LeCmd.K.CMD, LeCmd.CTRL_REQ);
				cmdJson.put(LeCmd.K.SUBCMD, LeCmd.Sub.CTRL_CMD_REQ);
			} else {
				cmdJson.put(LeCmd.K.CMD, LeCmd.CLOUD_MSG_CTRL_C2R_REQ);
			}
			timeout = cmdJson.getInt(LeCmd.K.TIMEOUT);
			timeout = timeout < 0 ? DEFAULT_TIMEOUT : timeout;
			cmdJson.put(LeCmd.K.TIMEOUT, timeout);
			mWaitCtrlUuid = cmdJson.getString(LeCmd.K.UUID);
		} catch (JSONException e) {
			LOGE("Json error");
			e.printStackTrace();
			return null;
		}
		mWaitCtrlBackData = null;
		if (!send(cmdJson, dataStr)) {
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
			if (mListener != null) {
				mListener.onCtrlBack(subcmd, mWaitCtrlUuid, null);
			}
		}
		return mWaitCtrlBackData;
	}

	/**
	 * 发送数据<br>
	 * 
	 * @param cmdJson
	 *            Json中包含值: String addr; int cmdId; int subCmdId
	 * 
	 * @param dataStr
	 * 			  发送的数据
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
				String jsonStr;
				try {
					com.alibaba.fastjson.JSONObject dataFJson = com.alibaba.fastjson.JSONObject.parseObject(dataStr);
					jsonStr = dataFJson.toJSONString();
				} catch (Exception e) {
					LOGE("Data json error");
					e.printStackTrace();
					return false;
				}
				mWaitSendCmds.put(String.valueOf(mSeqId + 1), jsonStr);
			}
		}
		mSeqId++;
		try {
			cmdJson.put(LeCmd.K.SEQID, mSeqId);
		} catch (JSONException e) {
			LOGE("Cmd json error");
			e.printStackTrace();
			return false;
		}
		LOGI("CmdJson:\n" + cmdJson.toString());
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
	 * 			消息类型
	 * 
	 * @param jsonStr
	 * 			消息参数
	 * 
	 * @param buf
	 * 			消息内容或者需要填充数据的缓冲区
	 * 
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
		LOGI("Type " + type + ", UUID " + uuid + ", Size " + buf.length + ", onMessage: " + jsonStr);
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
				if (mListener != null) {
					mListener.onAirConfigBack(uuid);
				}
			}
			if ((cmd == LeCmd.DISCOVER_REQ && subcmd == LeCmd.Sub.DISCOVER_STATUS_CHANGED_REQ)
					|| (cmd == LeCmd.CLOUD_IND_REQ && subcmd == LeCmd.Sub.CLOUD_IND_STATUS_REQ)) {
				try {
					dataStr = new String(buf, "UTF-8");
					if (mListener != null) {
						mListener.onStateChange(uuid, dataStr);
					}
				} catch (UnsupportedEncodingException e) {
					e.printStackTrace();
					return ret;
				}
			}
			break;
		case MSG_TYPE_REMOTERESPOND:
			try {
				dataStr = new String(buf, "UTF-8");
				LOGI("Json(" + buf.length + "):\n" + dataStr);
				if (cmd == LeCmd.CTRL_RSP || cmd == LeCmd.CLOUD_MSG_CTRL_C2R_RSP) {
					if (uuid.indexOf(mWaitCtrlUuid) < 0) {
						LOGW("Ctrl respond, but uuid: " + uuid + ", need: " + mWaitCtrlUuid);
						break;
					}
					uuid = mWaitCtrlUuid;
					if (cmd == LeCmd.CLOUD_MSG_CTRL_C2R_RSP && subcmd == LeCmd.Sub.CLOUD_MSG_CTRL_C2R_DO_OTA_RSP) {
						mWaitCtrlBackData = (buf.length == 0) ? "ok" : dataStr;
					} else {
						mWaitCtrlBackData = dataStr;
					}
					synchronized (mCtrlLock) {
						mCtrlLock.notifyAll();
					}
					if (mListener != null) {
						if ((cmd == LeCmd.CTRL_RSP && subcmd == LeCmd.Sub.CTRL_CMD_RSP)
								|| (cmd == LeCmd.CLOUD_MSG_CTRL_C2R_RSP && subcmd == LeCmd.Sub.CLOUD_MSG_CTRL_C2R_RSP)) {
							mListener.onCtrlBack(LeCmd.Sub.CTRL_DEV_CMD, uuid, dataStr);
						} else {
							mListener.onCtrlBack(subcmd, uuid, dataStr);
						}
					}
				} else if (cmd == LeCmd.DISCOVER_RSP || cmd == LeCmd.CLOUD_REPORT_RSP) {
					// LOGI("Data:\n" + dataStr);
					dataJson = new JSONObject(dataStr);
					uuid = dataJson.getString(LeCmd.K.UUID);
					if (mWaitGetUuid != null) {
						if (uuid.indexOf(mWaitGetUuid) >= 0) {
							mFindDevs.clear();
							mFindDevs.put(uuid, dataJson);
							synchronized (mGetLock) {
								mGetLock.notifyAll();
							}
						} else { // getstate not
							LOGW("getState respond, but uuid: " + uuid + ", need: " + mWaitGetUuid);
						}
						if (mListener != null) {
							if ((cmd == LeCmd.DISCOVER_RSP && subcmd == LeCmd.Sub.DISCOVER_RSP)
									|| (cmd == LeCmd.CLOUD_REPORT_RSP && subcmd == LeCmd.Sub.CLOUD_REPORT_RSP)) {
								mListener.onGetStateBack(LeCmd.Sub.GET_STATE_CMD, uuid, dataStr);
							} else {
								mListener.onGetStateBack(subcmd, uuid, dataStr);
							}
						}
					} else {
						mFindDevs.put(uuid, dataJson);
						if (mListener != null) {
							mListener.onDiscoverBack(uuid, dataStr);
						}
					}
				} else if (cmd == LeCmd.CLOUD_GET_TARGET_RSP && subcmd == LeCmd.Sub.CLOUD_GET_TARGET_RSP) {
					dataJson = new JSONObject(dataStr);
					if (dataJson.has(LeCmd.K.REMOTE_ADDR)) {
						mState = ST_t.AUTH2;
					} else {
						// waiting heart back;
					}
				} else if (cmd == LeCmd.CLOUD_AUTH_RSP && subcmd == LeCmd.Sub.CLOUD_AUTH_RSP) {
					// waiting heart back;
				} else if (cmd == LeCmd.CLOUD_HEARTBEAT_RSP && subcmd == LeCmd.Sub.CLOUD_HEARTBEAT_RSP) {
					mGetCloudHeartRspTime = System.currentTimeMillis();
					if (mState != ST_t.HEART) {
						mState = ST_t.HEART;
						if (mListener != null) {
							mListener.onCloudStateChange(true);
						}
					}
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

	/**
	 * 
	 * Lelink 事件监听器 <br>
	 * 
	 */
	public interface Listener {

		/**
		 * SDK云端接入状态变化 <br>
		 * 
		 * @param isCloud
		 * 			表示是否成功连接上云服务器<br>
		 */
		void onCloudStateChange(boolean isCloud);

		/**
		 * wifi配置时，成功配置了设备 <br>
		 * 
		 * @param uuid
		 * 			device uuid<br>
		 */
		void onAirConfigBack(String uuid);

		/**
		 * 设备发现 <br>
		 * 
		 * @param uuid
		 * 			device uuid<br>
		 * 
		 * @param dataStr
		 * 			设备的状态<br>
		 * 			
		 */
		void onDiscoverBack(String uuid, String dataStr);

		/**
		 * 设备状态获取回复 <br>
		 * 
		 * @param subcmd
		 * 			子命令类型<br>
		 * 
		 * @param uuid
		 * 			device uuid<br>
		 * 
		 * @param dataStr
		 * 			设备的状态, 超时为 null<br>
		 * 			
		 */
		void onGetStateBack(int subcmd, String uuid, String dataStr);

		/**
		 * 设备状态控制回复 <br>
		 * 
		 * @param subcmd
		 * 			子命令类型<br>
		 * 
		 * @param uuid
		 * 			device uuid<br>
		 * 
		 * @param dataStr
		 * 			设备的状态, 超时为 null<br>
		 * 			
		 */
		void onCtrlBack(int subcmd, String uuid, String dataStr);

		/**
		 * 设备状态变化通知 <br>
		 * 
		 * @param uuid
		 * 			device uuid<br>
		 * 
		 * @param dataStr
		 * 			设备的状态<br>
		 * 			
		 */
		void onStateChange(String uuid, String dataStr);
	}

	private static void LOGD(String msg) {
		logout(Log.DEBUG, msg);
	}

	private static void LOGI(String msg) {
		logout(Log.INFO, msg);
	}

	private static void LOGW(String msg) {
		logout(Log.WARN, msg);
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
	 * @return SDK information
	 * 
	 * @hide
	 */
	private static native String getSDKInfo();

	/**
	 * 
	 * @param jsonStr
	 *            null
	 * 
	 * @return native C pointer
	 * 
	 * @hide
	 */
	private native long init(String jsonStr);

	/**
	 * 
	 * @param ptr
	 * 
	 * @param jsonStr
	 *            String ssid; String passwd; int delay(ms); int
	 *            type(1-mutilcast, 2-broadcast)
	 * 
	 * @hide
	 */
	private native int airConfig(long ptr, String jsonStr);

	/**
	 * 
	 * @param ptr
	 * 
	 * @param jsonStr
	 *            String addr; int cmdId; int subCmdId; int seqid
	 *
	 * @hide
	 */
	private native int send(long ptr, String jsonStr);

	static {
		System.loadLibrary("lelink");
		// try{
		// System.loadLibrary("lelink");
		// }catch(Throwable ex){
		// ex.printStackTrace();
		// }
	}
}
