#ifdef __LE_SDK__
#undef __LE_SDK__
#endif

#include "leconfig.h"
#include "protocol.h"
#include "io.h"
#include "ota.h"
#include "MICOWLAN.h"
#include "MICODefine.h"
#include "MICO.h"
#include "MICONotificationCenter.h"
#include "state.h"

//模块的联网信息
IPStatusTypedef t11_network_info={0};

#include "t11_debug.h"

void show_mem_info(){
    micoMemInfo_t* mem_info = MicoGetMemoryInfo( );
    debug("allocated_mem = %d, free_mem = %d",mem_info->allocted_memory,mem_info->free_memory); 
}

//uint8_t arr[1024*60];
// 在SDRAM中申请100Kheap
#include "section_config.h"

#define SDRAM_HEAP_SIZE (100*1024)

SDRAM_DATA_SECTION static volatile uint8_t sdram_heap[SDRAM_HEAP_SIZE] = {0};

static void ins_heap(void){
    insert_heap(sdram_heap, SDRAM_HEAP_SIZE);
}

/**
 * 堆栈溢出回调
 */
void t11_stackoverflow_handler(char *taskname, void* arg){
    debug("stack over flow，taskname = %s",taskname);
}

void lelink_thread(void*arg){
    lelink_main();
}

/**
 * dhcp执行成功后的回调函数
 */
void t11_lelink_dhcp_callback(IPStatusTypedef *pnet, void*arg) {
    if (!pnet) {
        debug("dhcp callback parameter error,pnet == null");
        return;
    }
    t11_network_info = *pnet;
    
    debug("dhcp request success: ip = %s, mac = %s", t11_network_info.ip, t11_network_info.mac);
    
}

//  mico_notify_WIFI_Fatal_ERROR,             //void (*function)(mico_Context_t * const inContext);
void t11_fatal_err_handler(mico_Context_t* ctx){
    debug("fatal error");
}

void test_uart();
void start_lelink();

OSStatus MICOStartApplication( mico_Context_t * const inContext ){
    debug("=======================MICOStartApplication =====================");
    //halNetwork中的一些函数需要用到t11_network_info中的变量
    MICOAddNotification( mico_notify_Stack_Overflow_ERROR, t11_stackoverflow_handler );
    memset(&t11_network_info, 0, sizeof(t11_network_info));
    MICOAddNotification(mico_notify_DHCP_COMPLETED,t11_lelink_dhcp_callback);
    MICOAddNotification(mico_notify_WIFI_Fatal_ERROR,t11_fatal_err_handler);
    
    
    show_mem_info();
    ins_heap();
    show_mem_info();
    OSStatus err = kNoErr;
    
    //    mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "lelink_main", lelink_thread, 0x400*4, NULL );
    lelink_main();
    while(1){
        debug("lelink_main done!!!!!!!!!!");
        mico_thread_sleep(5);
    }
}

/**
* 调试的时候直接连接wifi
*/
void t11_debug_connect_wifi(){
    network_InitTypeDef_adv_st wNetConfigAdv;
    char* ap_key = "13091309GZ";
    memset(&wNetConfigAdv, 0x0, sizeof(network_InitTypeDef_adv_st));
    
    strcpy((char*)wNetConfigAdv.ap_info.ssid, "Xiaomi_Team11");
    strcpy((char*)wNetConfigAdv.key, ap_key);
    wNetConfigAdv.key_len = strlen(ap_key);
    wNetConfigAdv.ap_info.security = SECURITY_TYPE_AUTO;
    wNetConfigAdv.ap_info.channel = 0; //Auto
    wNetConfigAdv.dhcpMode = DHCP_Client;
    wNetConfigAdv.wifi_retry_interval = 100;
    micoWlanStartAdv(&wNetConfigAdv);
    
    while(1){
        if(t11_network_info.ip[0]=='\0'){
            debug("wait wifi connect");
            mico_thread_sleep(5);
        }else{
            debug("wifi connect success");
            break;
        }
    }

}
#define app_log debug
#include "MicoSocket.h"
#define buf_size 64
void udp_socket_test(){
    uint8_t g_wifi_status = 0;
    IPStatusTypedef g_para;
    uint8_t *g_ip;
    
    int len;
    int udpSearch_fd = -1;
    fd_set readfds;
    struct timeval_t t;
    struct sockaddr_t addr;
    socklen_t addrLen;
    //  Context = inContext;
    json_object* report = NULL;
    const char *  json_str;
    char recv_buf[1024];
    
    g_ip = (uint8_t *)(&addr.s_ip);
    while(g_wifi_status != 1)
        mico_thread_msleep( 500 );
    mico_thread_sleep(2);
    micoWlanGetIPStatus (&g_para, Station);
    app_log( "local ip = %s", g_para.ip );
    
    udpSearch_fd = socket(AF_INET, SOCK_DGRM, IPPROTO_UDP);
    addr.s_ip = INADDR_ANY;
    addr.s_port = 1122;
    bind(udpSearch_fd, &addr, sizeof(addr));
    
    while(1) {
        FD_ZERO(&readfds);
        t.tv_sec = 10;
        t.tv_usec = 0;
        
        FD_SET(udpSearch_fd, &readfds);
        
        select(1, &readfds, NULL, NULL, &t);
        
        if (FD_ISSET(udpSearch_fd, &readfds)) {
            memset( recv_buf, 0, sizeof(recv_buf) );
            len = recvfrom(udpSearch_fd, recv_buf, sizeof(recv_buf), 0, &addr, &addrLen);
            app_log( "recv data[%d]= %s", len, recv_buf );
            app_log( "remote ip = %d.%d.%d.%d, port = %d", *(g_ip+3), *(g_ip+2), *(g_ip+1), *(g_ip+0), addr.s_port );
        }
    }
    
}

void test_flash(){
    char* buf = malloc(1024);
    if(buf!=NULL){
        halFlashRead(NULL,buf,1024,0, 0);
        t11_print_mem(buf,1024);
        free(buf);
    }
}

/**
 * 乐视主函数
 */ 
int lelink_main() {
    //调试时，手动连接wifi
//    t11_debug_connect_wifi();
    //test_flash();
//    LEPRINTF("test X --%02X--",23);
//    udp_socket_test();
    int i, ret = 0,count=0;
    char *p;

    p = halMalloc(50*1024);
    p = halMalloc(10*1024);
    while(count<200)
    {
        p = halMalloc(25);
        count++;
    }

    ret = lelinkStorageInit(0, 0x3E000, 0x1000);
    if (0 > ret) {
        APPLOGE("lelinkStorageInit ret[%d]\r\n ", ret);
        return -2;
    }
    
    debug("lelinkStorageInit done,ret = %d",ret);
    show_mem_info();
    ret = lelinkInit();
    if (0 > ret) {
        debug("lelinkInit done，ret = %d ",ret);
        show_mem_info();
        return -3;
    }
    debug("lelinkInit success");
    
    
    void *ctxR2R = (void *) lelinkNwNew(REMOTE_BAK_IP, REMOTE_BAK_PORT, 0,NULL);
    //debug("ctxR2R = %p",ctxR2R);
    void *ctxQ2A = (void *) lelinkNwNew(NULL, 0, NW_SELF_PORT, NULL);
    //debug("ctxQ2A = %p",ctxQ2A);
    
    debug("start polling");
    //uint32_t idx = 0;
    while (1) {
        //100
        //uint32_t time_start = mico_get_time();
        StateId stateId = lelinkPollingState(50, ctxR2R, ctxQ2A);
        //debug("stateId = %d, used time %d ms",stateId,mico_get_time()-time_start);
        //show_mem_info();
        //idx++;
    }
    
    lelinkNwDelete(ctxR2R);
    lelinkNwDelete(ctxQ2A);
}

