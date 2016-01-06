import com.letv.lelive.*;


public class AndroidTestCallback extends LeDelegate {  

    public int notify(Object obj, LeData data) {
        System.out.printf("caller: " + obj.toString() + " callback: " + toString() + "\r\n");
    	if (data instanceof LeBin) {
    		LeBin objLeBin = (LeBin)data;
    		// String str = new String(objLeBin.bin);
        	System.out.printf("AndroidTestCallback:notify LeBin id[%d] bin[%d][%s]\r\n", 
        		objLeBin.id, objLeBin.bin.length, new String(objLeBin.bin));
    	} else if (data instanceof LeJson) {
    		LeJson objLeJson = (LeJson)data;
        	System.out.printf("AndroidTestCallback:notify LeJson str[%s] \r\n", 
        		objLeJson.str);
    	} else if (data instanceof LeStatus) {
            LeStatus status = (LeStatus)data;
            System.out.printf("AndroidTestCallback:notify LeStatus status[%d] \r\n", 
                status.what);
        }

        return 22;
    }
}