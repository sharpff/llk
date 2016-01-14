package com.letv.lelink;

public class LeCmd {
	public static final int HELLO_REQ = 7;
	public static final int HELLO_RSP = 8;
	public static final int DISCOVER_REQ = 9;
	public static final int DISCOVER_RSP = 10;
	public static final int CTRL_REQ = 15;
	public static final int CTRL_RSP = 16;

	// do not modify this item
	public static final int CLOUD_GET_TARGET_REQ = 87;
	public static final int CLOUD_GET_TARGET_RSP = 86;
	public static final int CLOUD_AUTH_REQ = 89;
	public static final int CLOUD_AUTH_RSP = 90;
	public static final int CLOUD_HEARTBEAT_REQ = 91;
	public static final int CLOUD_HEARTBEAT_RSP = 92;
	public static final int CLOUD_REPORT_REQ = 93;
	public static final int CLOUD_REPORT_RSP = 94;

	public static final int CLOUD_MSG_CTRL_C2R_REQ = 95;
	public static final int CLOUD_MSG_CTRL_C2R_RSP = 96;
	public static final int CLOUD_MSG_CTRL_R2T_REQ = 97;
	public static final int LOUD_MSG_CTRL_R2T_RSP = 98;

	public class Sub {
		public static final int HELLO_REQ = 1;
		public static final int HELLO_RSP = 2;
		public static final int DISCOVER_REQ = 1;
		public static final int DISCOVER_RSP = 2;
		public static final int DEVNOTICE_REQ = 1;
		public static final int DEVNOTICE_RSP = 2;
		public static final int DEVAUTH_REQ = 1;
		public static final int DEVAUTH_RSP = 2;
		public static final int CTRL_CMD_REQ = 1;
		public static final int CTRL_CMD_RSP = 2;
		public static final int CTRL_GET_STATUS_REQ = 3;
		public static final int CTRL_GET_STATUS_RSP = 4;

		public static final int CLOUD_GET_TARGET_REQ = 1;
		public static final int CLOUD_GET_TARGET_RSP = 2;
		public static final int CLOUD_AUTH_REQ = 1;
		public static final int CLOUD_AUTH_RSP = 2;
		public static final int CLOUD_HEARTBEAT_REQ = 1;
		public static final int CLOUD_HEARTBEAT_RSP = 2;

		public static final int CLOUD_MSG_CTRL_C2R_REQ = 1;
		public static final int CLOUD_MSG_CTRL_C2R_RSP = 2;
		public static final int CLOUD_MSG_CTRL_R2T_REQ = 1;
		public static final int CLOUD_MSG_CTRL_R2T_RSP = 2;

		public static final int CLOUD_MSG_RELAY_C2R_REQ = 1;
		public static final int CLOUD_MSG_RELAY_C2R_RSP = 2;
		public static final int CLOUD_MSG_RELAY_R2T_REQ = 1;
		public static final int CLOUD_MSG_RELAY_R2T_RSP = 2;
		public static final int CLOUD_MSG_NOTIFY_R2T_REQ = 1;
		public static final int CLOUD_MSG_NOTIFY_R2T_RSP = 2;
		public static final int CLOUD_REPORT_REQ = 1;
		public static final int LOUD_REPORT_RSP = 2;
	}

	public class K {
		public static final String SSID = "ssid";
		public static final String PASSWD = "passwd";
		public static final String TIMEOUT = "timeout";
		public static final String REMOTE_ADDR = "remoteIP";

		public static final String STATUS = "status";
		public static final String ADDR = "addr";
		public static final String UUID = "uuid";
		public static final String CMD = "cmdId";
		public static final String SUBCMD = "subCmdId";
		public static final String TOKEN = "token";
		public static final String SEQID = "seqId";
	}

	public class V {
		public static final String BROADCAST_ADDR = "255.255.255.255";
	}
}
