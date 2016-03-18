package com.letv.lelinkdemo;

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
	private JSONObject mJsonCmd = null;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		Log.i(TAG, LeLink.getSdkInfo());
		LeLink.setContent(getApplicationContext());
		mLeLink = LeLink.getInstance();
		mTestThread.start();
	}

	private Thread mTestThread = new Thread(new Runnable() {

		@Override
		public void run() {
			String devs;

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
			 * WIFI配置 必须传入参数: ssid, passwd, timeout
			 * 
			 * 在timeout时间内，不断重复发送配置包。如果期间收到hello,则退出该函数。
			 */
//			Log.e(TAG, "Wifi config test...");
//			try {
//				mJsonCmd = new JSONObject();
//				mJsonCmd.put(LeCmd.K.TIMEOUT, 60);
//				mJsonCmd.put(LeCmd.K.SSID, "Xiaomi_A7DD");
//				mJsonCmd.put(LeCmd.K.PASSWD, "987654321");
//			} catch (JSONException e) {
//				e.printStackTrace();
//			}
//			if (mLeLink.airConfig(mJsonCmd.toString()) == 0) {
//				Log.w(TAG, "airConfig ok!\n");
//			} else {
//				Log.e(TAG, "airConfig timeout");
//			}

			/*
			 * 设备发现 必须传入timeout
			 * 
			 * 进入该函数，首先发送一次发现包。然后等待timeout时间，最后返回大这timeout期间收到的发现回复的设备。
			 */
			Log.e(TAG, "Device discover test...");
			devs = mLeLink.discover(5);
			if (devs != null) {
				Log.w(TAG, "find devices:\n" + devs);
			} else {
				Log.e(TAG, "Can't find device!");
			}

			/*
			 * 获得设备状态 必须传入uuid, timeout
			 * 
			 * 如果是传入addr代表通过局域网获得状态。反之，如果没有传入addr表示通过广域网获得状态。
			 * 如果是该设备需要远程控制，则必须先通过该函数广域网获得到token.
			 */
			Log.e(TAG, "Get device state test...");
			try {
				mJsonCmd = new JSONObject();
				mJsonCmd.put(LeCmd.K.TIMEOUT, 5);
//				mJsonCmd.put(LeCmd.K.ADDR, "192.168.1.102");
				mJsonCmd.put(LeCmd.K.UUID, "10000100101000010007F0B429000012");
			} catch (JSONException e) {
				e.printStackTrace();
			}
			devs = mLeLink.getState(mJsonCmd.toString());
			if (devs != null) {
				Log.w(TAG, "get state:\n" + devs);
			} else {
				Log.e(TAG, "Can't get state");
			}

			/*
			 * 控制设备状态 必须传入uuid, timeout, token
			 * 
			 * 如果是传入addr代表通过局域网获得状态。反之，如果没有传入addr表示通过广域网获得状态。
			 * 如果该设备已经连接云，则必须传入token(由getState广域网获得)
			 */
			String ctrlStr = String.format("{\"ctrl\":{\"idx1\":%d,\"idx2\":%d,\"idx3\":%d,\"idx4\":%d}}", 0, 1, 1, 0);
			Log.e(TAG, "Control device test...\n" + ctrlStr);
			try {
				mJsonCmd = new JSONObject();
				mJsonCmd.put(LeCmd.K.TIMEOUT, 5);
//				mJsonCmd.put(LeCmd.K.ADDR, "192.168.1.102");
				mJsonCmd.put(LeCmd.K.UUID, "10000100101000010007F0B429000012");
				mJsonCmd.put(LeCmd.K.TOKEN, "FF8199AC80F1566539E37AD9236C9EE2");
			} catch (JSONException e) {
				e.printStackTrace();
			}
			devs = mLeLink.ctrl(mJsonCmd.toString(), ctrlStr);
			if (devs != null) {
				Log.w(TAG, "ctrl return:\n" + devs);
			} else {
				Log.e(TAG, "Can't ctrl");
			}
		}
	});
}
