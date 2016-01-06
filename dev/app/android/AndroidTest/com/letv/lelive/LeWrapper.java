package com.letv.lelive;

public class LeWrapper {

	public native int init(LeDelegate delegate);
	public native int exit();

	/*
	* airconfigSync is a blocking method
	* accountInfo: "SSID=letv-office,PASSWD=987654321,AES=912EC803B2CE49E4A541068D495AB570,TYPE=1,DELAY=10"
	* 	SSID: AP account
	*	PASSWD: AP passwd
	*	AES: AES Key 128bits
	*	TYPE: 1-multicuast, 2-broadcast
	*	DELAY: ms
	*/
	public native int airconfigCtrl(LeData accountInfo);

	public native int postDiscovery(LeData data);
	public native int postDevAuth(LeData data);
	public native int post(LeData data);



	// public native LeData recvFromAsync1();
	// public native byte[] recvFromAsync();
	// public native boolean sendToAsync1(LeData data);
	// public native int sendToAsync(byte[] data);

	// public native int airconfig
	// public native String getString(String test);
	// public native LeAccount getAccountInfo();
	// public native boolean getAccountInfo(String ssid, String passwd);
	// public native boolean getAccountInfo(byte[] ssid, byte[] passwd);


}

