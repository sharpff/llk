#ifdef __LE_SDK__
#undef __LE_SDK__
#endif

#include "leconfig.h"
#include "protocol.h"
#include "io.h"
#include "ota.h"

uint8_t ginBeCtrlToken[AES_LEN];
char *ginCtrlUUID = NULL;
char *ginCtrlIP = NULL;
char *ginCtrlCmd1 = NULL;
char *ginCtrlCmd2 = NULL;

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
    case '1': {
            char br[MAX_IPLEN] = {0};
            int ret = 0;
            node.cmdId = LELINK_CMD_DISCOVER_REQ; 
            node.subCmdId = LELINK_SUBCMD_DISCOVER_REQ;

            ret = halGetBroadCastAddr(br, sizeof(br));
            if (0 >= ret) {
                APPLOGE("halGetBroadCastAddr error\r\n");
                return;
            }
            strncpy(node.ndIP, br, ret);
            node.ndPort = LOCAL_TEST_PORT;
        }
        break;
    case '2': {
            node.cmdId = LELINK_CMD_CTRL_REQ; 
            node.subCmdId = LELINK_SUBCMD_CTRL_CMD_REQ;
            APPLOGE("0 cmdId[%d]", node.cmdId);
            strncpy(node.ndIP, ginCtrlIP, MAX_IPLEN); // TODO: caution 
            // strncpy(node.ndIP, "192.168.3.100", MAX_IPLEN); // TODO: caution
            node.ndPort = LOCAL_TEST_PORT;
            // APPLOG("2 cmdId[%d], subCmdId[%d]\r\n", node.cmdId, node.subCmdId);
            APPLOGE("1 cmdId[%d]", node.cmdId);
            if (!node.uuid[0]) {
                // uint8_t peerToken[AES_LEN] = {0x15, 0x7e, 0x83, 0x5e, 0x6c, 0x0b, 0xc5, 0x54, 0x74, 0xab, 0xcd, 0x91, 0xe0, 0x0e, 0x69, 0x79};
                uint8_t peerToken[AES_LEN] = {0};
                if (ginBeCtrlToken[0]) {
                    memcpy(peerToken, ginBeCtrlToken, AES_LEN);
                    APPLOGE("X cmdId[%d]", node.cmdId);
                }
                memcpy(node.token, peerToken, sizeof(node.token)); 
                APPLOGE("2 cmdId[%d]", node.cmdId);
            }
            APPLOGE("3 cmdId[%d]", node.cmdId);
        }
        break;    
    // case '2':
    //     node.cmdId = LELINK_CMD_DEVNOTICE_REQ; 
    //     node.subCmdId = LELINK_SUBCMD_DEVNOTICE_REQ; 
    //     break;
    case '3':
        node.cmdId = LELINK_CMD_CTRL_REQ; 
        node.subCmdId = LELINK_SUBCMD_CTRL_GET_STATUS_REQ;
        strncpy(node.ndIP, ginCtrlIP, MAX_IPLEN); // TODO: caution 
        // strncpy(node.ndIP, "192.168.3.100", MAX_IPLEN); // TODO: caution
        node.ndPort = LOCAL_TEST_PORT;
        if (!node.uuid[0]) {
            // uint8_t peerToken[AES_LEN] = {0x15, 0x7e, 0x83, 0x5e, 0x6c, 0x0b, 0xc5, 0x54, 0x74, 0xab, 0xcd, 0x91, 0xe0, 0x0e, 0x69, 0x79};
            uint8_t peerToken[AES_LEN] = {0};
            if (ginBeCtrlToken[0]) {
                memcpy(peerToken, ginBeCtrlToken, AES_LEN);
            }
            memcpy(node.token, peerToken, sizeof(node.token)); 
        }
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
        node.cmdId = LELINK_CMD_CLOUD_REPORT_REQ;
        node.subCmdId = LELINK_SUBCMD_CLOUD_REPORT_REQ; 
        }
        break;
    case '7': {
        // set peer token
        if (!node.uuid[0]) {
            uint8_t peerToken[AES_LEN] = {0};
        // set peer token
            if (ginBeCtrlToken[0]) {
                memcpy(peerToken, ginBeCtrlToken, AES_LEN);
            }
            memcpy(node.token, peerToken, sizeof(node.token)); 
        }
        // strncpy(node.ndIP, "192.168.253.4", MAX_IPLEN);
        // strncpy(node.ndIP, "192.168.1.101", MAX_IPLEN);
        // strncpy(node.ndIP, ginCtrlIP, MAX_IPLEN);
        // strncpy(node.ndIP, "172.27.35.15", MAX_IPLEN);
        // node.ndPort = TEST_PORT;
        node.cmdId = LELINK_CMD_CLOUD_MSG_CTRL_C2R_REQ;
        node.subCmdId = LELINK_SUBCMD_CLOUD_MSG_CTRL_C2R_REQ;

        // set peer uuid
        memcpy(node.uuid, ginCtrlUUID, MAX_UUID);

        }
        break;
    case '8': {
        node.cmdId = LELINK_CMD_CLOUD_REPORT_REQ;
        node.subCmdId = LELINK_SUBCMD_CLOUD_REPORT_OTA_QUERY_REQ; 
        }
        break;
    case '9': {
        node.cmdId = LELINK_CMD_CLOUD_MSG_CTRL_C2R_REQ;
        node.subCmdId = LELINK_SUBCMD_CLOUD_MSG_CTRL_C2R_DO_OTA_REQ; 

        // set peer uuid (for 1st, for cloud)
        memcpy(node.uuid, ginCtrlUUID, MAX_UUID);

        // set peer token (for 2nd)
        uint8_t peerToken[AES_LEN] = {0x83, 0x2d, 0x62, 0x4f, 0x28, 0x84, 0x11, 0x9c, 0x42, 0xb3, 0x83, 0x29, 0xfe, 0x9a, 0xcf, 0x53};
        if (ginBeCtrlToken[0]) {
            memcpy(peerToken, ginBeCtrlToken, AES_LEN);
        }
        memcpy(node.token, peerToken, sizeof(node.token)); 

        }
        break;
    case 'r': {
        node.cmdId = LELINK_CMD_CLOUD_HEARTBEAT_REQ;
        node.subCmdId = LELINK_SUBCMD_CLOUD_STATUS_CHANGED_REQ; 
        }
        break;
    case 'l': {
            char br[MAX_IPLEN] = {0};
            int ret = 0;
            node.cmdId = LELINK_CMD_DISCOVER_REQ;
            node.subCmdId = LELINK_SUBCMD_DISCOVER_STATUS_CHANGED_REQ; 
            ret = halGetBroadCastAddr(br, sizeof(br));
            if (0 >= ret) {
                APPLOGE("halGetBroadCastAddr error\r\n");
                return;
            }
            strncpy(node.ndIP, br, ret);
            node.ndPort = LOCAL_TEST_PORT;
        }
    case 't': {
        node.cmdId = LELINK_CMD_CLOUD_MSG_CTRL_C2R_REQ;
        node.subCmdId = LELINK_SUBCMD_CLOUD_MSG_CTRL_C2R_TELL_SHARE_REQ;

        // set peer uuid (for 1st, for cloud)
        memcpy(node.uuid, ginCtrlUUID, MAX_UUID);

        // set peer token (for 2nd)
        uint8_t peerToken[AES_LEN] = {0x83, 0x2d, 0x62, 0x4f, 0x28, 0x84, 0x11, 0x9c, 0x42, 0xb3, 0x83, 0x29, 0xfe, 0x9a, 0xcf, 0x53};
        if (ginBeCtrlToken[0]) {
            memcpy(peerToken, ginBeCtrlToken, AES_LEN);
        }
        memcpy(node.token, peerToken, sizeof(node.token)); 

        }
        break;
    case 'c': {
        node.cmdId = LELINK_CMD_CLOUD_MSG_CTRL_C2R_REQ;
        node.subCmdId = LELINK_SUBCMD_CLOUD_MSG_CTRL_C2R_CONFIRM_SHARE_REQ;

        // set peer uuid (for 1st, for cloud)
        memcpy(node.uuid, ginCtrlUUID, MAX_UUID);

        // set peer token (for 2nd)
        uint8_t peerToken[AES_LEN] = {0x83, 0x2d, 0x62, 0x4f, 0x28, 0x84, 0x11, 0x9c, 0x42, 0xb3, 0x83, 0x29, 0xfe, 0x9a, 0xcf, 0x53};
        if (ginBeCtrlToken[0]) {
            memcpy(peerToken, ginBeCtrlToken, AES_LEN);
        }
        memcpy(node.token, peerToken, sizeof(node.token)); 

        }
        break;
    case 'd': {
        char br[MAX_IPLEN] = {0};
        int ret = 0;
        node.cmdId = LELINK_CMD_DISCOVER_REQ; 
        node.subCmdId = LELINK_SUBCMD_DISCOVER_STATUS_CHANGED_REQ;

        ret = halGetBroadCastAddr(br, sizeof(br));
        if (0 >= ret) {
            APPLOGE("halGetBroadCastAddr error\r\n");
            return;
        }
        strncpy(node.ndIP, br, ret);
        node.ndPort = LOCAL_TEST_PORT;
        }break;
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
    void *ctxR2R = (void *)arg;
    while (1) {
        waitForInput(ctxR2R);
        delayMS(100);
    }
}


#define PORT_ONLY_FOR_VM 0 // (NW_SELF_PORT + 100) // the port for r2r should be 0, 

//#define DO_AIR_CONFIG    
//#define ENABLE_WIFI_SOFT_AP 1

#ifndef DO_AIR_CONFIG
int main(int argc, char *argv[]) {
    pthread_t id;
    int i, ret = 0;
    AuthData authData;

    // {
    //     // char buf[1024] = {0};
    //     extern const uint8_t ginScript[];
    //     ret = lelinkStorageInit(0x1C2000, 0x3E000, 256);
    //     if (0 > ret) {
    //         // APPLOGE("lelinkStorageInit ret[%d]\r\n", ret);
    //     }
    //     printf(ginScript);
    //     return 0;
    // }
    if (argc < 5) {
        APPLOG("EXEC [target uuid] [target ip] [cmd1] [cmd2]");
        APPLOG("default uuid[%s] ip[%s] will be used", ginCtrlUUID, ginCtrlIP);
        return -1;
    } else {
        ginCtrlUUID = argv[1];
        ginCtrlIP = argv[2];
        ginCtrlCmd1 = argv[3];
        ginCtrlCmd2 = argv[4];
        APPLOG("default uuid[%s] ip[%s] cmd1[%s], cmd2[%s]", ginCtrlUUID, ginCtrlIP, ginCtrlCmd1, ginCtrlCmd2);
        // return 0;
    }


    //APPPRINTF("sdk1 \n");
    //ret = genSig(signatureSDK1, sizeof(signatureSDK1));
    //APPPRINTF("dev1 \n");
    //ret = genSig(signatureDev1, sizeof(signatureDev1));
    //APPPRINTF("dev2 \n");
    //ret = genSig(signatureDev2, sizeof(signatureDev2));
    APPLOG("node size [%d]\r\n", sizeof(NodeData));

    // memcpy(authData.uuid, ginUUIDSDK3, MAX_UUID);
    // authData.pubkeyLen = sizeof(ginPubkeySDK3Der);
    // memcpy(authData.pubkey, ginPubkeySDK3Der, authData.pubkeyLen);
    // authData.signatureLen = RSA_LEN;
    // memcpy(authData.signature, ginSigSDK3, authData.signatureLen);
    // lelinkInit(&authData);

    // sector 0x1000(512pcs), block 0x10000(32pcs)
    ret = lelinkStorageInit(0x1C2000, 0x3E000, 0x1000);
    if (0 > ret) {
        APPLOGE("lelinkStorageInit ret[%d]\r\n", ret);
        return -2;
    }
    ret = lelinkInit(NULL);
    if (0 > ret) {
        APPLOGE("lelinkInit failed [%d]\r\n", ret);
        return -3;
    }


    void *ctxR2R = (void *)lelinkNwNew(REMOTE_BAK_IP, REMOTE_BAK_PORT, PORT_ONLY_FOR_VM, NULL);
    void *ctxQ2A = (void *)lelinkNwNew(NULL, 0, NW_SELF_PORT, NULL);


    ret = pthread_create(&id, NULL, (void *)thread_input_check, ctxR2R);
    if (ret != 0) {
        APPLOGE("Create pthread error \r\n");
        return -4;
    }

    while (1) {
        lelinkPollingState(100, ctxR2R, ctxQ2A);
        
        // lelinkDoPollingQ2A(ctxQ2A);
        // lelinkDoPollingR2R(ctxR2R);
        // delayMS(100);
    }
    
    lelinkNwDelete(ctxR2R);
    lelinkNwDelete(ctxQ2A);

    return 0;
}

#else

int main(int argc, char** argv) {

    char configInfo[256] = {0};
    int delay = 10, type = 2;
    const char *configFmt = "SSID=%s,PASSWD=%s,AES=%s,TYPE=%d,DELAY=%d";
    int ret = 0;
    //test();
    // int ret = testBigEndin();
    // int a = sizeof(double);
    // a = sizeof(CommonHeader);
    // a = sizeof(CmdHeader);
    // a = sizeof(PayloadHeader);

	// uint16_t a = 512;
 //    uint16_t inAir = 0;
 //    uint16_t magic = 0x1221;

 //    inAir = a ^ magic;
 //    a = inAir ^ magic;

    char *ssid = argv[1];
    char *passwd = argv[2];
    delay = argv[3] ? (atoi(argv[3]) < 10 ? 10 : atoi(argv[3])) : 10;

    if (argc < 4) {
        APPLOG("EXEC [ssid] [passwd] [delay] [key as optional]");
        return -1;
    }

    ret = lelinkStorageInit(0x1C2000, 0x3E000, 0x1000);
    if (0 > ret) {
        APPLOGE("lelinkStorageInit ret[%d]\r\n", ret);
        return -2;
    }

    ret = lelinkInit(NULL);
    if (0 > ret) {
        APPLOGE("lelinkInit failed [%d]\r\n", ret);
        return -3;
    }

    // configInfo = "SSID=TP-LINK_JJFA1,PASSWD=987654321,AES=912EC803B2CE49E4A541068D495AB570,TYPE=2,DELAY=10";
    // configInfo = "SSID=Xiaomi_A7DD,PASSWD=987654321,AES=912EC803B2CE49E4A541068D495AB570,TYPE=2,DELAY=10";
    // configInfo = "SSID=360WiFi-JJFA1,PASSWD=987654321,AES=912EC803B2CE49E4A541068D495AB570,TYPE=1,DELAY=10";
    // configInfo = "SSID=TP-LINK_08F8,PASSWD=12345678,AES=912EC803B2CE49E4A541068D495AB570,TYPE=1,DELAY=10";
    // configInfo = "SSID=TP-LINK_564FCE,PASSWD=64373537,AES=912EC803B2CE49E4A541068D495AB570,TYPE=2,DELAY=10";
    // configInfo = "SSID=ff,PASSWD=fengfeng2qiqi,AES=912EC803B2CE49E4A541068D495AB570,TYPE=1,DELAY=10";
    APPLOG("starting with [%s:%s][%d] type[%d]...", ssid, passwd, delay, type);

#if  ENABLE_WIFI_SOFT_AP
    while (1) {
        ret = softApDoConfig(ssid, passwd, delay);
    }
#else
    while (1) {
        sprintf(configInfo, configFmt, ssid, passwd, "912EC803B2CE49E4A541068D495AB570", type, delay);
        APPLOG("start => %s", configInfo);
        APPLOG("starting with [%s:%s][%d] type[%d]...", ssid, passwd, delay, type);
        ret = lelinkDoConfig(configInfo);
        if (0 > ret) {
            APPLOG("waiting ...");
            delayMS(1000);
        } else {
            APPLOG("ending with [%s:%s][%d] type[%d]...", ssid, passwd, delay, type);
            type = type == 1 ? 2 : 1;
            APPLOG("starting with [%s:%s][%d] type[%d]...", ssid, passwd, delay, type);
        }

    }
#endif
	return (EXIT_SUCCESS);

}

#endif

