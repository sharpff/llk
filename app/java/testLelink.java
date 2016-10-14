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

public class testLelink {
	private static LeLink mLeLink = null;

	public static void main(String argv[]) {

		byte[] bytes = new byte[10*2048];
		bytes[0] = (byte)0x01;
		bytes[1] = (byte)0x02;

		try {
			// new testLelink(bytes);
			System.out.printf("start testLelink\n");
			System.out.printf(LeLink.getSdkInfo());
			// mLeLink = LeLink.getInstance();
			// LeLink.setContext("abc", "11:22:33:44:55:66");
			// if (LeLink.setContext("abc", "11:22:33:44:55:66")) {
			// 	mLeLink = LeLink.getInstance();
			// } else {

			// }
			System.out.printf("end testLelink\n");
		}
		catch (Exception e) {
			System.out.printf("Exception for java\n");
		}
		catch (Error e) {
			// System.out.printf("Error for java\n");
			e.printStackTrace();
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

	
}
