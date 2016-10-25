#include "MICO.h"
#include "MICONotificationCenter.h"
#include "wifi_monitor.h"
#include "t11_debug.h"
#include "airconfig.h"

//是否已经锁定通道了
static bool is_channel_locked = false;

//打印的id
static int idx = 0;

//切换过程中，当前的channel
static int channel = 1; //1~13

//乐视api中关心的，只需要传入即可
static int channel_locked[MAX_CHANNEL_CARE] = {0};

static uint16_t base=0;
static ap_passport_t account= {0};

//锁定后，需要过滤的mac地址
static uint8_t phone_mac[6] = {0};
static bool get_info_success = false;

static mico_mutex_t* airconfig_sem = NULL;
static mico_timer_t* channel_change_timer = NULL;
static mico_timer_t* channel_timeout_timer = NULL;
static void change_channel(void* arg);
static void channel_timeout(void* arg);

//lelink配置是否正在进行中（即使配置成功了，也可以处于配置过程中）
static bool is_airconfig_thread_running = false;

#define MLAN_MAX_SSID_LENGTH 32
static char ginSsid[MLAN_MAX_SSID_LENGTH] = {0};

static void monitor_cb(uint8_t *data , int len){
    if(get_info_success){
        return;
    }
    if(len<22){
        //长度小于22，则不合法
        return;
    }
    if(data[0] == 0x80 && len > 39){
        memset(ginSsid, 0, MLAN_MAX_SSID_LENGTH);
        memcpy(ginSsid, &data[38], data[37]);
        return;
    }

    //第二个字节的前两位
    uint8_t to_from_ds = data[1] & 0x03;
    uint8_t *addr1 = data+4;
    uint8_t *addr2 = data+10;
    uint8_t *addr3 = data+16;
    
    uint8_t* da = NULL;
    uint8_t* sa = NULL;
    uint8_t* bssid = NULL;
    
    switch(to_from_ds){
    case 0:     // 00 -> DA     SA      BSSID
        da = addr1;
        sa = addr2;
        bssid = addr3;
        break;
    case 0x01:  // 10 -> BSSID  SA      DA（接收到的比特顺序跟MCU上的比特顺序是相反的）
        bssid = addr1;
        sa = addr2;
        da = addr3;
        break;
    case 0x02:  // 01 -> DA     BSSID   SA 
        da = addr1;
        bssid = addr2;
        sa = addr3;
        break;  
    default:       
        debug("invalid wifi packet!!!!!!!!!!!!!");
        return;
    }

    //idx ++;
    //if(idx%20==0){
    //    debug("idx = %d",idx);
    //}
    

    target_item_t item;
    item.data = len;
    memcpy(item.mac_src,sa,6);
    memcpy(item.mac_dst,da,6);
    memcpy(item.mac_bssid,bssid,6);
    //debug("monitor_cb [%d] !!!!", is_channel_locked);
    if(!is_channel_locked) {
        //准备锁定信道
        int ret = airconfig_do_sync(&item,channel,channel_locked,&base);
        if(AIRCONFIG_NW_STATE_CHANNEL_LOCKED == ret){
            is_channel_locked = true;
            memcpy(phone_mac,sa,6);
            mico_wlan_set_channel(channel_locked[0]);
            debug("channel[%d] [%d] lock success!!!!", channel_locked[0], channel);
            mico_start_timer(channel_timeout_timer);
        }
    } else {
        if(memcmp(phone_mac,sa,6)){
            return;
        }

        int ret = airconfig_get_info(len, base, &account, ginSsid, strlen(ginSsid));

        if(ret){
            debug("get info success!!!!");
            get_info_success = true;
            mico_stop_timer(channel_timeout_timer);
            mico_wlan_stop_monitor();
            return;
        }
    }
}

/**
 * 重新初始化系统参数
 */
static void reinit(){
    
    is_channel_locked = false;
    idx =0;
    channel = 1;
//    memset(channel_locked,0,MAX_CHANNEL_CARE*sizeof(int));
    for(int i=0;i<MAX_CHANNEL_CARE;i++){
        channel_locked[i] = 0;
    }
    base = 0;
    
    memset(&account,0,sizeof(account));
    
    memset(phone_mac,0,6);
    
    get_info_success = false;
    
    //airconfig_sem只初始化一次
    if(airconfig_sem==NULL){
        airconfig_sem = halMalloc(sizeof(mico_mutex_t));
        mico_rtos_init_mutex(airconfig_sem);
    }
    
    //乐视内部数据重置
    airconfig_reset();
    
    //创建信道切换的timer
    if(channel_change_timer==NULL){
        channel_change_timer = halMalloc(sizeof(mico_timer_t));
        channel_timeout_timer = halMalloc(sizeof(mico_timer_t));
        mico_init_timer(channel_change_timer,30, change_channel, NULL);
        mico_init_timer(channel_timeout_timer,30*1000, channel_timeout, NULL);
    }
}

/**
 * 配置过程中，定时切换wifi的信道
 */
static void change_channel(void* arg){
    if(!get_info_success){
        if(!is_channel_locked){
            //在所有的信道之间跳转
            mico_wlan_set_channel(++channel);
            if(channel==13){
                channel = 1;
            }
        }
        //再次初始化定时器
        mico_reload_timer(channel_change_timer);
    }
}

static void channel_timeout(void* arg){
    debug("channel_timeout!!!!");
    is_channel_locked = false;
    airconfig_reset();
    mico_stop_timer(channel_timeout_timer);
}

//lelink配置是否成功
bool is_lelink_config_done(){
    return get_info_success;
}
//获取lelink配置的用户名和密码
ap_passport_t get_lelink_config_info(){
    return account;
}

/**
 * 停止lelink配置
 */
void stop_lelink_config(){
    if(airconfig_sem==NULL) 
	return;
    mico_rtos_lock_mutex(airconfig_sem);
    
    //没有在执行lelink-config操作
    if(!is_airconfig_thread_running){
        mico_rtos_unlock_mutex(airconfig_sem);
        return ;
    }
    
    // stop monitor
    mico_wlan_stop_monitor();
    is_airconfig_thread_running = false;
    mico_stop_timer(channel_change_timer);
    mico_stop_timer(channel_timeout_timer);
    mico_deinit_timer(channel_change_timer);
    mico_deinit_timer(channel_timeout_timer);
    halFree(channel_change_timer);
    halFree(channel_timeout_timer);
    channel_change_timer = NULL;
    channel_timeout_timer = NULL;
    mico_rtos_unlock_mutex(airconfig_sem);
}

/**
 * 启动lelink配置
 */
void start_lelink_config(){
    
    reinit();
    debug("start caputure packet");
    mico_wlan_monitor_rx_type(WLAN_FILTER_RX_MCAST_DATA);
    mico_wlan_monitor_rx_type(WLAN_FILTER_RX_BEACON);
    // set the callback function to monitor_cb
    mico_wlan_register_monitor_cb(monitor_cb);
    
    // start monitor
    mico_wlan_start_monitor(0);
    
    //如果已经在执行lelink操作了
    mico_rtos_lock_mutex(airconfig_sem);
    if(is_airconfig_thread_running){
        mico_rtos_unlock_mutex(airconfig_sem);
        return ;
    }
    is_airconfig_thread_running = true;
    //启动channel切换(airconfig_sem会保证timer一定处于close的状态)
    mico_start_timer(channel_change_timer);
    mico_rtos_unlock_mutex(airconfig_sem);
    return ;
}



