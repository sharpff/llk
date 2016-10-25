#include "halHeader.h"
//#include "netConfig.h"
#include "airconfig.h"
#include "io.h"
#include "MICODefine.h"
//四个状态，返回大于则继续进行下一步

extern void start_lelink_config();
extern ap_passport_t get_lelink_config_info();
extern void stop_lelink_config();
extern bool is_lelink_config_done();
extern mico_Context_t *getGlobalContext (void);

/*
* 功能: 配置wifi进入monitor模式
* 参数:
*      ptr: 保留
*      ptrLen: 保留
* 返回值:
*      1 - 成功配置, 其它表示失败
*/
int halDoConfig(void *ptr, int ptrLen) {
    
    /*
    * 在进入monitor模式后, 调用以下接口(airconfig.h)得到ssid及passwd
    * int airconfig_do_sync(const target_item_t *item, int channel, int channel_locked[MAX_CHANNEL_CARE], uint16_t *base);
    * int airconfig_get_info(int len, int base, ap_passport_t *account, const char *ssid, int len_ssid);
    * int airconfig_reset(void);
    */
    start_lelink_config();
    debug("start lelink config");
    return 1;
}

/*
* 功能: 查看是否得到wifi的配置
*
* 参数:
*      ptr: 保留
*      ptrLen: 保留
*
* 返回值:
*      1 - 得到配置, 其它表示失败
*/
int halDoConfiguring(void *ptr, int ptrLen) {
    
    bool ret= is_lelink_config_done();
    
    if(ret){
        debug("lelink config done");
    }
    
    return ret;
}

/*
* 功能: 请求wifi连接AP
*
* 参数:
*      ptr: 指向结构PrivateCfg 
*
* 返回值:
*      1 - 正在连接AP
*/
int halDoApConnect(void *ptr, int ptrLen) {
    
    PrivateCfg *cfg = (PrivateCfg*)ptr;
    
    char ap_ssid[32]={0};
    char ap_key[64]={0};
    
    if(cfg!=NULL){
        
        
        debug("ssid_len = %d, psk_len = %d",cfg->data.nwCfg.config.ssid_len,cfg->data.nwCfg.config.psk_len);
        debug("ssid = %s, psk = %s",cfg->data.nwCfg.config.ssid,cfg->data.nwCfg.config.psk);
        memcpy(ap_ssid,cfg->data.nwCfg.config.ssid,cfg->data.nwCfg.config.ssid_len);
        
        memcpy(ap_key,cfg->data.nwCfg.config.psk,strlen(cfg->data.nwCfg.config.psk));
        
    }else{
        
        ap_passport_t account = get_lelink_config_info();
        
        memcpy(ap_ssid,account.ssid,strlen(account.ssid));
        
        memcpy(ap_key,account.psk,strlen(account.psk));
        
    }
    
    debug("start wlan using ssid = %s, key = %s",ap_ssid,ap_key);
    
    network_InitTypeDef_adv_st wNetConfigAdv;
    
    memset(&wNetConfigAdv, 0x0, sizeof(network_InitTypeDef_adv_st));
    
    strcpy((char*)wNetConfigAdv.ap_info.ssid, ap_ssid);
    
    strcpy((char*)wNetConfigAdv.key, ap_key);
    
    wNetConfigAdv.key_len = strlen(ap_key);
    
    wNetConfigAdv.ap_info.security = SECURITY_TYPE_AUTO;
    
    wNetConfigAdv.ap_info.channel = 6; //Auto
    
    wNetConfigAdv.dhcpMode = DHCP_Client;
    
    wNetConfigAdv.wifi_retry_interval = 100;
    
    micoWlanStartAdv(&wNetConfigAdv);
    
    return 1;
}
extern IPStatusTypedef t11_network_info;
/*
* 功能: 查看是否成功连接上AP
*
* 参数:
*      ptr: 保留
*      ptrLen: 保留
*
* 返回值:
*      1 - 连接AP成功, 其它表示失败
*/
int halDoApConnecting(void *ptr, int ptrLen) {
    // 连dhcpOK，返回1
    
    if(t11_network_info.ip[0]=='\0'){
        
        return 0;
    }
    debug("dhcp success, ip = %s",t11_network_info.ip);
    return 1;
}


int halStopConfig(void){
    bool ret= is_lelink_config_done();    
    stop_lelink_config();
    
    debug("stop lelink config");

    if(ret) {
	  	mico_Context_t *inContext = getGlobalContext();
	    inContext->flashContentInRam.micoSystemConfig.configured = allConfigured;
	    MICOUpdateConfiguration(inContext);
	    MicoSystemReboot();
    }

    return 0;
}



/*
* 功能: 启动wifi的AP功能模式
*
* 参数: 
*      ssid: AP的名称
*      wpa2_passphrase: AP的wpa2密码
*
* 返回值:
*      0 - 成功启动, 其它表示失败
*/
int halSoftApStart(const char *ssid, char *wpa2_passphrase, uint8_t *aesKey, int aesKeyLen) {
    debug("start soft ap");
    network_InitTypeDef_st wNetConfig;
    
    char *ap_ssid = ssid;
    char *ap_key = wpa2_passphrase;
    
    memset(&wNetConfig, 0x0, sizeof(network_InitTypeDef_st));
    
    strcpy((char*)wNetConfig.wifi_ssid, ap_ssid);
    strcpy((char*)wNetConfig.wifi_key, ap_key);
    
    wNetConfig.wifi_mode = Soft_AP;
    wNetConfig.dhcpMode = DHCP_Server;
    wNetConfig.wifi_retry_interval = 100;
    strcpy((char*)wNetConfig.local_ip_addr, "192.168.10.1");
    strcpy((char*)wNetConfig.net_mask, "255.255.255.0");
    strcpy((char*)wNetConfig.dnsServer_ip_addr, "192.168.10.1");
    
    debug("ssid:%s  key:%s", wNetConfig.wifi_ssid, wNetConfig.wifi_key);
    micoWlanStart(&wNetConfig);
    debug("start softap");
    return 0;
}


/*
* 功能: 停止wifi的AP功能模式
* 
* 返回值:
*      0 - 成功启动, 其它表示失败
*/
int halSoftApStop(int success) {
    if(success)
    {
	  	mico_Context_t *inContext = getGlobalContext();
	    inContext->flashContentInRam.micoSystemConfig.configured = allConfigured;
	    MICOUpdateConfiguration(inContext);
	    MicoSystemReboot();
    }
    return 0;
}
