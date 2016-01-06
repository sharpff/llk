import com.letv.lelive.*;

// import java.util.*;
// import com.letv.lelive.*;
// import java.io.BufferedReader;
// import java.io.File;
// import java.io.FileInputStream;
// import java.io.FileReader;
// import java.io.IOException;
// import java.io.InputStream;
// import java.io.InputStreamReader;
// import java.io.RandomAccessFile;
// import java.io.Reader;
// import java.util.Scanner;


// import java.io.BufferedInputStream;
// import java.io.ByteArrayOutputStream;
// import java.io.File;
// import java.io.FileInputStream;
// import java.io.FileNotFoundException;
// import java.io.IOException;
// import java.io.RandomAccessFile;
// import java.nio.ByteBuffer;
// import java.nio.MappedByteBuffer;
// import java.nio.channels.FileChannel;
// import java.nio.channels.FileChannel.MapMode;

public class AndroidTest {  

    static {  
        System.loadLibrary("lelive");
    }  
  
    public static void main(String[] args) {  
        LeWrapper test = new LeWrapper();
        LeWrapper test1 = new LeWrapper();
        AndroidTestCallback cb = new AndroidTestCallback();
        if (0 == test.init(cb)) {
            LeAccount account = new LeAccount();
            account.ssid = "TP-LINK_JJFA1";
            account.passwd = "987654321";
            account.aes = "912EC803B2CE49E4A541068D495AB570";
            account.type = 1;
            account.delay = 10;
            // test.airconfigCtrl(account);

            // LeData data = new LeData();
            // data.id = 1;
            // data.bin = new byte[8];
            // // data.bin.length
            // test.postDiscovery(data);

            // LeJson json = new LeJson();
            // json.str = "{\"test\":1}";
            // test.postDiscovery(json);

            // LeData obj = test.polling();
            // if (obj instanceof LeData) {

            // }
            // System.out.printf(test.toString());


        }

        // AndroidTestCallback cb1 = new AndroidTestCallback();
        // if (0 == test1.init(cb)) {
        //     // System.out.printf(test1.toString());

        //     // LeAccount account = new LeAccount();
        //     // account.ssid = "ssid";
        //     // account.passwd = "passwd";
        //     // // test.airconfigCtrl("SSID=letv-office,PASSWD=987654321,AES=912EC803B2CE49E4A541068D495AB570,TYPE=1,DELAY=10");
        //     // test.airconfigCtrl(account);

        // }
        
        try{
            Thread.sleep(10000000);
        }catch(Exception e){
            System.out.printf("MAIN thread Exception\n");

        }finally{
            System.out.printf("MAIN thread finally\n");
        }
        test.exit();
        // test1.exit();

    }  
}  