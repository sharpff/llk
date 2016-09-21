package com.letv.lelink;

/** 
 * 
 * Lelink Android平台接入SDK接口参数.<br>
 * Copyright © 2004-2016 乐视网（letv.com）All rights reserved.<br><br>
 * 
 * 接口函数中使用到的参数.<br>
 * 
 * @author  feiguoyou@letv.com
 * 
 */
public class LeCmd {
	public static final int HELLO_REQ = 7;
	public static final int HELLO_RSP = 8;
	public static final int DISCOVER_REQ = 9;
	public static final int DISCOVER_RSP = 10;
	public static final int CTRL_REQ = 15;
	public static final int CTRL_RSP = 16;
	public static final int CLOUD_GET_TARGET_REQ = 87;
	public static final int CLOUD_GET_TARGET_RSP = 88;
	public static final int CLOUD_AUTH_REQ = 89;
    public static final int CLOUD_AUTH_RSP = 90;
	public static final int CLOUD_HEARTBEAT_REQ = 91;
	public static final int CLOUD_HEARTBEAT_RSP = 92;
	public static final int CLOUD_REPORT_REQ = 93;
	public static final int CLOUD_REPORT_RSP = 94;
	public static final int CLOUD_MSG_CTRL_C2R_REQ = 95;
	public static final int CLOUD_MSG_CTRL_C2R_RSP = 96;
	public static final int CLOUD_MSG_CTRL_R2T_REQ = 97;
	public static final int CLOUD_MSG_CTRL_R2T_RSP = 98;
	public static final int CLOUD_IND_REQ = 99;
	public static final int CLOUD_IND_RSP = 100;

	/** 子命令集 */
	public class Sub {
		/* HELLO */
		public static final int HELLO_REQ = 1;
		public static final int HELLO_RSP = 2;
		/* DISCOVER */
		public static final int DISCOVER_REQ = 1;
		public static final int DISCOVER_RSP = 2;
		public static final int DISCOVER_STATUS_CHANGED_REQ = 3;
		public static final int DISCOVER_STATUS_CHANGED_RSP = 4;
		/* CTRL */
		public static final int CTRL_CMD_REQ = 1;
		public static final int CTRL_CMD_RSP = 2;
		public static final int CTRL_GET_STATUS_REQ = 3;
		public static final int CTRL_GET_STATUS_RSP = 4;
		/* CLOUD_GET_TARGET */
		public static final int CLOUD_GET_TARGET_REQ = 1;
		public static final int CLOUD_GET_TARGET_RSP = 2;
		/* CLOUD_AUTH */
		public static final int CLOUD_AUTH_REQ = 1;
		public static final int CLOUD_AUTH_RSP = 2;
		/* CLOUD_HEARTBEAT */
		public static final int CLOUD_HEARTBEAT_REQ = 1;
		public static final int CLOUD_HEARTBEAT_RSP = 2;
		/* CLOUD_MSG_CTRL_C2R */
		public static final int CLOUD_MSG_CTRL_C2R_REQ = 1;
		public static final int CLOUD_MSG_CTRL_C2R_RSP = 2;
		public static final int CLOUD_MSG_CTRL_C2R_DO_OTA_REQ = 3;
		public static final int CLOUD_MSG_CTRL_C2R_DO_OTA_RSP = 4;
	    public static final int CLOUD_MSG_CTRL_C2R_TELL_SHARE_REQ = 5;
	    public static final int CLOUD_MSG_CTRL_C2R_TELL_SHARE_RSP = 6;
	    public static final int CLOUD_MSG_CTRL_C2R_CONFIRM_SHARE_REQ = 7;
	    public static final int CLOUD_MSG_CTRL_C2R_CONFIRM_SHARE_RSP = 8;
		/* CLOUD_MSG_CTRL_R2T */
		public static final int CLOUD_MSG_CTRL_R2T_REQ = 1;
		public static final int CLOUD_MSG_CTRL_R2T_RSP = 2;
		public static final int CLOUD_MSG_CTRL_R2T_DO_OTA_REQ = 3;
		public static final int CLOUD_MSG_CTRL_R2T_DO_OTA_RSP = 4;
	    public static final int CLOUD_MSG_CTRL_R2T_TELL_SHARE_REQ = 5;
	    public static final int CLOUD_MSG_CTRL_R2T_TELL_SHARE_RSP = 6;
	    public static final int CLOUD_MSG_CTRL_R2T_CONFIRM_SHARE_REQ = 7;
	    public static final int CLOUD_MSG_CTRL_R2T_CONFIRM_SHARE_RSP = 8;
	    /* CLOUD_MSG_CTRL_R2T */
		public static final int CLOUD_REPORT_REQ = 1;
		public static final int CLOUD_REPORT_RSP = 2;
		public static final int CLOUD_REPORT_OTA_QUERY_REQ = 3;
		public static final int CLOUD_REPORT_OTA_QUERY_RSP = 4;
		/* CLOUD_IND */
        public static final int CLOUD_IND_CTRL_REQ = 1; 
        public static final int CLOUD_IND_CTRL_RSP = 2;
        public static final int CLOUD_IND_OTA_REQ = 3;
        public static final int CLOUD_IND_OTA_RSP = 4;
        public static final int CLOUD_IND_STATUS_REQ = 5;                                                                                                                              
        public static final int CLOUD_IND_STATUS_RSP = 6;
        public static final int CLOUD_IND_MSG_REQ = 7;
        public static final int CLOUD_IND_MSG_RSP = 8;
		/* 命令映射 */
		public static final int GET_STATE_CMD = CLOUD_GET_TARGET_REQ;
		public static final int CTRL_DEV_CMD = CLOUD_MSG_CTRL_C2R_REQ;
	}
	/** Json中的键名 */
	public class K {
		public static final String PUBLIC_KEY = "public_key";
		public static final String SIGNATURE = "signature";
		public static final String APSSID = "apSsid";
		public static final String TYPE = "type";
		public static final String SSID = "ssid";
		public static final String PASSWD = "passwd";
		public static final String DELAY = "delay"; // ms
		public static final String TIMEOUT = "timeout"; // sec
		public static final String REMOTE_ADDR = "remoteIP";
		public static final String STATUS = "status";
		public static final String MSGSTATUS = "msgstatus";
		public static final String ADDR = "addr";
		public static final String UUID = "uuid";
		public static final String CMD = "cmdId";
		public static final String SUBCMD = "subCmdId";
		public static final String TOKEN = "token";
		public static final String SEQID = "seqId";
		public static final String VERSION = "ver";
		public static final String URL = "url";
		public static final String IAID = "iaId";
		public static final String JARVER = "jarver";
		public static final String LIBVER = "version";
		public static final String AUTH = "auth";
		public static final String MAC = "mac";
		public static final String ACCOUNT = "account";
		public static final String SHARE = "share";
		public static final String ACCEPTED = "accepted";
		public static final String SDEV = "sDev";
		public static final String ZMAC = "mac";
	}
	/** Json中的值 */
	public class V {
		public static final int AIR_CONFIG_TYPE_MULTICAST = 1;
		public static final int AIR_CONFIG_TYPE_BROADCAST = 2;
		public static final int AIR_CONFIG_TYPE_SOFTAP = 3;
		public static final int AIR_CONFIG_TYPE_MAXNUM = 4;
		/** 固件升级 */
		public static final int OTA_TYPE_FIRMWARE = 2;
		/** 固件脚本升级 */
		public static final int OTA_TYPE_FW_SCRIPT = 4;
		/** 联动脚本升级 */
		public static final int OTA_TYPE_IA_SCRIPT = 5;
		/** 广播地址 */
		public static final String BROADCAST_ADDR = "255.255.255.255";
	}
}
