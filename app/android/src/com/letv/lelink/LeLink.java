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

/**
 * 
 * Lelink Android平台接入SDK接口<br>
 * Copyright © 2004-2016 乐视网（letv.com）All rights reserved.<br>
 * 
 * @version 2.1
 * 
 * @author feiguoyou@le.com
 */
public class LeLink {

	/*
	 * 2.1, 接口更改
	 * 1.0, 添加airhug配置入网方式
	 * 0.9, 添加日志，方便调试
	 * 0.8, Zigbee的uuid去掉mac
	 * 0.7, Zigbee设备状态BUG
	 * 0.6, 解决启动了两个线程的BUG
	 * 0.5,
	 * 0.4, 优化wifi配置
	 * 0.3, 添加Listener onControl()
	 * 0.2, 添加Listener onPushMessage()
	 * 0.1, 添加Listener
	 */
	private static final String VERSION = "2.1"; // 与以上的注释一致
	private static final String TAG = "LeLinkJar";
	private static LeLink sLeLink = null;
	private static boolean isAuthed = false;
	private static final int MAX_WAIT_CMD = 5;
	private static final int DEFAULT_TIMEOUT = 3;
	private static final int DEFAULT_AIRCONFIG_DELAY = 10;
	private static final long SEND_HEART_TIME = 1000 * 20;
	private static final long CLOUD_HEART_RESPOND_TIMEOUT = 1000 * 60;

	private int mSeqId = 0;
	private int mState = LeCmd.State.NONE;
	private boolean mIsGetDevHello = false;
	private long mSendHeartTime = 0;
	private Listener mListener = null;
	private static String mSdkInfo = null;
	// for get & discover
	private String mWaitGetUuid = null;
	private Map<String, JSONObject> mFindDevs = new HashMap<String, JSONObject>(); // uuid
	// for send
	private Lock mGetLock = new ReentrantLock();
	private Map<String, String> mWaitSendCmds = new HashMap<String, String>(); // seqid
	// for ctrl
	private String mWaitCtrlUuid = null;
	private String mWaitCtrlBackData = null;
	private Lock mCtrlLock = new ReentrantLock();

	/**
	 * 获得LeLink SDK的信息.<br>
	 * 
	 * @return SDK信息
	 */
	public static String getSdkInfo() {
		if (mSdkInfo == null) {
			mSdkInfo = getSDKInfo();
		}
		return mSdkInfo;
	}

	/**
	 * 获得LeLink的实例接口.<br>
	 * 
	 * @return Lelink实例
	 */
	public static LeLink getInstance(String authStr, String macStr) {
        return getInstance(authStr, macStr, null);
	}
	
	/**
	 * 获得LeLink的实例接口.<br>
     * 
	 * 设置SDK需要的基本信息, 只有正确的设置了该信息, 才能正确使用其它功能.<br>
	 * 
	 * @param authStr
	 *            Auth String<br>
	 *            Lelink的认证信息，要求符合官方格式<br>
	 *            
	 * @param macStr
	 * 			  本机的MAC地址<br>
     * 			  
	 * @param listener
	 * 			  LeLink.Listener <br>
	 * 			  SDK 状态通知监听器. <br>
	 * 
	 * @return Lelink实例
	 */
	public static LeLink getInstance(String authStr, String macStr, Listener listener) {
		String infoJson = null;
//		String macStr = "11:22:33:44:55:66";
		JSONObject jsonObj = null;
		
		synchronized (TAG) {
			if (sLeLink != null) {
				sLeLink.setListener(listener);
				return sLeLink;
			}
			if (macStr == null || !macStr.matches("([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}")) {
				LOGE("Mac address error");
				return null;
			}
			jsonObj = new JSONObject();
			try {
				// jsonObj.put(LeCmd.K.SCRIPT, scriptStr);
				jsonObj.put(LeCmd.K.AUTH, authStr);
				jsonObj.put(LeCmd.K.MAC, macStr);
			} catch (JSONException e1) {
				e1.printStackTrace();
				return null;
			}
			infoJson = jsonObj.toString();
			sLeLink = new LeLink(infoJson, listener);
			mSdkInfo = getSDKInfo();
			LOGI(mSdkInfo);
			try {
				jsonObj = new JSONObject(mSdkInfo);
				jsonObj.put(LeCmd.K.JARVER, VERSION);
				mSdkInfo = jsonObj.toString();
			} catch (JSONException e) {
				e.printStackTrace();
			}
			LOGI(mSdkInfo);
		}
        if(!isAuthed) {
            sLeLink = null;
        }
		return sLeLink;
	}
	
	/**
	 * @hide
	 */
	public Listener getListener()
	{
		return mListener;
	}
	
	/**
	 * @hide
	 */
	public void setListener(Listener listener)
	{
		synchronized(this) {
			mListener = listener;
		}
	}
	
	/**
	 * SDK状态获取.<br>
	 * 
	 * @return LeCmd.State.xxx
	 */
	public int getState() // 
	{
		return mState;
	}
	
	/**
	 * 得到SDK对应的UUID.<br>
	 * 
	 * @return String - SDK UUID
	 */
	public String getSdkUUID() {
		String uuid = null;
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
	 * 必须传入参数: ssid, passwd, timeout.<br>
	 * 
	 * @param jsonStr
	 *            String ssid, String passwd, String apSsid(only for type == LeCmd.V.AIR_CONFIG_TYPE_SOFTAP), int type, int timeout(sec)<br>
	 *            <p>
	 *            type(LeCmd.K.TYPE)有四种方式: <br>
	 *            <ol>
	 *            		<li>LeCmd.V.AIR_CONFIG_TYPE_MULTICAST (默认该方式) </li>
	 *            		<li>LeCmd.V.AIR_CONFIG_TYPE_BROADCAST </li>
	 *            		<li>LeCmd.V.AIR_CONFIG_TYPE_SOFTAP </li>
	 *            		<li>LeCmd.V.AIR_CONFIG_TYPE_AIRHUG </li>
	 *            </ol>
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
		if (!isAuthed) {
			LOGE("Error, Authentication failed!!");
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
			if (airConfigType == LeCmd.V.AIR_CONFIG_TYPE_SOFTAP) {
				sendJson.getString(LeCmd.K.APSSID);
			}
			String logStr = String.format("AirConfig type = %d\n%s", airConfigType, sendJson.toString());
			LOGI(logStr);
			while (((int) (System.currentTimeMillis() / 1000) - startTime < timeout) && !mIsGetDevHello) {
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
	 * 设备发现 必须传入timeout.<br>
	 * 进入该函数, 首先发送一次发现包. 然后等待timeout时间, 最后返回大这timeout期间收到的发现回复的设备.<br>
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
	 * 获得状态.<br>
	 * 
	 * @param cmdStr
	 *            详细参考下述说明
	 * @param dataStr
	 *            详细参考下述说明<br>
	 * <p>
	 * cmdStr Json 必须包含:
	 * <ol>
	 *  <li>LeCmd.K.SUBCMD(int), 根据该值的不同，cmdStr/dataStr包含的内容不同, 返回值也不同</li>
	 *  <li>LeCmd.K.UUID(String), 设备的UUID</li>
	 *  <li>LeCmd.K.TIMEOUT(int), 设备超时时间, 单位秒</li>
	 * </ol>
	 * <p>
	 * <dl>
	 *  <dd>1, LeCmd.K.SUBCMD == LeCmd.Sub.GET_STATE_CMD, 得到设备状态或者上报设备状态
	 *   <ul>
	 *    <li>cmdStr:
	 *     <dl>
	 *       <dd>LeCmd.K.ADDR(String), 设置的地址，如果设置则为局域网得到设备状态</dd>
	 *     </dl>
	 *    </li>
	 *    <li>dataStr:
	 *     <dl>
	 *      <dd>LeCmd.K.UUID(String), 设备的UUID</dd>
	 *     </dl>
	 *    </li>
	 *    <li>return
	 *     <dl>
	 *      <dd>出错返回null</dd>
	 *      <dd>设备列表的Json字符串</dd>
	 *     </dl>
	 *    </li>
	 *   </ul>
	 *  </dd>
	 *  <dd>2, LeCmd.K.SUBCMD == LeCmd.Sub.CLOUD_REPORT_OTA_QUERY_REQ, OTA查询
	 *   <ul>
	 *    <li>cmdStr:
	 *     <dl>
	 *      <dd>不包括其它键值</dd>
	 *     </dl>
	 *    </li>
	 *    <li>dataStr:
	 *     <dl>
	 *      <dd>LeCmd.K.UUID(String), 设备的UUID</dd>
	 *      <dd>LeCmd.K.VERSION(String), 设备的当前版本号</dd>
	 *      <dd>LeCmd.K.TYPE(int), 查询的类型(OTA_TYPE_FIRMWARE-固件, OTA_TYPE_FW_SCRIPT-固件脚本, OTA_TYPE_IA_SCRIPT-联动脚本</dd>
	 *      <dd>LeCmd.K.IAID(String), 如果LeCmd.K.TYPE的值是5(联动脚本)的时候需要填充该</dd>
	 *     </dl>
	 *    </li>
	 *    <li>return
	 *      <dd>出错返回nul</dd>
	 *      <dd>键值有LeCmd.K.URL(String</dd>
	 *    </li>
	 *   </ul>
	 *  </dd>
	 * </dl>
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
			LOGI("Wait over! for '" + mWaitGetUuid + "'");
			if (mFindDevs.size() <= 0) {
				synchronized(this) {
					if(!isDiscover && mListener != null) {
						mListener.onGetStateBack(subcmd, mWaitGetUuid, null);
					}
				}
				LOGI("Wait timeout!!");
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
	 * 控制设备
	 * 
	 * @param cmdStr
	 *            详细参考下述说明
	 * @param dataStr
	 *            详细参考下述说明<br>
	 * <p>
	 * cmdStr Json 必须包含:
	 * <ol>
	 *  <li>LeCmd.K.SUBCMD(int), 根据该值的不同，cmdStr/dataStr包含的内容不同, 返回值也不同</li>
	 *  <li>LeCmd.K.UUID(String), 设备的UUID</li>
	 *  <li>LeCmd.K.TIMEOUT(int), 设备超时时间, 单位秒</li>
	 * </ol>
	 * <p>
	 * cmdStr Json 其它包含:
	 * <dl>
	 *  <dd>1, 控制设备(LeCmd.K.SUBCMD == LeCmd.Sub.CTRL_DEV_CMD)
	 *  <br><br>
	 *    <ul>
	 *     <li> cmdStr:
	 *      <dl>
	 *       <dd>LeCmd.K.ADDR(String), 设置的地址，如果设置则为局域网控制设备</dd>
	 *       <dd>LeCmd.K.TOKEN(String), 设备的token, 如果不是局域网控制，则必须传入该值</dd>
	 *      </dl>
	 *     <li> dataStr:
	 *      <dl>
	 *       <dd>控制设备的Json字符串</dd>
	 *      </dl>
	 *     </li>
	 *     <li> return
	 *      <dl>
	 *       <dd>出错返回null</dd>
	 *       <dd>设备列表的Json字符串</dd>
	 *      </dl>
	 *     </li>
	 *    </ul>
	 *  </dd>
	 *  <br>
	 * <dd>2, 要求设备进行OTA升级(LeCmd.K.SUBCMD == LeCmd.Sub.CLOUD_MSG_CTRL_C2R_DO_OTA_REQ)
	 *  <br><br>
	 *    <ul>
	 *     <li>cmdStr:
	 *      <dl>
	 *       <dd>LeCmd.K.TOKEN(String), 设备的token</dd>
	 *      </dl>
	 *     </li>
	 *     <li>dataStr:
	 *      <dl>
	 *       <dd>填充由OTA查询得到的Json字符串</dd>
	 *      </dl>
	 *     </li>
	 *     <li>return
	 *      <dl>
	 *       <dd>成功返回字符串"ok"</dd>
	 *       <dd>失败返回失败说明字符串</dd>
	 *      </dl>
	 *    </ul>
	 *  </dd>
	 * </dl>
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
			synchronized(this) {
				if (mListener != null) {
					mListener.onCtrlBack(subcmd, mWaitCtrlUuid, null);
				}
			}
		}
		return mWaitCtrlBackData;
	}

	/**
	 * 发送数据.<br>
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
		mSeqId++;
		if (mSeqId >= 0xFFFF) {
			mSeqId = 1;
			mWaitSendCmds.clear();
		}
		synchronized (mWaitSendCmds) {
			String keyStr;
			int min = 1;
			if (mWaitSendCmds.size() >= MAX_WAIT_CMD) {
				for (min = 1; min < mSeqId; min++) {
					keyStr = String.valueOf(min);
					if (mWaitSendCmds.get(keyStr) != null) {
						mWaitSendCmds.remove(keyStr);
						break;
					}
				}
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
				mWaitSendCmds.put(String.valueOf(mSeqId), jsonStr);
			}
		}
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
	 * 得到已经发送的命令的内容.<br>
	 * 
	 * @param seqid
	 * 
	 * @return 
	 * 		命令内容
	 * 
	 * @hide
	 */
	private String getAndRemoveSendCmd(int seqid) {
		String dataJson = null;
		String keyStr = String.valueOf(seqid);

		synchronized (mWaitSendCmds) {
			dataJson = mWaitSendCmds.get(keyStr);
			if (dataJson != null) {
				// mWaitSendCmds.remove(keyStr);
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
		int cmd, subcmd, seqId, status;
		String addr, uuid, dataStr;
		JSONObject cmdJson, sendCmdJson, dataJson;

		if (MSG_TYPE_LELINKSTATE == type) {
			mState = buf[1];
			LOGI("State: " + buf[0] + " -> " + buf[1]);
			synchronized(this) {
				if(mListener != null) {
					mListener.onLelinkStateChange(buf[0], buf[1]);
				}
			}
			return ret;
		}
		try {
			cmdJson = new JSONObject(jsonStr);
			cmd = cmdJson.getInt(LeCmd.K.CMD);
			subcmd = cmdJson.getInt(LeCmd.K.SUBCMD);
			addr = cmdJson.getString(LeCmd.K.ADDR);
			uuid = cmdJson.getString(LeCmd.K.UUID);
			seqId = cmdJson.getInt(LeCmd.K.SEQID);
			status = cmdJson.getInt(LeCmd.K.STATUS);
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
			if ((cmd == LeCmd.DISCOVER_REQ && subcmd == LeCmd.Sub.DISCOVER_STATUS_CHANGED_REQ)
					|| (cmd == LeCmd.CLOUD_IND_REQ && subcmd == LeCmd.Sub.CLOUD_IND_STATUS_REQ)) {
				try {
					dataStr = new String(buf, "UTF-8");
					synchronized(this) {
						if (mListener != null) {
							mListener.onStateChange(uuid, dataStr);
						}
					}
				} catch (UnsupportedEncodingException e) {
					e.printStackTrace();
					return ret;
				}
			} else if (cmd == LeCmd.CLOUD_IND_REQ && subcmd == LeCmd.Sub.CLOUD_IND_MSG_REQ) {
				try {
					dataStr = new String(buf, "UTF-8");
					synchronized(this) {
						if (mListener != null) {
							mListener.onPushMessage(dataStr);
						}
					}
				} catch (UnsupportedEncodingException e) {
					e.printStackTrace();
					return ret;
				}
			} else if (cmd == LeCmd.CLOUD_MSG_CTRL_R2T_REQ) {
//				try {
//					dataStr = new String(buf, "UTF-8");
//					synchronized(this) {
//					if (mListener != null) {
//						mListener.onControl(subcmd, uuid, dataStr);
//					}
//					}
//				} catch (UnsupportedEncodingException e) {
//					e.printStackTrace();
//					return ret;
//				}
			} else if (cmd == LeCmd.HELLO_REQ && subcmd == LeCmd.Sub.HELLO_REQ) {
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
				synchronized (this) {
					if (mListener != null) {
						try {
							dataStr = new String(buf, "UTF-8");
						} catch (UnsupportedEncodingException e) {
							e.printStackTrace();
							return ret;
						}
						mListener.onAirConfigBack(uuid, dataStr);
					}
				}
			}
			break;
		case MSG_TYPE_REMOTERESPOND:
			try {
				dataStr = new String(buf, "UTF-8");
				// LOGI("Json(" + buf.length + "):%s\n", dataStr);
				if (cmd == LeCmd.CTRL_RSP || cmd == LeCmd.CLOUD_MSG_CTRL_C2R_RSP) {
					if (uuid.indexOf(mWaitCtrlUuid) < 0) {
						LOGW("Ctrl respond, but uuid: " + uuid + ", need: " + mWaitCtrlUuid);
						break;
					}
					uuid = mWaitCtrlUuid;
					if (cmd == LeCmd.CLOUD_MSG_CTRL_C2R_RSP && 
							(subcmd == LeCmd.Sub.CLOUD_MSG_CTRL_C2R_DO_OTA_RSP || 
							subcmd == LeCmd.Sub.CLOUD_MSG_CTRL_C2R_TELL_SHARE_RSP || 
							subcmd == LeCmd.Sub.CLOUD_MSG_CTRL_C2R_CONFIRM_SHARE_RSP)) {
						mWaitCtrlBackData = (buf.length == 0) ? "ok" : dataStr;
					} else {
						mWaitCtrlBackData = dataStr;
					}
					synchronized (mCtrlLock) {
						mCtrlLock.notifyAll();
					}
					synchronized (this) {
						if (mListener != null) {
							if ((cmd == LeCmd.CTRL_RSP && subcmd == LeCmd.Sub.CTRL_CMD_RSP)
									|| (cmd == LeCmd.CLOUD_MSG_CTRL_C2R_RSP && subcmd == LeCmd.Sub.CLOUD_MSG_CTRL_C2R_RSP)) {
								mListener.onCtrlBack(LeCmd.Sub.CTRL_DEV_CMD, uuid, dataStr);
							} else {
								mListener.onCtrlBack(subcmd, uuid, dataStr);
							}
						}
					}
				} else if (cmd == LeCmd.DISCOVER_RSP || cmd == LeCmd.CLOUD_REPORT_RSP) {
					LOGI("Get '" + uuid + "' Data:\n" + dataStr);
					dataJson = new JSONObject(dataStr);
					String keyuuid = uuid = dataJson.getString(LeCmd.K.UUID);
					JSONObject objSDev = dataJson.optJSONObject(LeCmd.K.SDEV);	
					if (null != objSDev) {
						keyuuid += dataJson.optString(LeCmd.K.ZMAC);
					}
					dataJson.put(LeCmd.K.MSGSTATUS, status);
					dataStr = dataJson.toString();
					if (mWaitGetUuid != null) {
						if (uuid.indexOf(mWaitGetUuid) >= 0) {
							mFindDevs.clear();
							mFindDevs.put(keyuuid, dataJson);
							LOGI("GetState '" + keyuuid + "' Data:\n" + dataStr);
							synchronized (mGetLock) {
								mGetLock.notifyAll();
							}
						} else { // getstate not
							LOGW("getState respond, but uuid: " + uuid + ", need: " + mWaitGetUuid);
						}
						synchronized (this) {
							if (mListener != null) {
								if ((cmd == LeCmd.DISCOVER_RSP && subcmd == LeCmd.Sub.DISCOVER_RSP)
										|| (cmd == LeCmd.CLOUD_REPORT_RSP && subcmd == LeCmd.Sub.CLOUD_REPORT_RSP)) {
									mListener.onGetStateBack(LeCmd.Sub.GET_STATE_CMD, uuid, dataStr);
								} else {
									mListener.onGetStateBack(subcmd, uuid, dataStr);
								}
							}
						}
					} else {
						LOGI("Discover '" + keyuuid + "', Data:\n" + dataStr);
						mFindDevs.put(keyuuid, dataJson);
						synchronized (this) {
							if (mListener != null) {
								mListener.onDiscoverBack(uuid, dataStr);
							}
						}
					}
				} else if (cmd == LeCmd.CLOUD_GET_TARGET_RSP && subcmd == LeCmd.Sub.CLOUD_GET_TARGET_RSP) {
					dataJson = new JSONObject(dataStr);
					if (dataJson.has(LeCmd.K.REMOTE_ADDR)) {
					} else {
						// waiting heart back;
					}
				} else if (cmd == LeCmd.CLOUD_AUTH_RSP && subcmd == LeCmd.Sub.CLOUD_AUTH_RSP) {
					// waiting heart back;
				} else if (cmd == LeCmd.CLOUD_HEARTBEAT_RSP && subcmd == LeCmd.Sub.CLOUD_HEARTBEAT_RSP) {
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
		 * SDK状态变化.<br>
		 * 
		 * @param s1
		 * 			之前状态 <br>
         * 			
		 * @param s2
		 * 			当前状态 <br>
		 * <br>
		 * 状态有如下几种: <br>
         *      LeCmd.State.NONE : 未启动 <br>
         *      LeCmd.State.STAR : 正在启动 <br>
         *      LeCmd.State.AP_CONNECTIN : 正在连接AP <br>
         *      LeCmd.State.AP_CONNECTED : 已经连上AP <br>
         *      LeCmd.State.CLOUD_LINKED : 开始联云 <br>
         *      LeCmd.State.CLOUD_AUTHED : 云认证通过 <br>
         *      LeCmd.State.CLOUD_ONLINE : 进入联网工作模式 <br>
         */
		void onLelinkStateChange(int s1, int s2);

		/**
		 * wifi配置时, 成功配置了设备.<br>
		 * 
		 * @param uuid
		 * 			device uuid<br>
		 */
		void onAirConfigBack(String uuid, String dataStr);

		/**
		 * 设备发现.<br>
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
		 * 设备状态获取回复.<br>
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
		 * 设备状态控制回复.<br>
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
		 * 设备状态变化通知.<br>
		 * 
		 * @param uuid
		 * 			device uuid<br>
		 * 
		 * @param dataStr
		 * 			设备的状态<br>
		 * 			
		 */
		void onStateChange(String uuid, String dataStr);
		
		/**
		 * 消息推送通知.<br>
		 * 
		 * @param dataStr
		 * 			消息内容<br>
		 * 			
		 */
		void onPushMessage(String dataStr);
		
//		/**
//		 * 接受到其它的控制.<br>
//		 * 
//		 * @param subcmd
//		 * 			子命令类型, 目前有:<br>
//		 * 			分享通知-> LeCmd.Sub.CLOUD_MSG_CTRL_C2R_TELL_SHARE_REQ<br>
//		 * 			确认分享-> LeCmd.Sub.CLOUD_MSG_CTRL_C2R_CONFIRM_SHARE_REQ<br>
//		 * 
//		 * @param uuid
//		 * 			对方sdk的uuid<br>
//		 * 
//		 * @param dataStr
//		 * 			控制内容(详见协议说明文档)<br>
//		 * 			
//		 */
//		void onControl(int subcmd, String uuid, String dataStr);
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

	/*********************************************** for native ********************************************************/
	private long mPtr = 0;

	private LeLink(String info, Listener listener) {
		mListener = listener;
		mPtr = init(info);
		isAuthed = (mPtr != 0);
		LOGE("init: " + (isAuthed ? "ok" : "error"));
	}

	private static final int MSG_TYPE_LOCALREQUEST = 1;
	private static final int MSG_TYPE_LOCALRESPOND = 2;
	private static final int MSG_TYPE_REMOTEREQUEST = 3;
	private static final int MSG_TYPE_REMOTERESPOND = 4;
	private static final int MSG_TYPE_LELINKSTATE = 5;

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
