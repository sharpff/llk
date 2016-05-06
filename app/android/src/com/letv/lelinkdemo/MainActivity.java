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

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		Log.i(TAG, LeLink.getSdkInfo());
		LeLink.setContext(getApplicationContext());
		mLeLink = LeLink.getInstance();
		mTestThread.start();
	}

	private Thread mTestThread = new Thread(new Runnable() {

		@Override
		public void run() {
			String retData;
			JSONObject dataJson;

			while (true) {
				Log.e(TAG, "Waitting auth...");
				try {
					Thread.sleep(500);
				} catch (InterruptedException e1) {
					e1.printStackTrace();
				}
				if (mLeLink.isCloud()) {
					break;
				}
			}
			
			/*
			 * 设备发现 必须传入timeout
			 * 
			 * 进入该函数，首先发送一次发现包。然后等待timeout时间，最后返回大这timeout期间收到的发现回复的设备。
			 */
			Log.w(TAG, "Get SDK uuid");
			sdkUUID = mLeLink.getSdkUUID();
			Log.i(TAG, "SDK UUID: " + sdkUUID);
			
			/*
			 * WIFI配置 必须传入参数: ssid, passwd, timeout
			 * 
			 * 在timeout时间内，不断重复发送配置包。如果期间收到hello,则退出该函数。
			 */
			Log.e(TAG, "Wifi config test...");
			try {
				mJsonCmd = new JSONObject();
				mJsonCmd.put(LeCmd.K.TIMEOUT, 60 * 5);
				mJsonCmd.put(LeCmd.K.SSID, "Letv_lelink");
				mJsonCmd.put(LeCmd.K.PASSWD, "987654321");
				mJsonCmd.put(LeCmd.K.TYPE, LeCmd.V.AIR_CONFIG_TYPE_MULTICAST);
			} catch (JSONException e) {
				e.printStackTrace();
			}
			if (mLeLink.airConfig(mJsonCmd.toString()) == 0) {
				Log.w(TAG, "airConfig ok!\n");
			} else {
				Log.e(TAG, "airConfig timeout");
				return;
			}

			/*
			 * 设备发现 必须传入timeout
			 * 
			 * 进入该函数，首先发送一次发现包。然后等待timeout时间，最后返回大这timeout期间收到的发现回复的设备。
			 */
			Log.e(TAG, "Device discover test...");
			retData = mLeLink.discover(20);
			if (retData != null) {
				Log.w(TAG, "find devices:\n" + retData);
			} else {
				Log.e(TAG, "Can't find device!");
				return;
			}

			/* 得到设备的uuid */
			String devUUID = "10000100101000010007C80E77ABCD52";
			try {
				JSONArray jsonArray = new JSONArray(retData);
				for (int i = 0; i < jsonArray.length(); i++) {
					dataJson = jsonArray.getJSONObject(i);
					if (dataJson.has(LeCmd.K.UUID) && dataJson.getString(LeCmd.K.UUID).equals(sdkUUID)) {
//						devUUID = dataJson.getString(LeCmd.K.UUID);
					}
				}
			} catch (JSONException e) {
				e.printStackTrace();
			}
			
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
				mJsonCmd.put(LeCmd.K.UUID, devUUID);
				mJsonCmd.put(LeCmd.K.TIMEOUT, 10);
//				mJsonCmd.put(LeCmd.K.ADDR, "192.168.1.102");
				mJsonData = new JSONObject();
				mJsonData.put(LeCmd.K.UUID, devUUID); 
			} catch (JSONException e) {
				e.printStackTrace();
			}
			retData = mLeLink.getState(mJsonCmd.toString(), mJsonData.toString());
			if (retData != null) {
				Log.w(TAG, "get state:\n" + retData);
			} else {
				Log.e(TAG, "Can't get state");
				return;
			}

			/* 得到设备的token */
			String devToken = "A9B864558E3CC920DEEDD13A6B1DE4FF";
			try {
				dataJson = new JSONObject(retData);
				if (dataJson.has(LeCmd.K.TOKEN)) {
					devToken = dataJson.getString(LeCmd.K.TOKEN);
				}
			} catch (JSONException e) {
				e.printStackTrace();
			}
			
			/*
			 * 控制设备状态 必须传入subcmd, uuid, timeout
			 * 
			 * 如果是传入addr代表通过局域网获得状态。反之，如果没有传入addr表示通过广域网获得状态。
			 * 如果该设备已经连接云，则必须传入token(由getState广域网获得)
			 */
			String dataStr = String.format("{\"ctrl\":{\"idx1\":%d,\"idx2\":%d,\"idx3\":%d,\"idx4\":%d}}", 0, 1, 0, 0);
			Log.e(TAG, "Control device test...\n" + dataStr);
			try {
				mJsonCmd = new JSONObject();
				mJsonCmd.put(LeCmd.K.SUBCMD, LeCmd.Sub.CTRL_DEV_CMD);
				mJsonCmd.put(LeCmd.K.UUID, devUUID);
				mJsonCmd.put(LeCmd.K.TOKEN, devToken);
				mJsonCmd.put(LeCmd.K.TIMEOUT, 5);
//				mJsonCmd.put(LeCmd.K.ADDR, "192.168.1.102");
			} catch (JSONException e) {
				e.printStackTrace();
			}
			retData = mLeLink.ctrl(mJsonCmd.toString(), dataStr);
			if (retData != null) {
				Log.w(TAG, "ctrl return:\n" + retData);
			} else {
				Log.e(TAG, "Can't ctrl");
				return;
			}
			
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
				mJsonCmd.put(LeCmd.K.UUID, devUUID);
				mJsonCmd.put(LeCmd.K.TIMEOUT, 10);
//				mJsonCmd.put(LeCmd.K.ADDR, "192.168.1.102");
				mJsonData = new JSONObject();
				mJsonData.put(LeCmd.K.UUID, devUUID); 
				mJsonData.put(LeCmd.K.VERSION, "1-0.1.0.svn.3338.0-1-1.0"); 
				mJsonData.put(LeCmd.K.TYPE, LeCmd.V.OTA_TYPE_IA_SCRIPT); 
				mJsonData.put(LeCmd.K.IAID, "2016042117413700000"); // type=LeCmd.V.OTA_TYPE_IA_SCRIPT的时候需要， 生成联动脚本后，得到的该值
			} catch (JSONException e) {
				e.printStackTrace();
			}
			retData = mLeLink.getState(mJsonCmd.toString(), mJsonData.toString());
			if (retData != null) {
				Log.w(TAG, "OTA state:\n" + retData);
			} else {
				Log.e(TAG, "Can't check OTA");
				return;
			}
			
			/*
			 * 触发OTA升级 必须传入subcmd, uuid, token, timeout
			 */
			dataStr = retData;
			Log.e(TAG, "Do OTA test...\n" + dataStr);
			try {
				mJsonCmd = new JSONObject();
				mJsonCmd.put(LeCmd.K.SUBCMD, LeCmd.Sub.CLOUD_MSG_CTRL_C2R_DO_OTA_REQ);
				mJsonCmd.put(LeCmd.K.UUID, devUUID);
				mJsonCmd.put(LeCmd.K.TOKEN, devToken);
				mJsonCmd.put(LeCmd.K.TIMEOUT, 5);
			} catch (JSONException e) {
				e.printStackTrace();
			}
			retData = mLeLink.ctrl(mJsonCmd.toString(), dataStr);
			Log.e(TAG, "Do OTA: " + retData);
		}
	});
}
