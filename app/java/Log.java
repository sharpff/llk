public class Log {  
       
    private static char[] codec_table = {};
  
    public Log() {  
  
    } 

    public static void i(String tag, String log) {
        System.out.printf("[%s] %s", tag, log);
    }
    public static void w(String tag, String log) {
        System.out.printf("[%s] %s", tag, log);
    }
    public static void e(String tag, String log) {
        System.out.printf("[%s] %s", tag, log);
    }
}  