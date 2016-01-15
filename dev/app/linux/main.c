#ifdef __LE_SDK__
#undef __LE_SDK__
#endif

#include "leconfig.h"
#include "protocol.h"
#include "airconfig_ctrl.h"
// #include "pack.h"
// #include "network.h"
#include "protocol.h"


// #define MAXSTRINGLENGTH 128
// #define DieWithUserMessage APPLOG
// #define DieWithSystemMessage APPLOG
// #define PREAMBLE_SIZE (4 - 1)
// #define DELAY_FOR_SEND (10)

// const char *sendString = "bonjour123";


// int configBroadcast(const char *ip, int port)
// {
// 	// struct sockaddr_in myaddr = {0};
// 	// int adrrlen = sizeof(myaddr);
// 	int count = 0;
// 	int i = PREAMBLE_SIZE;
// 	int sock = -1;
// 	int bBroadcast=1;

// 	sock = socket(AF_INET,SOCK_DGRAM,0);
// 	setsockopt(sock, SOL_SOCKET, SO_BROADCAST ,&bBroadcast, sizeof(bBroadcast));

//     count = 1;
// 	for (;;)
// 	{
//     	struct sockaddr_in bcast;
//     	bcast.sin_family = AF_INET;
//         bcast.sin_addr.s_addr = inet_addr(ip);
//     	bcast.sin_port = htons((u_short)port);

    	
// 		ssize_t numBytes = sendto(sock, sendString, strlen(sendString) - i, 0, (struct sockaddr*)&bcast, sizeof(bcast));
//     	//APPLOG("sendto bcast [%s:%u] [%d => %s] \r\n", inet_ntoa(bcast.sin_addr), ntohs(bcast.sin_port), numBytes, sendString);
//     	if (i == 0)
//     		i = PREAMBLE_SIZE;
//     	else
//     		i--;
// 		delayms(DELAY_FOR_SEND);
// 		// delayms(1000);
// 	}
// }

// int configMulticast()
// {
// #define MCAST_FMT "239.101.%d.1"
// #define MCAST_SCOPE 5
// #define MCAST_PORT 1234

// 	char mcastIP[32] = { 0 };
// 	int multicastTTL = 255;
// 	u_char mcTTL = (u_char)multicastTTL;
// 	int i = 0;
// 	int count = 1;
	
// 	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
// 	if (sock < 0)
// 	{
// 		APPLOG("socket() failed!");
// 		return -1;
// 	}
// 	if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, &mcTTL, sizeof(mcTTL)) < 0)
// 	{
// 	    APPLOG("setsockopt() failed!");
// 	    return -2;
// 	}   



// 	// for (i = 0; i < 0; i++)
// 	// {
// 	// 	struct ip_mreq mreq;
// 	// 	memset(g_mcast_ip, 0, sizeof(g_mcast_ip));
// 	// 	APPLOG(g_mcast_ip, MCAST_FMT, count + i);
// 	// 	mreq.imr_multiaddr.s_addr = inet_addr(g_mcast_ip);//多播组的IP
// 	// 	mreq.imr_interface.s_addr = htonl(INADDR_ANY);//本机的默认接口IP
// 	// 	APPLOG("add membership mcast ip :%s\r\n", g_mcast_ip);
// 	// 	if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
// 	// 	{
// 	// 		perror("IP_ADD_MEMBERSHIP");
// 	// 		return -1;
// 	// 	}
// 	// }


// 	count = 1;
// 	for (;;)
// 	{
// 		struct sockaddr_in mcast;
// 		mcast.sin_family = AF_INET;
// 		memset(mcastIP, 0, sizeof(mcastIP));
// 		sprintf(mcastIP, MCAST_FMT, count);
// 	    mcast.sin_addr.s_addr = inet_addr(mcastIP);
// 		mcast.sin_port = htons(MCAST_PORT);

		
// 		ssize_t numBytes = sendto(sock, sendString, strlen(sendString), 0, (struct sockaddr*)&mcast, sizeof(mcast));
// 		//APPLOG("sendto mcast [%s:%u] [%d => %s] \r\n", inet_ntoa(mcast.sin_addr), ntohs(mcast.sin_port), numBytes, sendString);

// 		if (count == MCAST_SCOPE)
// 			count = 1;
// 		else
// 			count++;
// 		delayms(DELAY_FOR_SEND);
// 		// delayms(1000);
// 	}
// 	return 0;	
// }

// void test() {
//     int i = 0;
//     char ssid[] = { "letv-office" };
//     for (i = 0; i < sizeof(ssid); i++) {
//         APPLOG("0x%x, %d\n", ssid[i], ssid[i]);
//     }
// 	// multicast
//     configMulticast();

//      	// broadcast
//     configBroadcast("255.255.255.255", 1234);
//     // configBroadcast("10.57.151.255", 1234);
// }

// #define MAX_DEV_ID 16
// #define MAX_COOKIES 16


// typedef struct {
//     uint8_t magic;
//     uint8_t encType;
//     uint16_t ver;
//     uint16_t len;
//     uint16_t csum;
//     uint16_t timestamp; // op
// }ALIGNED CmnHeader;

// typedef struct {
//     int16_t status; // not all
//     uint16_t cmdId;
//     uint16_t seqId; 
//     uint16_t encsum;
//     uint8_t passThru;
//     uint8_t encType;
//     uint16_t randID;
//     uint8_t devId[MAX_DEV_ID];
//     uint8_t cookies[MAX_COOKIES];
// }ALIGNED CmdHeader;

// typedef struct {
//     uint16_t subCmdId;
//     uint16_t len;
// }ALIGNED PayloadHeader;

static uint8_t sigSDK1[] = {
    0x53, 0xb9, 0x62, 0xa8, 0xc1, 0x52, 0xf0, 0x6d, 0xf0, 0x4f, 0x5c, 0x2a, 0x88, 0xa3, 0x50, 0x32, 
    0x59, 0x3e, 0xeb, 0xb7, 0x20, 0x5d, 0x51, 0x39, 0xff, 0xa4, 0x80, 0x94, 0xfd, 0x25, 0xad, 0x26, 
    0xa8, 0xe9, 0xd4, 0xee, 0x93, 0x24, 0x89, 0xf6, 0x47, 0x63, 0x90, 0x64, 0x04, 0xa6, 0x8f, 0x1e, 
    0x76, 0xe6, 0x25, 0x09, 0xa8, 0x17, 0xda, 0x0c, 0x74, 0x20, 0x66, 0x83, 0xbb, 0xa8, 0xbf, 0xf8, 
    0x43, 0x38, 0x8b, 0x43, 0x60, 0xbf, 0xc1, 0xff, 0x1e, 0x34, 0xa1, 0xec, 0x28, 0x2f, 0xa1, 0x2a, 
    0xc8, 0xcc, 0xd9, 0xd7, 0xdc, 0x02, 0x2a, 0x0a, 0xae, 0x4c, 0xe4, 0xfa, 0xa2, 0xa4, 0x88, 0xa9, 
    0xf6, 0x0c, 0x9a, 0x93, 0x6d, 0x08, 0x36, 0x3c, 0x30, 0x70, 0x12, 0xc2, 0x14, 0xff, 0x2b, 0xa5, 
    0x05, 0x4b, 0xe0, 0xe4, 0xa0, 0xc5, 0xad, 0x12, 0xb1, 0xa1, 0x59, 0x77, 0x3b, 0x44, 0x9e, 0xee
};


static uint8_t sigDEV1[] = {
    0x17, 0xb9, 0xd9, 0x78, 0x06, 0x80, 0xa0, 0x49, 0x32, 0xea, 0xf2, 0x08, 0x4d, 0x7e, 0x4b, 0xb6, 
    0xcd, 0xc5, 0xcf, 0x68, 0x93, 0x84, 0x91, 0xfa, 0x6b, 0xb8, 0x90, 0xcc, 0xa1, 0x9e, 0x8d, 0x61, 
    0x53, 0x08, 0x4f, 0x67, 0x41, 0x39, 0x6c, 0x9a, 0x53, 0x3c, 0xaf, 0xa8, 0x10, 0x93, 0x67, 0x44, 
    0xb6, 0x60, 0x81, 0x05, 0xea, 0xd7, 0x2b, 0xd4, 0x8b, 0xd7, 0xb2, 0xee, 0x07, 0xa9, 0x65, 0x9c, 
    0x98, 0xc7, 0xe7, 0xd5, 0xde, 0x6e, 0xe5, 0x78, 0x9f, 0x10, 0x3e, 0xd7, 0x15, 0x7c, 0x5c, 0x16, 
    0x55, 0xe4, 0xa5, 0xf0, 0x2d, 0xe2, 0xa3, 0x4e, 0x0a, 0x6e, 0x4c, 0x8a, 0x0a, 0xa7, 0x2b, 0x51, 
    0x88, 0x96, 0x8c, 0x86, 0x4b, 0x8b, 0x73, 0x5c, 0xf1, 0xaa, 0xdf, 0xa3, 0xa3, 0xee, 0xf2, 0x98, 
    0x0a, 0x9d, 0xd9, 0xfa, 0x40, 0x29, 0x17, 0x68, 0xba, 0x2c, 0xb8, 0x02, 0x8a, 0xbf, 0xd6, 0x7d 
};

static uint8_t ginPubkeySDK1Der[] =
{
    0x30, 0x81, 0x9F, 0x30, 0x0D, 0x06, 0x09, 0x2A, 0x86, 0x48, 
    0x86, 0xF7, 0x0D, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x81, 
    0x8D, 0x00, 0x30, 0x81, 0x89, 0x02, 0x81, 0x81, 0x00, 0x82, 
    0x24, 0x79, 0x05, 0xEA, 0x23, 0x81, 0x5F, 0xC3, 0x9B, 0x0A, 
    0x24, 0xB5, 0xB6, 0x6C, 0x8C, 0xEA, 0xD8, 0x2B, 0x53, 0x64, 
    0x2A, 0x47, 0x40, 0x72, 0xC9, 0xF3, 0x17, 0x8E, 0xFA, 0x75, 
    0xC7, 0x69, 0xCA, 0x89, 0x23, 0x40, 0x05, 0x7B, 0x63, 0x2C, 
    0x08, 0x84, 0x50, 0xF1, 0xC1, 0x33, 0x2A, 0x53, 0x84, 0x15, 
    0x12, 0x00, 0x39, 0x70, 0xEB, 0x33, 0x3B, 0x5C, 0xE0, 0x0D, 
    0x1E, 0x66, 0x96, 0x4A, 0x80, 0xCA, 0x58, 0xE8, 0xC3, 0x64, 
    0xA7, 0x8A, 0xF3, 0x6B, 0x99, 0x0E, 0x1F, 0xF8, 0x0B, 0x69, 
    0x3C, 0xD3, 0xAC, 0xCB, 0x6F, 0xCC, 0x22, 0x4E, 0xB8, 0x7E, 
    0x7B, 0xEA, 0x9E, 0x29, 0xA0, 0x62, 0x36, 0x76, 0x3C, 0x3C, 
    0xC3, 0x28, 0x04, 0x26, 0x2B, 0x36, 0xF3, 0xBF, 0x82, 0x73, 
    0x3C, 0x69, 0x4E, 0x06, 0x23, 0x28, 0x50, 0x9C, 0x67, 0xD3, 
    0x92, 0x07, 0x29, 0x4A, 0x6D, 0x9A, 0xAB, 0x02, 0x03, 0x01, 
    0x00, 0x01
};
// const int ginPubkeySDK1DerLen = sizeof(ginPubkeySDK1Der);

static uint8_t ginPubkeyDEV1Der[] =
{
    0x30, 0x81, 0x9F, 0x30, 0x0D, 0x06, 0x09, 0x2A, 0x86, 0x48, 
    0x86, 0xF7, 0x0D, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x81, 
    0x8D, 0x00, 0x30, 0x81, 0x89, 0x02, 0x81, 0x81, 0x00, 0xC2, 
    0xC9, 0x03, 0x30, 0xBD, 0x06, 0xAE, 0x23, 0x5F, 0xCF, 0x6E, 
    0xD4, 0x36, 0x17, 0x05, 0x53, 0x67, 0x00, 0xDC, 0x3D, 0xC0, 
    0xD7, 0x23, 0x59, 0xC4, 0x50, 0x8B, 0xEA, 0xB3, 0x3F, 0xAD, 
    0x96, 0x64, 0x33, 0xC2, 0x90, 0x96, 0x19, 0x43, 0xB0, 0x1D, 
    0xB6, 0x45, 0xDC, 0x05, 0x70, 0xF1, 0xBD, 0x73, 0xBC, 0x5E, 
    0x67, 0x5B, 0xF5, 0x14, 0xD1, 0xA1, 0x7C, 0xCF, 0x62, 0x8A, 
    0x0A, 0xAB, 0x66, 0xEA, 0x00, 0x78, 0x8D, 0x26, 0xE5, 0x6C, 
    0x96, 0x43, 0x52, 0x5B, 0xCC, 0x84, 0x04, 0xA2, 0x1A, 0x70, 
    0x94, 0x1A, 0x0D, 0xA5, 0x1D, 0xFD, 0xCC, 0x0E, 0xD8, 0x8C, 
    0x14, 0xE7, 0x86, 0xCB, 0x65, 0xE6, 0xBC, 0x12, 0xC3, 0x2D, 
    0x95, 0x4F, 0xDE, 0xE5, 0x3C, 0x80, 0x32, 0xDF, 0x4D, 0xF0, 
    0xAA, 0x5B, 0xE2, 0x56, 0xCD, 0x3A, 0xA8, 0x10, 0x16, 0x6B, 
    0x10, 0xAE, 0x48, 0x3D, 0x06, 0xE2, 0xFB, 0x02, 0x03, 0x01, 
    0x00, 0x01
};
// const int ginPubkeyDEV1DerLen = sizeof(ginPubkeyDEV1Der);

static uint8_t ginPubkeyDEV2Der[] =
{
    0x30, 0x81, 0x9F, 0x30, 0x0D, 0x06, 0x09, 0x2A, 0x86, 0x48, 
    0x86, 0xF7, 0x0D, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x81, 
    0x8D, 0x00, 0x30, 0x81, 0x89, 0x02, 0x81, 0x81, 0x00, 0x91, 
    0xCE, 0xE0, 0x39, 0xED, 0x8C, 0x76, 0xBC, 0x7E, 0xF3, 0xD5, 
    0x1A, 0x7D, 0x33, 0x3D, 0x23, 0xBF, 0xCB, 0x4C, 0xE3, 0x26, 
    0x37, 0xAA, 0x73, 0xE0, 0x93, 0x4F, 0x8E, 0xD0, 0x40, 0x7D, 
    0x86, 0x19, 0x91, 0xDD, 0xFE, 0x46, 0xEB, 0x05, 0x01, 0x6A, 
    0x96, 0xDB, 0x70, 0x85, 0x75, 0xFB, 0x5A, 0xE7, 0x30, 0x6A, 
    0xCA, 0xFD, 0x3F, 0xD1, 0xD2, 0xF8, 0x7D, 0x23, 0xD4, 0x88, 
    0x9F, 0x7E, 0x0E, 0xDA, 0xE1, 0x7F, 0x23, 0x03, 0x00, 0x23, 
    0x6F, 0x71, 0x52, 0xAD, 0xC1, 0xEC, 0x24, 0xB0, 0xC8, 0x16, 
    0x16, 0x38, 0x24, 0x8A, 0x97, 0x70, 0x14, 0x1A, 0xFF, 0x23, 
    0x54, 0x3A, 0x80, 0x86, 0x54, 0xDA, 0xC2, 0x78, 0xBF, 0x2C, 
    0x45, 0x35, 0xA1, 0xC8, 0xC8, 0x91, 0xC7, 0x81, 0x09, 0x61, 
    0x35, 0x1E, 0x01, 0xA9, 0xCC, 0x21, 0x00, 0xB6, 0xC3, 0x72, 
    0x54, 0x66, 0x4A, 0x1F, 0xEB, 0x07, 0x5B, 0x02, 0x03, 0x01, 
    0x00, 0x01
};
// const int ginPubkeyDEV2DerLen = sizeof(ginPubkeyDEV2Der);


union {  
    int number;  
    char s;  
}check;  
  
int testBigEndin() {  
    check.number = 0x01000002;  
    return (check.s == 0x01);  
}

/* ------------------------------- simulator ---------------------------------- */

void waitForInput(void *ctx) {
    char c;
    NodeData node = { 0 };
redo:

    c = getchar();
    // node.timeStamp = halGetTimeStamp();
    // node.timeoutRef = 200;
    switch (c) {
    case '\n':
        goto redo;
        break;
    case '0':
        node.cmdId = LELINK_CMD_HELLO_REQ; 
        node.subCmdId = LELINK_SUBCMD_HELLO_REQ;
        break;
    case '1':
        node.cmdId = LELINK_CMD_DISCOVER_REQ; 
        node.subCmdId = LELINK_SUBCMD_DISCOVER_REQ;
        strncpy(node.ndIP, "255.255.255.255", MAX_IPLEN); // TODO: caution
        node.ndPort = LOCAL_PORT;
        break;
    case '2':
        node.cmdId = LELINK_CMD_CTRL_REQ; 
        node.subCmdId = LELINK_SUBCMD_CTRL_CMD_REQ;
        strncpy(node.ndIP, LOCAL_TEST_IP, MAX_IPLEN); // TODO: caution 
        // strncpy(node.ndIP, "192.168.3.100", MAX_IPLEN); // TODO: caution
        node.ndPort = LOCAL_PORT;
        // APPLOG("2 cmdId[%d], subCmdId[%d]\r\n", node.cmdId, node.subCmdId);
#ifdef TEST_SDK
        if (!node.uuid[0]) {
            uint8_t peerToken[] = {0x15, 0x7e, 0x83, 0x5e, 0x6c, 0x0b, 0xc5, 0x54, 0x74, 0xab, 0xcd, 0x91, 0xe0, 0x0e, 0x69, 0x81};
            memcpy(node.token, peerToken, sizeof(node.token)); 
        }
#endif
        break;    
    // case '2':
    //     node.cmdId = LELINK_CMD_DEVNOTICE_REQ; 
    //     node.subCmdId = LELINK_SUBCMD_DEVNOTICE_REQ; 
    //     break;
    case '3':
        node.cmdId = LELINK_CMD_CTRL_REQ; 
        node.subCmdId = LELINK_SUBCMD_CTRL_GET_STATUS_REQ;
        strncpy(node.ndIP, LOCAL_TEST_IP, MAX_IPLEN); // TODO: caution 
        // strncpy(node.ndIP, "192.168.3.100", MAX_IPLEN); // TODO: caution
        node.ndPort = LOCAL_PORT;
#ifdef TEST_SDK
        if (!node.uuid[0]) {
            uint8_t peerToken[] = {0x15, 0x7e, 0x83, 0x5e, 0x6c, 0x0b, 0xc5, 0x54, 0x74, 0xab, 0xcd, 0x91, 0xe0, 0x0e, 0x69, 0x81};
            memcpy(node.token, peerToken, sizeof(node.token)); 
        }
#endif
        break;
    case '4':
        node.cmdId = LELINK_CMD_CLOUD_GET_TARGET_REQ;
        node.subCmdId = LELINK_SUBCMD_CLOUD_GET_TARGET_REQ; 
        // getOriServerIP(node.ndIP, sizeof(node.ndIP), &node.ndPort);
        break;
    case '5':
        node.cmdId = LELINK_CMD_CLOUD_HEARTBEAT_REQ;
        node.subCmdId = LELINK_SUBCMD_CLOUD_HEARTBEAT_REQ; 
        // getOriServerIP(node.ndIP, sizeof(node.ndIP), &node.ndPort);
        break;
    case '6': {
        // set peer token
        uint8_t peerToken[] = {0x15, 0x7e, 0x83, 0x5e, 0x6c, 0x0b, 0xc5, 0x54, 0x74, 0xab, 0xcd, 0x91, 0xe0, 0x0e, 0x69, 0x81}; 
        // strncpy(node.ndIP, "192.168.253.4", MAX_IPLEN);
        // strncpy(node.ndIP, "192.168.1.101", MAX_IPLEN);
        // strncpy(node.ndIP, LOCAL_TEST_IP, MAX_IPLEN);
        // strncpy(node.ndIP, "172.27.35.15", MAX_IPLEN);
        // node.ndPort = TEST_PORT;
        node.cmdId = LELINK_CMD_CLOUD_MSG_CTRL_C2R_REQ;
        node.subCmdId = LELINK_SUBCMD_CLOUD_MSG_CTRL_C2R_REQ;

        // set peer uuid
        memcpy(node.uuid, "10000100011000510005123456abcdef", MAX_UUID);
        memcpy(node.token, peerToken, AES_LEN);

        }
        break;
    case '7': {
        node.cmdId = LELINK_CMD_CLOUD_REPORT_REQ;
        node.subCmdId = LELINK_SUBCMD_CLOUD_REPORT_REQ; 
        }
        break;
    case '8': {
        node.cmdId = LELINK_CMD_CLOUD_REPORT_REQ;
        node.subCmdId = LELINK_SUBCMD_CLOUD_REPORT_OTA_QUERY_REQ; 
        }
        break;
    case '9': {
        node.cmdId = LELINK_CMD_CLOUD_REPORT_REQ;
        node.subCmdId = LELINK_SUBCMD_CLOUD_REPORT_OTA_DO_REQ; 
        }
        break;
    }
    
    // MUTEX_LOCK;

    int counts = 1;
    // if (LELINK_CMD_DISCOVER_REQ == node.cmdId) {
    //     counts = 1;
    // }
    while (counts--)
        lelinkNwPostCmd(ctx, &node);
    // MUTEX_UNLOCK;

}

void thread_input_check(void *arg) {
    void *ctx_r2r = (void *)arg;
    while (1) {
        waitForInput(ctx_r2r);
        delayms(100);
    }
}

// static uint8_t signatureSDK1[] = { 15, 41, -82, 122, -65, -52, 19, -111, 50, 57, 20, -121, -68, -47, 47, 110, 103, -118, -75, 114, -96, -30, 99, -13, -18, -112, 64, -128, -3, -119, 75, 29, 93, 32, -73, -120, 56, -7, 7, -56, 89, -127, -25, -82, -114, -105, 111, -86, 73, -22, -23, -51, 93, 29, 66, 69, -52, -11, 111, -120, 9, -32, -7, -74, 54, -68, 65, 117, 30, -128, -109, 5, 111, -33, 81, -89, -62, -24, -96, -112, 52, 92, 35, -110, -70, 0, 64, 121, 50, -51, -105, 16, -49, -60, -19, 127, -70, 75, 102, 27, -88, -87, 77, 21, -64, 91, -34, 19, -74, -85, 102, 47, -54, 43, 126, -127, 85, 11, -126, 9, 103, -70, 72, -23, -34, -40, 90, -26 };
// static uint8_t signatureDev1[] = { 23, -71, -39, 120, 6, -128, -96, 73, 50, -22, -14, 8, 77, 126, 75, -74, -51, -59, -49, 104, -109, -124, -111, -6, 107, -72, -112, -52, -95, -98, -115, 97, 83, 8, 79, 103, 65, 57, 108, -102, 83, 60, -81, -88, 16, -109, 103, 68, -74, 96, -127, 5, -22, -41, 43, -44, -117, -41, -78, -18, 7, -87, 101, -100, -104, -57, -25, -43, -34, 110, -27, 120, -97, 16, 62, -41, 21, 124, 92, 22, 85, -28, -91, -16, 45, -30, -93, 78, 10, 110, 76, -118, 10, -89, 43, 81, -120, -106, -116, -122, 75, -117, 115, 92, -15, -86, -33, -93, -93, -18, -14, -104, 10, -99, -39, -6, 64, 41, 23, 104, -70, 44, -72, 2, -118, -65, -42, 125 };
// static uint8_t signatureDev2[] = { 53, -122, -102, 101, 21, 31, -123, 27, 45, -115, -17, 122, 14, -113, 63, 51, -116, -47, -84, 65, 61, 90, 85, -5, 80, -74, 66, 70, 74, 72, -102, -65, 108, 32, -76, -59, -119, -77, -107, -83, 18, -68, 19, 37, -98, -96, -128, 101, 90, 120, 113, -29, -7, 116, -104, 77, -12, -51, -59, 3, -40, 103, -69, 48, 45, -122, -13, -47, -69, 103, 103, -97, -76, 40, -41, -3, 36, -66, -63, -25, 75, -4, -22, 12, 122, -26, 25, 68, 44, -79, -9, -53, -90, 67, 37, 40, -82, 45, -12, -36, 64, 27, -20, -9, -53, -69, 73, 41, -14, -56, 19, 113, 2, -98, -96, -34, 11, 43, 13, -41, 107, -53, 70, 70, 68, 39, -65, 2 };
// int genSig(uint8_t *sig, int len) {
// 	int row = 8;
// 	int col = 16;
// 	int i = 0, j = 0;
// 	for (i = 0; i < row; i++) {
// 		for (j = 0; j < col; j++) {
//     		APPPRINTF("0x%02x, ", sig[i*col + j]);
// 		}
//     	APPPRINTF("\n");
// 	}
// 	return 0;
// }

#define PORT_ONLY_FOR_VM 0 // (NW_SELF_PORT + 100) // the port for r2r should be 0, 
#if 1
int main(int argc, char *argv[]) {
    pthread_t id;
    int i, ret = 0;
    //APPPRINTF("sdk1 \n");
    //ret = genSig(signatureSDK1, sizeof(signatureSDK1));
    //APPPRINTF("dev1 \n");
    //ret = genSig(signatureDev1, sizeof(signatureDev1));
	//APPPRINTF("dev2 \n");
	//ret = genSig(signatureDev2, sizeof(signatureDev2));
    APPLOG("node size [%d]\r\n", sizeof(NodeData));
    LelinkInfo info;
#ifdef TEST_SDK
    memcpy(info.uuid, "24e7d1f0-cbda-481a-9FFFFFFFFFFFF", MAX_UUID);
    info.pubkey = ginPubkeySDK1Der;
    info.pubkeyLen = sizeof(ginPubkeySDK1Der);
    info.signature = sigSDK1;
    info.signatureLen = RSA_LEN;
    lelinkInit(&info);
#endif 
#ifdef TEST_DEV
    // memcpy(info.uuid, "10000100011000510005123456abcdef", MAX_UUID);
    // info.pubkey = ginPubkeyDEV1Der;
    // info.pubkeyLen = sizeof(ginPubkeyDEV1Der);
    // info.signature = sigDEV1;
    // info.signatureLen = RSA_LEN;
    lelinkInit(NULL);
#endif 

    char *ip = argv[1];
    int port = atoi(argv[2]);

    if (argc != 3) {
        ip = REMOTE_IP;
        port = REMOTE_PORT;
    }

    void *ctx_r2r = (void *)lelinkNwNew(ip, port, PORT_ONLY_FOR_VM, 0);
    void *ctx_qa = (void *)lelinkNwNew(NULL, 0, NW_SELF_PORT, 0);


    ret = pthread_create(&id, NULL, (void *)thread_input_check, ctx_r2r);
    if (ret != 0) {
        APPLOGE("Create pthread error \r\n");
        return -1;
    }

    while (1) {
        lelinkDoPollingQ2A(ctx_qa);
        
        // share the QA queue to handle the remote ctrl
        lelinkDoPollingR2R(ctx_r2r);
        
        delayms(100);

        // pollingState(0);
    }
    
    lelinkNwDelete(ctx_r2r);
    lelinkNwDelete(ctx_qa);

    return 0;
}

#else

int main(int argc, char** argv) {
  
    //test();
    // int ret = testBigEndin();
    // int a = sizeof(double);
    // a = sizeof(CommonHeader);
    // a = sizeof(CmdHeader);
    // a = sizeof(PayloadHeader);


    void *context = airconfig_new("SSID=TP-LINK_JJFA1,PASSWD=987654321,AES=912EC803B2CE49E4A541068D495AB570,TYPE=1,DELAY=10");
    // void *context = airconfig_new("SSID=Xiaomi_A7DD,PASSWD=987654321,AES=912EC803B2CE49E4A541068D495AB570,TYPE=2,DELAY=10");
	// void *context = airconfig_new("SSID=360WiFi-JJFA1,PASSWD=987654321,AES=912EC803B2CE49E4A541068D495AB570,TYPE=1,DELAY=10");
	// void *context = airconfig_new("SSID=ff,PASSWD=fengfeng2qiqi,AES=912EC803B2CE49E4A541068D495AB570,TYPE=1,DELAY=10");

    while (1) {
        if (airconfig_do_config(context)) {
            break;
        }
    };
    
    airconfig_delete(context);
	return (EXIT_SUCCESS);

}

#endif
