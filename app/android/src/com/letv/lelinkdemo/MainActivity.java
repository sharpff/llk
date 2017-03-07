package com.letv.lelinkdemo;

import java.io.IOException;
import java.io.InputStream;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import android.app.Activity;
import android.os.Bundle;
import android.os.Handler;
import android.util.Base64;
import android.util.Log;
import android.widget.TextView;

import com.letv.lelink.LeCmd;
import com.letv.lelink.LeLink;

public class MainActivity extends Activity {
	private static final String TAG = "LeLinkDemo";
	private String sdkUUID = null;
	private JSONObject mJsonCmd = null;
	private JSONObject mJsonData = null;
	
	private static boolean TEST_WIFI_CONFIG = true;
	private static boolean TEST_SDK_AUTH = true;
	private static boolean TEST_DISCOVER_DEV = true;
	private static boolean TEST_GET_STATE =  true;
	private static boolean TEST_CTRL_DEV = true;
	private static boolean TEST_OTA_CHECK = false;
	private static boolean TEST_OTA_DO = false;
	private static boolean TEST_AUTO_UUID = false; // depend on TEST_DISCOVER_DEV
//	private static String mTestDevUUID = "10000100101000010007C80E77ABCD5A"; // 插排
//	private static String mTestDevUUID = "10000100091000610006C80E77ABCD40"; // 窗帘
	private static String mTestDevUUID = "10000100141001010012C89346E0046E"; // 晾霸
//	private static String mTestTevToken = "A9B864558E3CC920DEEDD13A6B1DE4FF"; // auto set by uuid, depend on TEST_GET_STATE
	private static String mTestTevToken = null; // auto set by uuid, depend on TEST_GET_STATE
//	private static String mTestCtrlCmd = String.format("{\"ctrl\":{\"idx1\":%d,\"idx2\":%d,\"idx3\":%d,\"idx4\":%d}}", 0, 0, 1, 0); // 插排
//	private static String mTestCtrlCmd = String.format("{\"ctrl\":{\"action\":1}}"); // 窗帘
	private static String mTestCtrlCmd = String.format("{\"ctrl\":{\"light\":1}}"); // 晾霸
	private static int mWifiConfigTimeout = (60 * 2);
	private static int mDiscoverTimeout = 10;
	private static int mOtherTimeout = 10;
	
	private String mMacStr = null;
	private String mAuthStr = null;
	private TextView mTextView = null;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		int rd = -1;
		InputStream in = null;
		byte buffer[] = new byte[1024 * 10];
		
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		mTextView = (TextView) findViewById(R.id.hello);
		
		// sdk info
		Log.i(TAG, LeLink.getSdkInfo());
		mTextView.append("\n\nInfo:\n " + LeLink.getSdkInfo());
		// Authentication
		try {
			in = getApplicationContext().getAssets().open("lelink/auth.cfg");
		} catch (IOException e) {
			e.printStackTrace();
			mTextView.append("\n" + e.toString());
			return;
		}
		if(in == null) {
			Log.e(TAG, "Can't Open file: " + "lelink/auth.cfg");
			mTextView.append("\nCan't Open file: " + "lelink/auth.cfg");
			return;
		}
		try {
			rd = in.read(buffer);
		} catch (IOException e) {
			e.printStackTrace();
			e.printStackTrace();
			mTextView.append("\n" + e.toString());
			return;
		}
		mAuthStr = Base64.encodeToString(buffer, 0, rd, Base64.NO_WRAP);
		if(mAuthStr == null) {
			Log.e(TAG, "Auth error");
			mTextView.append("\nAuth error");
			return;
		}
		mMacStr = "11:22:33:44:55:66";
//		LeLink.getInstance(mAuthStr, mMacStr, mLeLinkListener);
//		mTextView.append("\n\nInfo:\n " + LeLink.getSdkInfo());
		mTestThread.start();
	}

	private Thread mTestThread = new Thread(new Runnable() {

		@Override
		public void run() {
			JSONObject dataJson;
			String retData = null;
			String dataStr = null;

			/*
			 * 设备发现 必须传入timeout
			 * 
			 * 进入该函数，首先发送一次发现包。然后等待timeout时间，最后返回大这timeout期间收到的发现回复的设备。
			 */
			Log.w(TAG, "Get SDK uuid ...");
			mHandler.sendMessage(mHandler.obtainMessage(0, "Get SDK uuid ..."));
			sdkUUID = LeLink.getInstance(mAuthStr, mMacStr).getSdkUUID();
			Log.i(TAG, "SDK UUID: " + sdkUUID);
			mHandler.sendMessage(mHandler.obtainMessage(0, "SDK UUID: " + sdkUUID));
			
			if (TEST_WIFI_CONFIG) {
				/*
				 * WIFI配置 必须传入参数: ssid, passwd, timeout
				 * 
				 * 在timeout时间内，不断重复发送配置包。如果期间收到hello,则退出该函数。
				 */
				Log.e(TAG, "Wifi config test...");
				mHandler.sendMessage(mHandler.obtainMessage(0, "Wifi config test... " + System.currentTimeMillis()));
				try {
					mJsonCmd = new JSONObject();
					mJsonCmd.put(LeCmd.K.TIMEOUT, mWifiConfigTimeout);
//					mJsonCmd.put(LeCmd.K.SSID, "Xiaomi_Lelink");
//					mJsonCmd.put(LeCmd.K.PASSWD, "12345678");
					mJsonCmd.put(LeCmd.K.SSID, "LLGD");
					mJsonCmd.put(LeCmd.K.PASSWD, "12345678");
					// mJsonCmd.put(LeCmd.K.APSSID, "tplink");
					mJsonCmd.put(LeCmd.K.TYPE, LeCmd.V.AIR_CONFIG_TYPE_AIRHUG);
//					mJsonCmd.put(LeCmd.K.TYPE, LeCmd.V.AIR_CONFIG_TYPE_MULTICAST);
					// mJsonCmd.put(LeCmd.K.AESKEY, "4d90c52bea5259b95b53d33c63a706e2");
					// AESKEY is optional
//					mJsonCmd.put(LeCmd.K.AESKEY, "157e835e6c0bc55474abcd91e00e6979");
				} catch (JSONException e) {
					e.printStackTrace();
					return;
				}
				if (LeLink.getInstance(mAuthStr, mMacStr, mLeLinkListener).airConfig(mJsonCmd.toString()) == 0) {
					Log.w(TAG, "airConfig ok!");
					mHandler.sendMessage(mHandler.obtainMessage(0, "airConfig ok! " + System.currentTimeMillis()));
				} else {
					Log.e(TAG, "airConfig timeout");
					mHandler.sendMessage(mHandler.obtainMessage(0, "airConfig timeout"));
					return;
				}
			}
			
			Log.e(TAG, "Waitting auth...");
			while (TEST_SDK_AUTH && (LeLink.getInstance(mAuthStr, mMacStr).getState() < LeCmd.State.CLOUD_ONLINE)) {
				mHandler.sendMessage(mHandler.obtainMessage(0, "Waitting auth... " + System.currentTimeMillis()));
				try {
					Thread.sleep(500);
				} catch (InterruptedException e) {
				}
			}
			Log.i(TAG, LeLink.getSdkInfo());
			mHandler.sendMessage(mHandler.obtainMessage(0, LeLink.getSdkInfo()));

			if (TEST_DISCOVER_DEV) {
				/*
				 * 设备发现 必须传入timeout
				 * 
				 * 进入该函数，首先发送一次发现包。然后等待timeout时间，最后返回大这timeout期间收到的发现回复的设备。
				 */
				Log.e(TAG, "Device discover test...");
				mHandler.sendMessage(mHandler.obtainMessage(0, "Device discover test..."));
				retData = LeLink.getInstance(mAuthStr, mMacStr, mLeLinkListener).discover(mDiscoverTimeout);
				if (retData != null) {
					Log.w(TAG, "Find devices:\n" + retData);
					mHandler.sendMessage(mHandler.obtainMessage(0, "Find devices:\n" + retData));
				} else {
					Log.e(TAG, "Can't find device!");
					mHandler.sendMessage(mHandler.obtainMessage(0, "Can't find device!"));
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
				mHandler.sendMessage(mHandler.obtainMessage(0, "Get device state test... " + System.currentTimeMillis()));
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
				retData = LeLink.getInstance(mAuthStr, mMacStr, mLeLinkListener).getState(mJsonCmd.toString(), mJsonData.toString());
				if (retData != null) {
					Log.w(TAG, "Get state:\n" + retData);
					mHandler.sendMessage(mHandler.obtainMessage(0, "Get state " + System.currentTimeMillis() + "\n" + retData));
				} else {
					Log.e(TAG, "Can't get state");
					mHandler.sendMessage(mHandler.obtainMessage(0, "Can't get state"));
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
				Log.e(TAG, "Control device test..." + dataStr);
				mHandler.sendMessage(mHandler.obtainMessage(0, "Control device test... " + System.currentTimeMillis()));
				try {
					mJsonCmd = new JSONObject();
					mJsonCmd.put(LeCmd.K.SUBCMD, LeCmd.Sub.CTRL_DEV_CMD);
					mJsonCmd.put(LeCmd.K.UUID, mTestDevUUID);
					mJsonCmd.put(LeCmd.K.TOKEN, mTestTevToken);
					mJsonCmd.put(LeCmd.K.TIMEOUT, mOtherTimeout);
//					mJsonCmd.put(LeCmd.K.ADDR, "192.168.3.238");
				} catch (JSONException e) {
					e.printStackTrace();
					return;
				}
				mHandler.sendMessage(mHandler.obtainMessage(0, mJsonCmd.toString()));
				retData = LeLink.getInstance(mAuthStr, mMacStr, mLeLinkListener).ctrl(mJsonCmd.toString(), dataStr);
				if (retData != null) {
					Log.w(TAG, "ctrl return:\n" + retData);
					mHandler.sendMessage(mHandler.obtainMessage(0, "ctrl return: " + System.currentTimeMillis() + "\n" + retData));
				} else {
					Log.e(TAG, "Can't ctrl");
					mHandler.sendMessage(mHandler.obtainMessage(0, "Can't ctrl"));
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
				mHandler.sendMessage(mHandler.obtainMessage(0, "OTA check test..."));
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
				retData = LeLink.getInstance(mAuthStr, mMacStr, mLeLinkListener).getState(mJsonCmd.toString(), mJsonData.toString());
				if (retData != null) {
					Log.w(TAG, "OTA state:\n" + retData);
					mHandler.sendMessage(mHandler.obtainMessage(0, "OTA state:\n" + retData));
				} else {
					Log.e(TAG, "Can't check OTA");
					mHandler.sendMessage(mHandler.obtainMessage(0, "Can't check OTA"));
					return;
				}
			}
			if (TEST_OTA_DO) {
				/*
				 * 触发OTA升级 必须传入subcmd, uuid, token, timeout
				 */
				dataStr = retData;
				Log.e(TAG, "Do OTA test..." + dataStr);
				mHandler.sendMessage(mHandler.obtainMessage(0, "Do OTA test..."));
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
				retData = LeLink.getInstance(mAuthStr, mMacStr, mLeLinkListener).ctrl(mJsonCmd.toString(), dataStr);
				Log.e(TAG, "Do OTA: " + retData);
				mHandler.sendMessage(mHandler.obtainMessage(0, "Do OTA: " + retData));
			}
		}
	});
	
	private LeLink.Listener mLeLinkListener = new LeLink.Listener() {
		@Override
		public void onStateChange(String uuid, String dataStr) {
			String str = String.format("onStateChange(%s):\n%s", uuid, dataStr);
			Log.e(TAG, str);
			mHandler.sendMessage(mHandler.obtainMessage(0, str));
		}

		@Override
		public void onAirConfigBack(String uuid, String dataStr) {
			String str = String.format("onAirConfigBack: %s, %s", uuid, dataStr);
			Log.e(TAG, str);
			mHandler.sendMessage(mHandler.obtainMessage(0, str));
		}

		@Override
		public void onDiscoverBack(String uuid, String dataStr) {
			String str = String.format("onDiscoverBack(%s):\n%s", uuid, dataStr);
			Log.e(TAG, str);
			mHandler.sendMessage(mHandler.obtainMessage(0, str));
		}

		@Override
		public void onGetStateBack(int subcmd, String uuid, String dataStr) {
			String str = String.format("onGetStateBack-%d(%s):\n%s", subcmd, uuid, dataStr);
			Log.e(TAG, str);
			mHandler.sendMessage(mHandler.obtainMessage(0, str));
		}

		@Override
		public void onCtrlBack(int subcmd, String uuid, String dataStr) {
			String str = String.format("onCtrlBack-%d(%s):\n%s", subcmd, uuid, dataStr);
			Log.e(TAG, str);
			mHandler.sendMessage(mHandler.obtainMessage(0, str));
		}

		@Override
		public void onPushMessage(String dataStr) {
			String str = String.format("onPushMessage:\n%s", dataStr);
			Log.e(TAG, str);
			mHandler.sendMessage(mHandler.obtainMessage(0, str));
		}

		@Override
		public void onLelinkStateChange(int s1, int s2) {
			String str = String.format("onStateChange:  %d -> %d", s1, s2);
			Log.e(TAG, str);
			mHandler.sendMessage(mHandler.obtainMessage(0, str));
		}
	};
	private Handler mHandler = new Handler() {
		public void handleMessage(android.os.Message msg) {
			mTextView.append("\n\n");
			mTextView.append((CharSequence) msg.obj);
		};
	};
}
