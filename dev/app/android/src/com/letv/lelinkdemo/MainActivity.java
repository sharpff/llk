package com.letv.lelinkdemo;

import org.json.JSONException;
import org.json.JSONObject;

import com.letv.lelinkdemo.R;
import com.letv.lelink.LeCmd;
import com.letv.lelink.LeLink;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;

public class MainActivity extends Activity {
	private static final String TAG = "LeLinkDemo";
	private LeLink mLeLink = null;
	private JSONObject mJsonCmd = null;
	private static final String PUBLIC_KEY = "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDDxqkuVDvetJDbq1JgBr93VBYKJZdnZbMFS7gWoNjX1BUTxjBRY+2ek2k/V08lTj9ejlCsnn927lE4iOBzrHY66xIktxsyw6btmGcqQ/doDX6y89mHcbq8e2oFE+eJ/vuqI426g+waaYxPrCtsuN+G+4PUEnZnOZP/PFAGMj3pywIDAQAB";
	private static final String SIGNALTURE = "vUckaTiTQsBuvtKHS1UgNMarBopvFTZolNGXbnkwqD2GX9d7c6dSQOHOr8niTFYPWkhl2/3DQWouRWfMXt55QYKvSpToiQA7EZqcnNR7/T1hfa0RKU8lzuRV3RWD7CVApmTJ5ZIHQMd8GL1kDjj8zPNdgAZKr9Dpnpyz0yfBzaM=";
	private static final String UUID = "f1b312fd6f7b427f8306";
//	private static final String PUBLIC_KEY = "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCf2eu1g68UFbocZLROH90/3rGWpnJkOkRWSO4C3QUjMJ00b4nJDqbTwwkr9w1sLIJd5VsQ0UHwl80+62E6PcUV1ST9KgLfPyvqNbhN3NmpqPOS5wCZGsFp8zGkS9NYdtc3KmClF2K5OlSTaxg7EgdYwytRa1IxZdRNc1MJiKgEtwIDAQAB";
//	private static final String SIGNALTURE = "Zuv23oJM/86hFqwwXv2uMkc60p5OwWyxYMTOHyIu+ShG3IkmWKXNCNNFYG9Vkc3ApgWul0WpFQaYT99lYhbUK8GS6h1jyqiPbj7eCHhZcbVNsaQA+Umw/S3+Fj7HhBBJ/2MwwIi4GgfT65BYW96yT9p0WDhmWEyOnmGtb3rEHBo=";
//	private static final String UUID = "6914eae8a5714537a395";

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		/*
		 * SDK版本信息查看
		 */
		Log.i(TAG, LeLink.getSdkInfo());

		JSONObject json = new JSONObject();
		try {
			json.put(LeCmd.K.PUBLIC_KEY, PUBLIC_KEY);
			json.put(LeCmd.K.SIGNATURE, SIGNALTURE);
			json.put(LeCmd.K.UUID, UUID);
		} catch (JSONException e) {
			e.printStackTrace();
		}
		LeLink.setInfo(json.toString());
		
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
				mJsonCmd.put(LeCmd.K.UUID, "10000100101000710007123456abcdef");
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
			String ctrlStr = String.format("{\"ctrl\":{\"idx1\":%d,\"idx2\":%d,\"idx3\":%d,\"idx4\":%d}}", 0, 0, 0, 0);
			Log.e(TAG, "Control device test...\n" + ctrlStr);
			try {
				mJsonCmd = new JSONObject();
				mJsonCmd.put(LeCmd.K.TIMEOUT, 5);
//				mJsonCmd.put(LeCmd.K.ADDR, "192.168.1.102");
				mJsonCmd.put(LeCmd.K.UUID, "10000100101000710007123456abcdef");
				mJsonCmd.put(LeCmd.K.TOKEN, "dfad9fe665e85f67ba1d79ebf1be9c8c");
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

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// Handle action bar item clicks here. The action bar will
		// automatically handle clicks on the Home/Up button, so long
		// as you specify a parent activity in AndroidManifest.xml.
		int id = item.getItemId();
		if (id == R.id.action_settings) {
			return true;
		}
		return super.onOptionsItemSelected(item);
	}
}
