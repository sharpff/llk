#include "halHeader.h"
#include <errno.h>
#include "debug.h"
#include "hal_uart.h"
#include "hal_gpio.h"
#include "hal_flash.h"
#include "hal_pwm.h"
#include "wifi_api.h"
#include "io.h"

static volatile uint32_t receive_notice = 0;
static volatile uint32_t send_notice = 0;
static uint8_t rx_vfifo_buffer[512]; // __attribute__ ((section(".noncached_zidata")));
static uint8_t tx_vfifo_buffer[512]; // __attribute__ ((section(".noncached_zidata")));

static void halUartIrq(hal_uart_callback_event_t status, void *user_data) {
   if(status == HAL_UART_EVENT_READY_TO_WRITE)
       send_notice = 1;
   else if(status == HAL_UART_EVENT_READY_TO_READ)
       receive_notice = 1;
}

void *halUartOpen(void* dev) {
    hal_uart_config_t uart_config;
    hal_uart_dma_config_t dma_config;
	hal_uart_status_t status = HAL_UART_STATUS_OK;
    uartHand_t* uartHand = (uartHand_t*)dev;
    APPLOG("halUartOpen id[%d] baud[%d], dataBits[%d], stopBits[%d], parity[%d], flowCtrl[%d]", uartHand->id,
        uartHand->baud, uartHand->dataBits, uartHand->stopBits, uartHand->parity, uartHand->flowCtrl);

    switch(uartHand->baud) {
        case 110:           /**< Defines UART baudrate as 110 bps */
		  uart_config.baudrate = HAL_UART_BAUDRATE_110;
		  break;
		  
	    case 300:           /**< Defines UART baudrate as 300 bps */
		  uart_config.baudrate = HAL_UART_BAUDRATE_300;
		  break;
		  
	    case 1200:          /**< Defines UART baudrate as 1200 bps */
			uart_config.baudrate = HAL_UART_BAUDRATE_1200;
			break;
		
	    case 2400:          /**< Defines UART baudrate as 2400 bps */
           uart_config.baudrate = HAL_UART_BAUDRATE_2400;
		   break;
        
	    case 4800:         /**< Defines UART baudrate as 4800 bps */
		   uart_config.baudrate = HAL_UART_BAUDRATE_4800;
		   break;
			
	    case 9600:         /**< Defines UART baudrate as 9600 bps */
	       uart_config.baudrate = HAL_UART_BAUDRATE_9600;
		   break;
	    
	    case 19200:         /**< Defines UART baudrate as 19200 bps */
	       uart_config.baudrate = HAL_UART_BAUDRATE_19200;
		   break;
		  	
	    case 38400:         /**< Defines UART baudrate as 38400 bps */
		   uart_config.baudrate = HAL_UART_BAUDRATE_38400;
           break;
		   
	    case 57600:        /**< Defines UART baudrate as 57600 bps */
		   uart_config.baudrate = HAL_UART_BAUDRATE_57600;
		   break;
			 	
	    case 115200:        /**< Defines UART baudrate as 115200 bps */
		   uart_config.baudrate = HAL_UART_BAUDRATE_115200;
		   break;
		   
	    case 230400:      /**< Defines UART baudrate as 230400 bps */
		   uart_config.baudrate = HAL_UART_BAUDRATE_230400;
		   break;
		   
	    case 460800:      /**< Defines UART baudrate as 460800 bps */
			uart_config.baudrate = HAL_UART_BAUDRATE_460800;
			break;
			
	    case 921600:      /**< Defines UART baudrate as 921600 bps */
			uart_config.baudrate = HAL_UART_BAUDRATE_921600;
			break;
			
	    default:                /**< Defines maximum enum value of UART baudrate **/
            APPLOG("The param value of <baud> is not supported.");
			break;
    }
	
    switch(uartHand->dataBits) {
  		case 5:
  			uart_config.word_length = HAL_UART_WORD_LENGTH_5;
			break;

	    case 6:
			uart_config.word_length = HAL_UART_WORD_LENGTH_6;
			break;

		case 7:
			uart_config.word_length = HAL_UART_WORD_LENGTH_7;
			break;

  		case 8:
			uart_config.word_length = HAL_UART_WORD_LENGTH_8;
			break;

		default:
			APPLOG("The param value of <dataBits> is not supported.");
			break;
    }
    
    switch(uartHand->stopBits) {
        case 1:
		  uart_config.stop_bit = HAL_UART_STOP_BIT_1;
		  break;
		case 2:
		  uart_config.stop_bit = HAL_UART_STOP_BIT_2;
		  break;
	    default:
			APPLOG("The param value of <stopBits> is not supported.");
			break;
    }
	
    uart_config.parity = uartHand->parity;
    status = hal_uart_init(uartHand->id, &uart_config);
	if(status != HAL_UART_STATUS_OK) {
        APPLOG("open uart[%d] failed", uartHand->id);
        return NULL;
    }	

    dma_config.receive_vfifo_alert_size = 50;
    dma_config.receive_vfifo_buffer = rx_vfifo_buffer;
    dma_config.receive_vfifo_buffer_size = 512;
    dma_config.receive_vfifo_threshold_size = 128;
    dma_config.send_vfifo_buffer = tx_vfifo_buffer;
    dma_config.send_vfifo_buffer_size = 512;
    dma_config.send_vfifo_threshold_size = 51;
    hal_uart_set_dma(uartHand->id, &dma_config);
    hal_uart_register_callback(uartHand->id, halUartIrq, NULL);
    return (void *)0xABCDFFFF;
}

int halUartClose(void *dev) {
	hal_uart_status_t status = HAL_UART_STATUS_OK;
    uartHand_t* uartHand = (uartHand_t*)dev;
	status = hal_uart_deinit(uartHand->id);
	if(status != HAL_UART_STATUS_OK)
		return -1;
	else
		return 0;
}

int halUartRead(void *dev, uint8_t *buf, uint32_t len) {
    uint32_t rcv_cnt, size = 0;
    uartHand_t* uartHand = (uartHand_t*)dev;
    memset(buf, 0, len);
#if 0
    while(1){
       uint32_t timeout = 0;
       rcv_cnt = hal_uart_receive_dma(uartHand->id, buf+size, len);
       if(rcv_cnt == 0)
          break;
       len -= rcv_cnt;
       size += rcv_cnt;
       while(!receive_notice)
        {
            if(timeout++ > 500)
                break;
        }
       receive_notice = 0;
    }
#else
     while(receive_notice){
       rcv_cnt = hal_uart_receive_dma(uartHand->id, buf+size, len);
       if(rcv_cnt == 0)
          break;
       len -= rcv_cnt;
       size += rcv_cnt;
    }
    receive_notice = 0;
#endif
#if 0
    if(size > 0)
    {
        uint32_t i;
        APPLOG("halUartRead [%d]",size);
        for(i=0;i<size;i++)
        {
            APPLOG("0x%02x ", buf[i]);
        }
        APPLOG("halUartRead data end");
    }
#endif
    return size;
}

int halUartWrite(void *dev, const uint8_t *buf, uint32_t len) {
    uint32_t snd_cnt, size = 0;
    uartHand_t* uartHand = (uartHand_t*)dev;
    //APPLOG("halUartWrite id[%d] len[%d]", uartHand->id, len);
    while(1){
        uint32_t timeout = 0;
        snd_cnt = hal_uart_send_dma(uartHand->id, buf+size, len);
        len -= snd_cnt;
        size += snd_cnt;
        if(len == 0)
           break;
        while(!send_notice) {
            if(timeout++ > 500)
                break;
        }
        send_notice = 0;
   }
   //APPLOG("halUartWrite id[%d]", uartHand->id);
   return size;
}

void *halGPIOInit(void) {
    //not to do anything;
    return 0xABCEFFFF;
}

int halGPIOClose(void *dev) {
    gpioHand_t* gpioHand = (gpioHand_t*)dev;
    //APPLOG("halGPIOClose id[%d]", gpioHand->id);
	hal_gpio_deinit(gpioHand->id);
    return 0;
}

int halGPIOOpen(void *dev) {
	hal_gpio_status_t ret;
    gpioHand_t* gpioHand = (gpioHand_t*)dev;
    APPLOG("halGPIOOpen b[%d]", gpioHand->id);
    ret = hal_gpio_init(gpioHand->id);
    if (HAL_GPIO_STATUS_OK != ret) {
        APPLOG("hal_gpio_init failed");
        hal_gpio_deinit(gpioHand->id);
        return -1;
    }

    switch(gpioHand->dir) {
        case GPIO_DIR_INPUT:
            hal_gpio_set_direction(gpioHand->id, HAL_GPIO_DIRECTION_INPUT);
            break;
        case GPIO_DIR_OUTPUT:
            hal_gpio_set_direction(gpioHand->id, HAL_GPIO_DIRECTION_OUTPUT);
            break;
        default:
            return -1;
    }

    switch(gpioHand->mode) {
    	case GPIO_MODE_DEFAULT:
    	    break;
        case GPIO_MODE_PULLUP:
            hal_gpio_pull_up(gpioHand->id);
            break;
        case GPIO_MODE_PULLDOWN:
            hal_gpio_pull_down(gpioHand->id);
            break;
        case GPIO_MODE_NOPULL:
            hal_gpio_disable_pull(gpioHand->id);
            break;
        default:
            break;
    }

    return gpioHand->id;
}

int halGPIORead(void *dev, int *val) {
	hal_gpio_data_t gpio_data;
    gpioHand_t* gpioHand = (gpioHand_t*)dev;
    hal_gpio_get_input(gpioHand->id, &gpio_data);
    *val = (int)gpio_data;
    //APPLOG("halGPIORead e[%d]", *val);
    return 0;
}

int halGPIOWrite(void *dev, const int val) {
    gpioHand_t* gpioHand = (gpioHand_t*)dev;
    //APPLOG("halGPIOWrite b[%d]", val);
	hal_gpio_set_output(gpioHand->id, (hal_gpio_data_t)val);
    return 0;
}

void halPWMWrite(void *dev, uint32_t percent) {
    uint32_t duty_cycle = 0;
    pwmHand_t* pwmHand = (pwmHand_t*)dev;
    //APPLOG("halPWMSetDuty id[%d] percent[%d] [%d] [%d]", pwmHand->id, percent, pwmHand->duty, pwmHand->reserved);
    pwmHand->percent = percent;
    duty_cycle = (pwmHand->reserved * percent) / pwmHand->duty;
    hal_pwm_set_duty_cycle(pwmHand->id, duty_cycle);
}

void halPWMRead(void *dev, uint32_t *percent) {
    pwmHand_t* pwmHand = (pwmHand_t*)dev;
    //APPLOG("halPWMRead id[%d]", pwmHand->id);
    *percent = pwmHand->percent;
}

void halPWMSetFrequency(void *dev) {
    pwmHand_t* pwmHand = (pwmHand_t*)dev;
    hal_pwm_set_frequency(pwmHand->id, pwmHand->frequency, &pwmHand->reserved);
    APPLOG("halPWMSetFrequency id[%d] frequency[%d] [%d]", pwmHand->id, pwmHand->frequency, pwmHand->reserved);
}

int halPWMClose(void* dev) {
    pwmHand_t* pwmHand = (pwmHand_t*)dev;
    //APPLOG("halPWMClose id[%d]", pwmHand->id);
    hal_pwm_stop(pwmHand->id);
    return 0;
}

int halPWMOpen(void *dev) {
    pwmHand_t* pwmHand = (pwmHand_t*)dev;
    hal_pwm_start(pwmHand->id);
    return pwmHand->id;
}

void* halPWMInit(int clock) {
    APPLOG("halPWMInit clock[%d]", clock);
    hal_pwm_init(clock); // HAL_PWM_CLOCK_2MHZ
    return 0xABCFFFFF;
}

//Flash;
int halFlashInit(void) {
    hal_flash_status_t ret;
    
    ret = hal_flash_init();
    if (ret < HAL_FLASH_STATUS_OK)
      	return ret;
	else
		return 0;
}

int halFlashDeinit(void) {
    hal_flash_status_t ret;
    
    ret = hal_flash_deinit();
	if (ret < HAL_FLASH_STATUS_OK)
      	return ret;
	else
    	return 0;
}

// static int ginMinSize = 0x1000; // 4k
//#define GET_PAGE_SIZE(l) \
//    ((((l - 1) / ginMinSize) + 1)*ginMinSize)
    
void *halFlashOpen(void) {
    //not need to do anything;
    return (void *)0xffffffff;
}

int halFlashClose(void *dev) {
    //not need to do anything;
    return 0;
}

int halFlashErase(void *dev, uint32_t startAddr, uint32_t size) {
     int ret = 0;

     int sector_to_erase = startAddr >> 12;//4K address align;
     int sector_count = size >> 12;//4K*n;
    
     while(sector_count--) {        
           APPLOG("[Lelink]: ==>starting erase 0x%x",sector_to_erase << 12);
	       ret = hal_flash_erase(sector_to_erase << 12, HAL_FLASH_BLOCK_4K);
	       if(ret != HAL_FLASH_STATUS_OK) {
	                //LOG_ERROR("erase flash sector error, %s\r\n",__func__);
	                APPLOG("[Lelink]: erase flash sector error, %s",__func__);
	                return ret;
	       }
		   //LOG_DEBUG("==>ending erase 0x%x",sector_to_erase << 12);
		   APPLOG("[Lelink]: ==>ending erase 0x%x",sector_to_erase << 12);
	       sector_to_erase ++;
     }
     return ret;
}

int halFlashWrite(void *dev, const uint8_t *data, int len, uint32_t startAddr, int32_t offsetToBegin) {
    hal_flash_status_t ret;

	ret = hal_flash_write(startAddr + offsetToBegin, data, len);

	if (ret < HAL_FLASH_STATUS_OK)
      	return ret;
	else
		return 0;
}

int halFlashRead(void *dev, uint8_t *data, int len, uint32_t startAddr, int32_t offsetToBegin) {
    hal_flash_status_t ret;

	ret = hal_flash_read(startAddr + offsetToBegin, data, len);

	if (ret < HAL_FLASH_STATUS_OK)
      	return ret;
	else
		return 0;
}

void halCommonInit(void* dev) {
    int i = 0;
    commonManager_t* commonManager = (commonManager_t*)dev;
    for(i=0; i<commonManager->num; i++) {
        hal_gpio_init(commonManager->table[i].id);
        hal_pinmux_set_function(commonManager->table[i].id, commonManager->table[i].mux );
        hal_gpio_deinit(commonManager->table[i].id);
    }
}

/*
 * 测试数据，表示窗帘的状态: 开/关、百分比
 */
static int sStatePwr = 1; // 1, 开; 2, 关
static int sStatePercent = 0; // %

/*
 * 功能: 打开Pipe通讯接口
 *
 * 参数: 
 *      name: Pipe名称(固件脚本中配置)
 *
 * 返回值:
 *      Pipe的handle
 */
void *halPipeOpen(char *name) {
    return (void *)0xffffffff;
}

/*
 * 功能: 关闭Pipe通讯接口 
 *
 * 参数:
 *      dev: Pipe的handle
 *      
 * 返回值:
 *      0 - 成功, 否则失败
 */
int halPipeClose(void *dev) {
    return 0;
}

/*
 *  功能: 读取设备数据。包括2类数据(与固件脚本保持一致, 固件中判断数据类型)
 *      1, 设备状态数据
 *      2, wifi重置数据
 *  
 *  参数:
 *      dev: Pipe的handle
 *      buf: 数据缓冲区
 *      len: buf缓冲区的长度
 *
 *  返回值:
 *      读取到的数据
 */
int halPipeRead(void *dev, uint8_t *buf, uint32_t len) {
    char data[128];

    if(sStatePwr == 1 && sStatePercent < 100) {
        sStatePercent += 10;
    }
    if(sStatePwr == 2 && sStatePercent > 0) {
        sStatePercent -= 10;
    }
    /*snprintf(data, sizeof(data), "{\"type\":2, \"data\":{\"pwr\":%d,\"percentage\":%d}}", sStatePwr, sStatePercent);*/
    snprintf(data, sizeof(data), "{\"type\":2, \"data\":{\"percentage\":%d}}", sStatePercent);
    memcpy(buf, data, sizeof(data));
    return sizeof(data);
}

/*
 *  功能: 写设备数据。包括5类数据(与固件脚本保持一致, 固件中规定各类型对应的数据)
 *      1, 查询设备状态
 *      2, 设备进入配置状态
 *      3, 设备进入连接AP状态
 *      4, 已经连接到AP，可以本地控制
 *      5, 已经正常连到云服务，可远程控制
 *  
 *  参数:
 *      dev: Pipe的handle
 *      buf: 写入数据
 *      len: buf长度
 *
 *  返回值:
 *      写入数据长度
 *
 *  示例数据:
 *      1, {"DataType":1}
 *      2, {"ctrl":{"action":1}}
 */
int halPipeWrite(void *dev, const uint8_t *buf, uint32_t len) {
    char data[128] = {0};

    memcpy(data, buf, len);
    APPLOG("halPipeWrite(%d):%s", (int)len, data);
    if(len > 20) { // {"ctrl":{"action":1}}
        sStatePwr = (sStatePwr + 1) % 3;
        sStatePercent = (sStatePwr == 1) ? 0 : 100;
    }
    return len;
}

int halGetMac(uint8_t *mac, int len) {
    uint8_t status = 0;
    
    status = wifi_config_get_mac_address(WIFI_PORT_STA, mac);
    
    if(status >= 0) {
        APPLOG("[Lelink]: wifi_config_get_mac_address(): (%02x:%02x:%02x:%02x:%02x:%02x),status = %d ",
                   mac[0], mac[1], mac[2],mac[3], mac[4], mac[5],status);
        return 0;
    }
    else {
        APPLOG("[Lelink]: wifi_config_get_mac_address(): get failed.");
        return -1;
    }
}

void halPrint(const char *log) {
    printf(log);
}
