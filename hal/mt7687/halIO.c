#include "halHeader.h"
#include <errno.h>

#include "hal_uart.h"
#include "hal_gpio.h"
#include "hal_flash.h"
#include "wifi_api.h"

void *halUartOpen(int baud, int dataBits, int stopBits, int parity, int flowCtrl) 
{
    hal_uart_config_t uart_config;
	hal_uart_status_t status = HAL_UART_STATUS_OK;
		
    /* Set Pinmux to UART */
    hal_pinmux_set_function(HAL_GPIO_0, HAL_GPIO_0_UART1_RTS_CM4);
    hal_pinmux_set_function(HAL_GPIO_1, HAL_GPIO_1_UART1_CTS_CM4);
    hal_pinmux_set_function(HAL_GPIO_2, HAL_GPIO_2_UART1_RX_CM4);
    hal_pinmux_set_function(HAL_GPIO_3, HAL_GPIO_3_UART1_TX_CM4);

    switch(baud)
    {
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
            printf("The param value of <baud> is not supported. \n");
			break;
    }
	
    //uart_config.baudrate = HAL_UART_BAUDRATE_115200;
    
    //uart_config.word_length = HAL_UART_WORD_LENGTH_8;
    switch(dataBits)
    {
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
			printf("The param value of <dataBits> is not supported. \n");
			break;
    }
    
	
    //uart_config.stop_bit = HAL_UART_STOP_BIT_1;
    switch(stopBits)
    {
        case 1:
		  uart_config.stop_bit = HAL_UART_STOP_BIT_1;
		  break;

		case 2:
		  uart_config.stop_bit = HAL_UART_STOP_BIT_2;
		  break;

	    default:
			printf("The param value of <stopBits> is not supported. \n");
			break;
    }
	
   
    //uart_config.parity = HAL_UART_PARITY_NONE;
    uart_config.parity = parity;
	
    status = hal_uart_init(HAL_UART_0, &uart_config);
	if(status != HAL_UART_STATUS_OK)
		return NULL;
	else
		return (void *)0xABCDFFFF;
}

int halUartClose(void *dev) 
{
	hal_uart_status_t status = HAL_UART_STATUS_OK;
	
	status = hal_uart_deinit(HAL_UART_0);
	if(status != HAL_UART_STATUS_OK)
		return -1;
	else
		return 0;
}

int halUartRead(void *dev, uint8_t *buf, uint32_t len) 
{
    int size;
	
	size = (int)hal_uart_receive_polling(HAL_UART_0, buf, len);
	
    return size;
}

int halUartWrite(void *dev, const uint8_t *buf, uint32_t len) 
{
    int size;
	
	size = (int)hal_uart_send_polling(HAL_UART_0, (const uint8_t *)buf, len);

    return size;
}

void *halGPIOInit(void) 
{
    //not to do anything;
    return NULL;
}

int halGPIOClose(void *dev) {
    return 0;
}

int halGPIOOpen(int8_t id, int8_t dir, int8_t mode) 
{
    return -1;
}

int halGPIORead(void *dev, int gpioId, int *val) {
    return 0;
}

int halGPIOWrite(void *dev, int gpioId, const int val) {
    return 0;
}

//Flash;
int halFlashInit(void)
{
    hal_flash_status_t ret;
    
    ret = hal_flash_init();
    if (ret < HAL_FLASH_STATUS_OK)
      	return ret;
	else
		return 0;
}

int halFlashDeinit(void)
{
    hal_flash_status_t ret;
    
    ret = hal_flash_deinit();
	if (ret < HAL_FLASH_STATUS_OK)
      	return ret;
	else
    	return 0;
}

// static int ginMinSize = 0x1000; // 4k
#define GET_PAGE_SIZE(l) \
    ((((l - 1) / ginMinSize) + 1)*ginMinSize)
    
void *halFlashOpen(void)
{
    //not need to do anything;
    return (void *)0xffffffff;
}

int halFlashClose(void *dev)
{
    //not need to do anything;
    return 0;
}

int halFlashErase(void *dev, uint32_t startAddr, uint32_t size)
{
     int ret = 0;

     int sector_to_erase = startAddr >> 12;//4K address align;
     int sector_count = size >> 12;//4K*n;
    
     while(sector_count--)
     {        
           //LOG_DEBUG("==>starting erase 0x%x\r\n",sector_to_erase << 12);
           printf("[Lelink]: ==>starting erase 0x%x\r\n",sector_to_erase << 12);
	       ret = hal_flash_erase(sector_to_erase << 12, HAL_FLASH_BLOCK_4K);
	       if(ret != HAL_FLASH_STATUS_OK)
	       {
	                //LOG_ERROR("erase flash sector error, %s\r\n",__func__);
	                printf("[Lelink]: erase flash sector error, %s\r\n",__func__);
	                return ret;
	       }
		   //LOG_DEBUG("==>ending erase 0x%x\r\n",sector_to_erase << 12);
		   //printf("[Lelink]: ==>ending erase 0x%x\r\n",sector_to_erase << 12);
	       sector_to_erase ++;
     }

     return ret;
}

int halFlashWrite(void *dev, const uint8_t *data, int len, uint32_t startAddr)
{
    hal_flash_status_t ret;

	ret = hal_flash_write(startAddr, data, len);

	if (ret < HAL_FLASH_STATUS_OK)
      	return ret;
	else
		return 0;
    //return ret;
}

int halFlashRead(void *dev, uint8_t *data, int len, uint32_t startAddr)
{
    hal_flash_status_t ret;

	ret = hal_flash_read(startAddr, data, len);

	if (ret < HAL_FLASH_STATUS_OK)
      	return ret;
	else
		return 0;
    //return ret;
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
    printf("halPipeWrite(%d):%s\n", (int)len, data);
    if(len > 20) { // {"ctrl":{"action":1}}
        sStatePwr = (sStatePwr + 1) % 3;
        sStatePercent = (sStatePwr == 1) ? 0 : 100;
    }
    return len;
}

int halGetMac(uint8_t *mac, int len)
{
    uint8_t addr[6] = {0};
    uint8_t status = 0;
    
    status = wifi_config_get_mac_address(WIFI_PORT_STA, addr);
    
    if(status >= 0)
    {
        printf("[Lelink]: wifi_config_get_mac_address(): (%02x:%02x:%02x:%02x:%02x:%02x),status = %d \n",
                   addr[0], addr[1], addr[2],addr[3], addr[4], addr[5],status);

        return 0;
    }
    else
    {
        printf("[Lelink]: wifi_config_get_mac_address(): get failed. \n");

        return -1;
    }
}

void halPrint(const char *log) {
    printf(log);
}