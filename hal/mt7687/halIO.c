#include "halHeader.h"
#include <errno.h>
#include "timers.h"
#include "debug.h"
#include "hal_uart.h"
#include "hal_gpio.h"
#include "hal_flash.h"
#include "hal_pwm.h"
#include "hal_eint.h"
#include "wifi_api.h"
#include "io.h"

#define LED_BLINK_TIMEOUT (500/portTICK_PERIOD_MS)

static TimerHandle_t halPWMTimerHandler = NULL;
extern pwmManager_t ginPWMManager;

static TimerHandle_t halGPIOTimerHandler = NULL;
extern gpioManager_t ginGpioManager;

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

void *halUartOpen(uartHandler_t* handler) {
    hal_uart_config_t uart_config;
    hal_uart_dma_config_t dma_config;
	hal_uart_status_t status = HAL_UART_STATUS_OK;

    switch(handler->baud) {
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
	
    switch(handler->dataBits) {
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
    
    switch(handler->stopBits) {
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
	
    uart_config.parity = handler->parity;
    status = hal_uart_init(handler->id, &uart_config);
	if(status != HAL_UART_STATUS_OK) {
        APPLOG("open uart[%d] failed[%d]", handler->id, status);
        return NULL;
    }	

    dma_config.receive_vfifo_alert_size = 50;
    dma_config.receive_vfifo_buffer = rx_vfifo_buffer;
    dma_config.receive_vfifo_buffer_size = 512;
    dma_config.receive_vfifo_threshold_size = 128;
    dma_config.send_vfifo_buffer = tx_vfifo_buffer;
    dma_config.send_vfifo_buffer_size = 512;
    dma_config.send_vfifo_threshold_size = 51;
    hal_uart_set_dma(handler->id, &dma_config);
    hal_uart_register_callback(handler->id, halUartIrq, NULL);
    return (void *)handler;
}

int halUartClose(uartHandler_t* handler) {
	hal_uart_status_t status = HAL_UART_STATUS_OK;
	status = hal_uart_deinit(handler->id);
	if(status != HAL_UART_STATUS_OK)
		return -1;
	else
		return 0;
}

int halUartRead(uartHandler_t* handler, uint8_t *buf, uint32_t len) {
    uint32_t rcv_cnt, size = 0;
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
       rcv_cnt = hal_uart_receive_dma(handler->id, buf+size, len);
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

int halUartWrite(uartHandler_t* handler, const uint8_t *buf, uint32_t len) {
    uint32_t snd_cnt, size = 0;
    while(1){
        uint32_t timeout = 0;
        snd_cnt = hal_uart_send_dma(handler->id, buf+size, len);
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

    // if(size > 0)
    // {
    //     uint32_t i;
    //     APPPRINTF("halUartWrite [%d]: ", size);
    //     for(i=0;i<size;i++)
    //     {
    //         APPPRINTF("%02x ", buf[i]);
    //     }
    //     APPPRINTF("\r\n");
    // }

   return size;
}

void *halGPIOInit(void) {
    return 0xABCEFFFF;
}

int halGPIOClose(gpioHandler_t* handler) {
	hal_gpio_deinit(handler->id);
    return 0;
}

int halGPIOOpen(gpioHandler_t* handler) {
	hal_gpio_status_t ret;
    ret = hal_gpio_init(handler->id);
    if (HAL_GPIO_STATUS_OK != ret) {
        APPLOG("hal_gpio_init failed");
        hal_gpio_deinit(handler->id);
        return -1;
    }

    switch(handler->dir) {
        case GPIO_DIR_INPUT:
            hal_gpio_set_direction(handler->id, HAL_GPIO_DIRECTION_INPUT);
            break;
        case GPIO_DIR_OUTPUT:
            hal_gpio_set_direction(handler->id, HAL_GPIO_DIRECTION_OUTPUT);
            break;
        default:
            return -1;
    }

    switch(handler->mode) {
    	case GPIO_MODE_DEFAULT:
    	    break;
        case GPIO_MODE_PULLUP:
            hal_gpio_pull_up(handler->id);
            break;
        case GPIO_MODE_PULLDOWN:
            hal_gpio_pull_down(handler->id);
            break;
        case GPIO_MODE_NOPULL:
            hal_gpio_disable_pull(handler->id);
            break;
        default:
            break;
    }
    return handler->id;
}

int halGPIORead(gpioHandler_t* handler, int *val) {
    if (handler->gpiostate > 1) {
        *val = handler->gpiostate;
    } else {
        hal_gpio_data_t gpio_data;
        if (GPIO_DIR_INPUT == handler->dir) {
            hal_gpio_get_input(handler->id, &gpio_data);
        } else if (GPIO_DIR_OUTPUT == handler->dir) {
            hal_gpio_get_output(handler->id, &gpio_data);
        }
        *val = (int)gpio_data;
    }
    // APPLOG("halGPIORead dir[%d] id [%d] v[%d]", handler->dir, handler->id, *val);
    return 0;
}

static void halGPIOBlinkTimerCallback( TimerHandle_t tmr ) {
    gpioHandler_t *table = NULL;
    int i,j;
    for(i = 0, table = ginGpioManager.table; i < ginGpioManager.num; i++, table++) {
        if (table->gpiostate == 2) {
            if (table->reserved2%4) {
               hal_gpio_set_output(table->id, table->state);
            } else if (table->reserved2%2 == 0) {
                hal_gpio_set_output(table->id, !table->state);
            }
            table->reserved2++;
        } else if (table->gpiostate == 3) {
            if (table->reserved2%12 == 0) {
               hal_gpio_set_output(table->id, table->state);
            } else if (table->reserved2%6 == 0) {
                hal_gpio_set_output(table->id, !table->state);
            }
            table->reserved2++;
        }
    }
    if (halGPIOTimerHandler != NULL) {
        xTimerStart(halGPIOTimerHandler, 0);
    }
}

int halGPIOWrite(gpioHandler_t* handler, const int val) {
    // APPLOG("halGPIOWrite dir[%d] id [%d] v[%d]", handler->dir, handler->id, val);
    handler->gpiostate = val;
    if (val < 2) {
        hal_gpio_set_output(handler->id, (hal_gpio_data_t)val);
    } else {
        if (halGPIOTimerHandler == NULL) {
            halGPIOTimerHandler = xTimerCreate( "gpio_blink_timer",
                                           LED_BLINK_TIMEOUT,
                                           pdFALSE,
                                           NULL,
                                           halGPIOBlinkTimerCallback);
            if (halGPIOTimerHandler != NULL) {
                xTimerStart(halGPIOTimerHandler, 0);
            }
        }
    }
    return 0;
}

static void halPWMBlinkTimerCallback( TimerHandle_t tmr ) {
    pwmHandler_t *table = NULL;
    int i,j;
    //APPLOG("halPWMBlinkTimerCallback");
    for(i = 0, table = ginPWMManager.table; i < ginPWMManager.num; i++, table++) {
        if (table->percent == 0x8001) {
            if (table->reserved2%4) {
               hal_pwm_set_duty_cycle(table->id, table->duty);
            } else if (table->reserved2%2 == 0) {
                hal_pwm_set_duty_cycle(table->id, 0);
            }
            table->reserved2++;
        } else if (table->percent == 0x8002) {
            if (table->reserved2%12 == 0) {
               hal_pwm_set_duty_cycle(table->id, table->duty);
            } else if (table->reserved2%6 == 0) {
                hal_pwm_set_duty_cycle(table->id, 0);
            }
            table->reserved2++;
        } else if (table->percent == 0x8003) {
            if (table->reserved2%6 == 0) {
               for (j = 0; table->percent && j < table->duty ; j+=2) {
                    hal_pwm_set_duty_cycle(table->id, j);
                    halDelayms(5);
                }
                for (j = table->duty; table->percent && j > 0; j-=2) {
                    hal_pwm_set_duty_cycle(table->id, j);
                    halDelayms(5);
                }
            }
            table->reserved2++;
        }
    }
    if (halPWMTimerHandler != NULL) {
        xTimerStart(halPWMTimerHandler, 0);
    }
}

void halPWMWrite(pwmHandler_t *handler, uint32_t percent) {
    //APPLOG("halPWMBlinkTimerCallback percent[0x%x]", percent);
    if (percent < 0x8000) {
        uint32_t duty_cycle = 0;
        handler->reserved2 = 0;
        handler->percent = percent;
        duty_cycle = (handler->reserved1 * percent) / handler->duty;
        hal_pwm_set_duty_cycle(handler->id, duty_cycle);
    } else{
        handler->percent = percent;
        if (percent == 0x8001 || percent == 0x8002) {
            handler->reserved2++;
            hal_pwm_set_duty_cycle(handler->id, handler->duty);
        }
        handler->percent = percent;
    }
}

void halPWMRead(pwmHandler_t *handler, uint32_t *percent) {
    *percent = handler->percent;
}

void halPWMSetFrequency(pwmHandler_t *handler) {
    hal_pwm_set_frequency(handler->id, handler->frequency, &handler->reserved1);
}

int halPWMClose(pwmHandler_t *handler) {
    hal_pwm_stop(handler->id);
    return 0;
}

int halPWMOpen(pwmHandler_t *handler) {
    hal_pwm_start(handler->id);
    return handler->id;
}

void* halPWMInit(int clock) {
    hal_pwm_init(clock);
    if (halPWMTimerHandler == NULL) {
        halPWMTimerHandler = xTimerCreate( "pwm_blink_timer",
                                       LED_BLINK_TIMEOUT,
                                       pdFALSE,
                                       NULL,
                                       halPWMBlinkTimerCallback);
        if (halPWMTimerHandler != NULL) {
            xTimerStart(halPWMTimerHandler, 0);
        }
    }
    return 0xABCFFFFF;
}

static void halEINTIrqHandler(void *data) {
    eintHandler_t *handler = (eintHandler_t *)data;
    //APPLOG("halEINTIrqHandler type[%d]", handler->type);
    if(handler) {
        if(handler->type > 0) {
            hal_gpio_data_t gpio_data;
            hal_gpio_get_input(handler->gid, &gpio_data);
            //APPLOG("halEINTIrqHandler state[%d] [%d]", gpio_data, handler->state);
            if (gpio_data == handler->state) {
                handler->count++;
                if(handler->timeout && handler->count == 1) {
                    handler->timeStamp = xTaskGetTickCount();
                }
                if (handler->longPressStart) {
                    if(xTaskGetTickCount() - handler->longPressStart > 2*1000/portTICK_PERIOD_MS) {
                        handler->longPress = 1;
                        handler->count = 0;
                        handler->timeStamp = 0;
                    }
                    handler->longPressStart = 0;
                }
            } else {
                //APPLOG("halEINTIrqHandler longPressStart");
                handler->longPressStart = xTaskGetTickCount();
            }
        } else {
            handler->count++;
            if(handler->timeout && handler->count == 1) {
                handler->timeStamp = xTaskGetTickCount();
            }
        }
    }
}

int halEINTRead(eintHandler_t* handler, int *val) {
    uint32_t curr;
    *val = 0;
    if(handler->type && handler->longPress) {
        handler->longPress = 0;
        *val = 0xFF;
        return 0;
    }
    if(handler->timeout) {
        if(handler->timeStamp > 0) {
            curr = xTaskGetTickCount();
            if(((curr - handler->timeStamp)/portTICK_PERIOD_MS) > handler->timeout) {
                *val = handler->count;
                handler->count = 0;
                handler->timeStamp = 0;
            }
        }
    } else {
        *val = handler->count;
        handler->count = 0;
    }
    return 0;
}

int halEINTClose(eintHandler_t *handler) {
    return 0;
}

int halEINTOpen(eintHandler_t *handler) {
    hal_eint_config_t eint_config;
    hal_gpio_init(handler->gid);
    hal_gpio_set_direction(handler->gid, HAL_GPIO_DIRECTION_INPUT);
    hal_gpio_disable_pull(handler->gid);
    eint_config.trigger_mode = handler->mode;
    eint_config.debounce_time = handler->debounce;
    hal_eint_init(handler->id, &eint_config);
    APPLOG("halEINTOpen id[%d] gid[%d] mode[%d] debounce[%d] timeout[%d]",
        handler->id, handler->gid, handler->mode, handler->debounce, handler->timeout);
    hal_eint_register_callback(handler->id, halEINTIrqHandler, handler);
    return handler->id;
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

    fota_port_isr_disable();
    while(sector_count--) {        
        APPLOG("[Lelink]: ==>starting erase 0x%x",sector_to_erase << 12);
        ret = hal_flash_erase(sector_to_erase << 12, HAL_FLASH_BLOCK_4K);
        if(ret != HAL_FLASH_STATUS_OK) {
        //LOG_ERROR("erase flash sector error, %s\r\n",__func__);
            APPLOG("[Lelink]: erase flash sector error, %s",__func__);
            fota_port_isr_enable();
            return ret;
        }
        //LOG_DEBUG("==>ending erase 0x%x",sector_to_erase << 12);
        APPLOG("[Lelink]: ==>ending erase 0x%x",sector_to_erase << 12);
        sector_to_erase ++;
    }
    fota_port_isr_enable();
    return ret;
}

int halFlashWrite(void *dev, const uint8_t *data, int len, uint32_t startAddr, int32_t offsetToBegin) {
    hal_flash_status_t ret;

    fota_port_isr_disable();
    ret = hal_flash_write(startAddr + offsetToBegin, data, len);
    fota_port_isr_enable();
    APPLOG("[Lelink]: ==>hal_flash_write ret[%d], startAddr[0x%x] offsetToBegin[0x%x] len[%d/0x%x] [0x%02x 0x%02x 0x%02x 0x%02x]", ret, startAddr, offsetToBegin, len, len, 
        data[0], data[1], data[2], data[3]);

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

void halCommonInit(commonManager_t* dev) {
    int i = 0;
    for(i=0; i<dev->num; i++) {
        hal_gpio_init(dev->table[i].id);
        hal_pinmux_set_function(dev->table[i].id, dev->table[i].mux );
        hal_gpio_deinit(dev->table[i].id);
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
        // mac[5] = 0x32;
        // wifi_profile_set_mac_address(WIFI_PORT_STA, mac);
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
