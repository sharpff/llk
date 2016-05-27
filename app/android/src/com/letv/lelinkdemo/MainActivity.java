package com.letv.lelinkdemo;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;

import com.letv.lelink.LeCmd;
import com.letv.lelink.LeLink;

public class MainActivity extends Activity {
	private static final String TAG = "LeLinkDemo";
	private LeLink mLeLink = null;
	private String sdkUUID = null;
	private JSONObject mJsonCmd = null;
	private JSONObject mJsonData = null;
	
	private static boolean TEST_WIFI_CONFIG = false;
	private static boolean TEST_SDK_AUTH = true;
	private static boolean TEST_DISCOVER_DEV = false;
	private static boolean TEST_GET_STATE =  false;
	private static boolean TEST_CTRL_DEV = true;
	private static boolean TEST_OTA_CHECK = false;
	private static boolean TEST_OTA_DO = false;
	private static boolean TEST_AUTO_UUID = false; // depend on TEST_DISCOVER_DEV
//	private static String mTestDevUUID = "10000100101000010007C80E77ABCD50"; // 插排
	private static String mTestDevUUID = "10000100091000610006C80E77ABCD40"; // 窗帘
//	private static String mTestTevToken = "A9B864558E3CC920DEEDD13A6B1DE4FF"; // auto set by uuid, depend on TEST_GET_STATE
	private static String mTestTevToken = null; // auto set by uuid, depend on TEST_GET_STATE
//	private static String mTestCtrlCmd = String.format("{\"ctrl\":{\"idx1\":%d,\"idx2\":%d,\"idx3\":%d,\"idx4\":%d}}", 0, 0, 0, 0); // 插排
	private static String mTestCtrlCmd = String.format("{\"ctrl\":{\"action\":1}}"); // 窗帘
	private static int mWifiConfigTimeout = (60 * 5);
	private static int mDiscoverTimeout = 10;
	private static int mOtherTimeout = 10;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		Log.i(TAG, LeLink.getSdkInfo());
		LeLink.setContext(getApplicationContext(), mLeLinkListener, "11:22:33:44:55:66");
		Log.i(TAG, "SDKUUID: " + LeLink.getSdkUUID());
		mLeLink = LeLink.getInstance();
		mTestThread.start();
	}

	private Thread mTestThread = new Thread(new Runnable() {

		@Override
		public void run() {
			JSONObject dataJson;
			String retData = null;
			String dataStr = null;

			while (TEST_SDK_AUTH && true) {
				Log.e(TAG, "Waitting auth...");
				try {
					Thread.sleep(500);
				} catch (InterruptedException e) {
				}
				if (mLeLink.isCloud()) {
					break;
				}
			}
			
			Log.i(TAG, LeLink.getSdkInfo());
			
			/*
			 * 设备发现 必须传入timeout
			 * 
			 * 进入该函数，首先发送一次发现包。然后等待timeout时间，最后返回大这timeout期间收到的发现回复的设备。
			 */
			Log.w(TAG, "Get SDK uuid");
			sdkUUID = LeLink.getSdkUUID();
			Log.i(TAG, "SDK UUID: " + sdkUUID);
			
			if (TEST_WIFI_CONFIG) {
				/*
				 * WIFI配置 必须传入参数: ssid, passwd, timeout
				 * 
				 * 在timeout时间内，不断重复发送配置包。如果期间收到hello,则退出该函数。
				 */
				Log.e(TAG, "Wifi config test...");
				try {
					mJsonCmd = new JSONObject();
					mJsonCmd.put(LeCmd.K.TIMEOUT, mWifiConfigTimeout);
					mJsonCmd.put(LeCmd.K.SSID, "Letv_lelink");
					mJsonCmd.put(LeCmd.K.PASSWD, "987654321");
					mJsonCmd.put(LeCmd.K.TYPE, LeCmd.V.AIR_CONFIG_TYPE_MULTICAST);
				} catch (JSONException e) {
					e.printStackTrace();
					return;
				}
				if (mLeLink.airConfig(mJsonCmd.toString()) == 0) {
					Log.w(TAG, "airConfig ok!\n");
				} else {
					Log.e(TAG, "airConfig timeout");
					return;
				}
			}

			if (TEST_DISCOVER_DEV) {
				/*
				 * 设备发现 必须传入timeout
				 * 
				 * 进入该函数，首先发送一次发现包。然后等待timeout时间，最后返回大这timeout期间收到的发现回复的设备。
				 */
				Log.e(TAG, "Device discover test...");
				retData = mLeLink.discover(mDiscoverTimeout);
				if (retData != null) {
					Log.w(TAG, "find devices:\n" + retData);
				} else {
					Log.e(TAG, "Can't find device!");
					return;
				}
				if (TEST_AUTO_UUID) {
					/* 得到设备的uuid */
					try {
						JSONArray jsonArray = new JSONArray(retData);
						for (int i = 0; i < jsonArray.length(); i++) {
							dataJson = jsonArray.getJSONObject(i);
							if (dataJson.has(LeCmd.K.UUID) && dataJson.getString(LeCmd.K.UUID).equals(sdkUUID)) {
								mTestDevUUID = dataJson.getString(LeCmd.K.UUID);
							}
						}
					} catch (JSONException e) {
						e.printStackTrace();
						return;
					}
				}
			}
			
			if (TEST_GET_STATE) {
				/*
				 * 获得设备状态 必须传入subcmd, uuid, timeout
				 * 
				 * 如果是传入addr代表通过局域网获得状态。反之，如果没有传入addr表示通过广域网获得状态。
				 * 如果是该设备需要远程控制，则必须先通过该函数广域网获得到token.
				 */
				Log.e(TAG, "Get device state test...");
				try {
					mJsonCmd = new JSONObject();
					mJsonCmd.put(LeCmd.K.SUBCMD, LeCmd.Sub.GET_STATE_CMD);
					mJsonCmd.put(LeCmd.K.UUID, mTestDevUUID);
					mJsonCmd.put(LeCmd.K.TIMEOUT, mOtherTimeout);
					// mJsonCmd.put(LeCmd.K.ADDR, "192.168.1.102");
					mJsonData = new JSONObject();
					mJsonData.put(LeCmd.K.UUID, mTestDevUUID);
				} catch (JSONException e) {
					e.printStackTrace();
					return;
				}
				retData = mLeLink.getState(mJsonCmd.toString(), mJsonData.toString());
				if (retData != null) {
					Log.w(TAG, "get state:\n" + retData);
				} else {
					Log.e(TAG, "Can't get state");
					return;
				}
				/* 得到设备的token */
				try {
					dataJson = new JSONObject(retData);
					if (dataJson.has(LeCmd.K.TOKEN)) {
						mTestTevToken = dataJson.getString(LeCmd.K.TOKEN);
					}
				} catch (JSONException e) {
					e.printStackTrace();
					return;
				}
			}
			
			if (TEST_CTRL_DEV) {
				/*
				 * 控制设备状态 必须传入subcmd, uuid, timeout
				 * 
				 * 如果是传入addr代表通过局域网获得状态。反之，如果没有传入addr表示通过广域网获得状态。
				 * 如果该设备已经连接云，则必须传入token(由getState广域网获得)
				 */
				dataStr = mTestCtrlCmd;
				Log.e(TAG, "Control device test...\n" + dataStr);
				try {
					mJsonCmd = new JSONObject();
					mJsonCmd.put(LeCmd.K.SUBCMD, LeCmd.Sub.CTRL_DEV_CMD);
					mJsonCmd.put(LeCmd.K.UUID, mTestDevUUID);
					mJsonCmd.put(LeCmd.K.TOKEN, mTestTevToken);
					mJsonCmd.put(LeCmd.K.TIMEOUT, mOtherTimeout);
					mJsonCmd.put(LeCmd.K.ADDR, "192.168.3.238");
				} catch (JSONException e) {
					e.printStackTrace();
					return;
				}
				retData = mLeLink.ctrl(mJsonCmd.toString(), dataStr);
				if (retData != null) {
					Log.w(TAG, "ctrl return:\n" + retData);
				} else {
					Log.e(TAG, "Can't ctrl");
					return;
				}
			}
			
			if (TEST_OTA_CHECK) {
				/*
				 * 设备OTA查询 必须传入subcmd, uuid, timeout
				 * 
				 * 如果是传入addr代表通过局域网获得状态。反之，如果没有传入addr表示通过广域网获得状态。
				 * 如果是该设备需要远程控制，则必须先通过该函数广域网获得到token.
				 */
				Log.e(TAG, "OTA check test...");
				try {
					mJsonCmd = new JSONObject();
					mJsonCmd.put(LeCmd.K.SUBCMD, LeCmd.Sub.CLOUD_REPORT_OTA_QUERY_REQ);
					mJsonCmd.put(LeCmd.K.UUID, mTestDevUUID);
					mJsonCmd.put(LeCmd.K.TIMEOUT, mOtherTimeout);
					// mJsonCmd.put(LeCmd.K.ADDR, "192.168.1.102");
					mJsonData = new JSONObject();
					mJsonData.put(LeCmd.K.UUID, mTestDevUUID);
					mJsonData.put(LeCmd.K.VERSION, "1-0.1.0.svn.3338.0-1-1.0");
					mJsonData.put(LeCmd.K.TYPE, LeCmd.V.OTA_TYPE_IA_SCRIPT);
					mJsonData.put(LeCmd.K.IAID, "2016042117413700000"); // type=LeCmd.V.OTA_TYPE_IA_SCRIPT的时候需要，
																		// 生成联动脚本后，得到的该值
				} catch (JSONException e) {
					e.printStackTrace();
					return;
				}
				retData = mLeLink.getState(mJsonCmd.toString(), mJsonData.toString());
				if (retData != null) {
					Log.w(TAG, "OTA state:\n" + retData);
				} else {
					Log.e(TAG, "Can't check OTA");
					return;
				}
			}
			if (TEST_OTA_DO) {
				/*
				 * 触发OTA升级 必须传入subcmd, uuid, token, timeout
				 */
				dataStr = retData;
				Log.e(TAG, "Do OTA test...\n" + dataStr);
				try {
					mJsonCmd = new JSONObject();
					mJsonCmd.put(LeCmd.K.SUBCMD, LeCmd.Sub.CLOUD_MSG_CTRL_C2R_DO_OTA_REQ);
					mJsonCmd.put(LeCmd.K.UUID, mTestDevUUID);
					mJsonCmd.put(LeCmd.K.TOKEN, mTestTevToken);
					mJsonCmd.put(LeCmd.K.TIMEOUT, mOtherTimeout);
				} catch (JSONException e) {
					e.printStackTrace();
					return;
				}
				retData = mLeLink.ctrl(mJsonCmd.toString(), dataStr);
				Log.e(TAG, "Do OTA: " + retData);
			}
		}
	});
	
	private LeLink.Listener mLeLinkListener = new LeLink.Listener() {
		@Override
		public void onStateChange(String uuid, String dataStr) {
			String str = String.format("onStateChange(%s):\n%s", uuid, dataStr);
			Log.e(TAG, str);
		}

		@Override
		public void onCloudStateChange(boolean isCloud) {
			String str = String.format("onCloudStateChange: %s", isCloud);
			Log.e(TAG, str);
		}

		@Override
		public void onAirConfigBack(String uuid) {
			String str = String.format("onAirConfigBack: %s", uuid);
			Log.e(TAG, str);
		}

		@Override
		public void onDiscoverBack(String uuid, String dataStr) {
			String str = String.format("onDiscoverBack(%s):\n%s", uuid, dataStr);
			Log.e(TAG, str);
		}

		@Override
		public void onGetStateBack(int subcmd, String uuid, String dataStr) {
			String str = String.format("onGetStateBack-%d(%s):\n%s", subcmd, uuid, dataStr);
			Log.e(TAG, str);
		}

		@Override
		public void onCtrlBack(int subcmd, String uuid, String dataStr) {
			String str = String.format("onCtrlBack-%d(%s):\n%s", subcmd, uuid, dataStr);
			Log.e(TAG, str);
		}

		@Override
		public void onPushMessage(String dataStr) {
			String str = String.format("onPushMessage:\n%s", dataStr);
			Log.e(TAG, str);
		}

		@Override
		public void onControl(int subcmd, String uuid, String dataStr) {
			String str = String.format("onControl:\n%s", dataStr);
			Log.e(TAG, str);
		}
	};
}
