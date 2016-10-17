import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.RandomAccessFile;
import java.io.Reader;
import java.io.BufferedInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.RandomAccessFile;

import com.letv.lelink.LeCmd;
import com.letv.lelink.LeLink;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.nio.file.Path;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

// import sun.misc.BASE64Decoder;
// import sun.misc.BASE64Encoder;

import java.io.FileOutputStream; 

public class testLelink {
	private static final String TAG = "LeLinkDemo";
	private static LeLink mLeLink = null;
	private String sdkUUID = null;
	private JSONObject mJsonCmd = null;
	private JSONObject mJsonData = null;
	
	private static boolean TEST_WIFI_CONFIG = false;
	private static boolean TEST_SDK_AUTH = true;
	private static boolean TEST_DISCOVER_DEV = true;
	private static boolean TEST_GET_STATE =  true;
	private static boolean TEST_CTRL_DEV = true;
	private static boolean TEST_OTA_CHECK = false;
	private static boolean TEST_OTA_DO = false;
	private static boolean TEST_AUTO_UUID = false; // depend on TEST_DISCOVER_DEV
	private static String mTestDevUUID = "10000100101000010007C80E77ABCD5A"; // 插排
//	private static String mTestDevUUID = "10000100091000610006C80E77ABCD40"; // 窗帘
//	private static String mTestTevToken = "A9B864558E3CC920DEEDD13A6B1DE4FF"; // auto set by uuid, depend on TEST_GET_STATE
	private static String mTestTevToken = null; // auto set by uuid, depend on TEST_GET_STATE
	private static String mTestCtrlCmd = String.format("{\"ctrl\":{\"idx1\":%d,\"idx2\":%d,\"idx3\":%d,\"idx4\":%d}}", 0, 0, 1, 0); // 插排
//	private static String mTestCtrlCmd = String.format("{\"ctrl\":{\"action\":1}}"); // 窗帘
	private static int mWifiConfigTimeout = (60 * 5);
	private static int mDiscoverTimeout = 10;
	private static int mOtherTimeout = 10;
	public static void main(String argv[]) {

		byte[] bytes = new byte[10*2048];
		bytes[0] = (byte)0x01;
		bytes[1] = (byte)0x02;
		String authStr;
		try {
			// new testLelink(bytes);
			System.out.printf("start testLelink\n");
			try {
				int count = 0;
				int read = 0;
				byte buffer[] = new byte[4*1024];
				// Path path = Paths.get("./0x1c2000.bin");
				// buffer = Files.readAllBytes(path);

				FileInputStream in = new FileInputStream("./0x1c2000.bin");
				while((count = in.read(buffer)) != -1){ 
					read += count;
				}
				in.close();

				// FileOutputStream out = new FileOutputStream("./0x1c2000.bin.out");
				// out.write(buffer);
				// out.close();
				// System.out.printf("read bytes [%d]\n", buffer.length);

				// File file = new File("./0x1c2000.bin");;
				// FileInputStream inputFile = new FileInputStream(file);
				// byte[] buffer1 = new byte[(int) file.length()];
				// inputFile.read(buffer1);
				// inputFile.close();
				authStr = new BASE64Encoder().encode(buffer);
			} catch (IOException e) {
				e.printStackTrace();
				return;
			}
			if (LeLink.setContext(authStr, "11:22:33:44:55:66")) {
				System.out.printf(LeLink.getSdkInfo() + "\n");
				// System.out.printf("111111" + LeLink.getSdkUUID());
				mLeLink = LeLink.getInstance();
				// mTestThread.start();
			} else {
				System.out.printf("222222");
			}
			System.out.printf("end testLelink\n");
		}
		catch (Exception e) {
			System.out.printf("Exception for java");
		}
		catch (Error e) {
			System.out.printf("Error for java");
			e.printStackTrace();
		}
		try {
	        System.out.println("请输入：");  
	        int i = 0;  
	        while(i != -1){//读取输入流中的字节直到流的末尾返回1  
	            i = System.in.read();  
	            System.out.println(i);  
	        }  
		}
		catch (IOException e) {

		}
	}

	// public testLelink(byte[] bytes) {
	// 	System.out.printf("start testLelink\n");
	// 	System.out.printf(LeLink.getSdkInfo());
	// 	if (LeLink.setContext("abc", "11:22:33:44:55:66")) {
	// 		mLeLink = LeLink.getInstance();
	// 	} else {
	// 	}

	// 	// mLeLink = LeLink.getInstance();
	// 	System.out.printf("end testLelink\n");
	// }

	// public static byte[] toByteArray(String filename, int[] pTotal){
		
	// 	File f = new File(filename);
	// 	/*
	// 	if(!f.exists()){
	// 		throw new FileNotFoundException(filename);
	// 	}
	// 	*/
	// 	ByteArrayOutputStream bos = new ByteArrayOutputStream((int)f.length());
	// 	BufferedInputStream in = null;
	// 	try{
	// 		in = new BufferedInputStream(new FileInputStream(f));
	// 		int buf_size = 10*1024;
	// 		byte[] buffer = new byte[buf_size];
	// 		int len = 0;
	// 		int total = 0;
	// 		while(-1 != (len = in.read(buffer,0,buf_size))){
	// 			bos.write(buffer,0,len);
	// 			total += len;
	// 		}
	// 		pTotal[0] = total;
	// 		return bos.toByteArray();
	// 	}catch (IOException e) {
	// 		e.printStackTrace();
	// 		//throw e;
	// 	}finally{
	// 		try{
	// 			in.close();
	// 		}catch (IOException e) {
	// 			e.printStackTrace();
	// 		}
	// 		//bos.close();
	// 		return bos.toByteArray();
	// 	}
	// }
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
					mJsonCmd.put(LeCmd.K.SSID, "ff");
					mJsonCmd.put(LeCmd.K.PASSWD, "fengfeng2qiqi");
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
//					mJsonCmd.put(LeCmd.K.ADDR, "192.168.3.238");
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
	
}
