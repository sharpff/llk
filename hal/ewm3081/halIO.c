#include <stdio.h>
#include "halHeader.h"
#include <errno.h>
#include "MICO.h"
#include "MicoDrivers/MicoDriverFlash.h"
#include "MicoDrivers/MicoDriverUart.h"
#include "MicoDrivers/MicoDriverGpio.h"
#include "platform.h"
//#include "platform_peripheral.h"
#include "io.h"
#include "t11_debug.h"

////////////////////////////////////////////////////////////////////////////////////////////////
//3081只有一个用户串口，因此这里可以不管uart的dev参数
#define UART_BUFFER_LENGTH                  2048
static volatile ring_buffer_t uart_rx_buffer;
SDRAM_DATA_SECTION volatile uint8_t rx_data[UART_BUFFER_LENGTH];

void *halUartOpen(uartHandler_t* handler) {
    debug("open uart, baud = %d, dataBits = %d, stopBits = %d, parity = %d, flowCtrl = %d",handler->baud,handler->dataBits,handler->stopBits,handler->parity,handler->flowCtrl);
    mico_uart_config_t uart_config;
    uart_config.baud_rate = handler->baud;
    uart_config.data_width = handler->dataBits-5;    //DATA_WIDTH_5BIT == 0
    uart_config.parity = handler->parity;
    uart_config.stop_bits = handler->stopBits-1;     //STOP_BITS_1 == 0
    uart_config.flow_control = handler->flowCtrl;
    uart_config.flags = 0;
    
    ring_buffer_init((ring_buffer_t *) &uart_rx_buffer, (uint8_t *) rx_data,UART_BUFFER_LENGTH);
    MicoUartInitialize(UART_FOR_APP, &uart_config,(ring_buffer_t *) &uart_rx_buffer);
    return (void *) 0xffffffff;
}

int halUartClose(uartHandler_t* handler) {
    debug("close uart, dev = %p",handler);
    MicoUartFinalize(UART_FOR_APP);
    
    return 0;
}

int halUartRead(uartHandler_t* handler, uint8_t *buf, uint32_t len) {
//    
    //	OSStatus MicoUartRecv( mico_uart_t uart, void* data, uint32_t size, uint32_t timeout )
    uint32_t time_start = mico_get_time();
    uint32_t len_in_buf = MicoUartGetLengthInBuffer(UART_FOR_APP);
    uint32_t target_len = 0;
    if(len_in_buf>0){
        target_len = len_in_buf>len?len:len_in_buf;
        MicoUartRecv(UART_FOR_APP, buf, target_len, 0);
        //t11_print_mem(buf, target_len);
    }
    //debug("receive data len = %d",target_len);
    return target_len;
}

int halUartWrite(uartHandler_t* handler, const uint8_t *buf, uint32_t len) {
//    
    uint32_t time_start = mico_get_time();
//    t11_print_mem(buf,len);
    MicoUartSend(UART_FOR_APP, buf, len);
//    debug("写串口 dev = %p, buf = %p, len = %d, 用时 %d 毫秒",dev,buf,len,mico_get_time()-time_start);
    return len;
}

void test_uart(){
#if 0    
    char* data = "data send to uart";
    
    halUartOpen(9600,8,1,0,0);
    
    halUartWrite(NULL,data,strlen(data));
    
    halUartClose(NULL);
    
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////
#define __gpio_enable
#ifdef __gpio_enable
//维护id与gpio引脚的映射
static mico_gpio_t id_2_mico_pin(int id) {
    switch (id) {
    case 1:
        return MICO_GPIO_1;
    case 2:
        return MICO_GPIO_1;
    case 3:
        return MICO_GPIO_1;
    default:
        return MICO_GPIO_1;
    }
}

void *halGPIOInit(void) {
    
    return NULL;
}

int halGPIOClose(gpioHandler_t* handler) {
    
    return 0;
}

int halGPIOOpen(gpioHandler_t* handler) {
    
    debug("open GPIO, id = %d, dir = %d, mode = %d",handler->id,handler->dir,handler->mode);
    mico_gpio_config_t conf;
    
    int gpio_id = id_2_mico_pin(handler->id);
    //TODO:GPIO初始化需要再确定功能
    switch (handler->mode) {
    case GPIO_MODE_DEFAULT:
        conf = (handler->dir == GPIO_DIR_INPUT) ? INPUT_PULL_UP : OUTPUT_PUSH_PULL;
        break;
    case GPIO_MODE_PULLUP:
        conf = (handler->dir == GPIO_DIR_INPUT) ? INPUT_PULL_UP : OUTPUT_PUSH_PULL;
        break;
    case GPIO_MODE_PULLDOWN:
        conf = (handler->dir == GPIO_DIR_INPUT) ?
    INPUT_PULL_DOWN : OUTPUT_OPEN_DRAIN_NO_PULL;
    break;
    case GPIO_MODE_NOPULL:
        conf = (handler->dir == GPIO_DIR_INPUT) ?
    INPUT_PULL_DOWN : OUTPUT_OPEN_DRAIN_NO_PULL;
    break;
    case GPIO_MODE_RISTATE:
        conf = (handler->dir == GPIO_DIR_INPUT) ?
    INPUT_HIGH_IMPEDANCE : OUTPUT_OPEN_DRAIN_NO_PULL;
    break;
    default:
        conf = INPUT_HIGH_IMPEDANCE;
        return -1;
    }
    MicoGpioInitialize(gpio_id, conf);
    return gpio_id;
}

int halGPIORead(gpioHandler_t* handler, int *val) {
    
    *val= MicoGpioInputGet(handler->id)?GPIO_STATE_HIGH:GPIO_STATE_LOW;
    return 0;
}

int halGPIOWrite(gpioHandler_t* handler, const int val) {
    
    if(val==GPIO_STATE_HIGH){
        MicoGpioOutputHigh(handler->id);
    }else{
        MicoGpioOutputLow(handler->id);
    }
    return 0;
}
#endif

void halPWMWrite(pwmHandler_t *handler, uint32_t percent) {

}

void halPWMRead(pwmHandler_t *handler, uint32_t *percent) {

}

void halPWMSetFrequency(pwmHandler_t *handler) {

}

int halPWMClose(pwmHandler_t *handler) {
    return 0;
}

int halPWMOpen(pwmHandler_t *handler) {
    return 0;
}

void* halPWMInit(int clock) {
    return NULL;
}

int halEINTClose(eintHandler_t *handler) {
    return 0;
}

int halEINTOpen(eintHandler_t *handler) {
    return 0;
}

int halEINTRead(eintHandler_t* handler, int *val) {
    return 0;
}

void halCommonInit(commonManager_t* dev) {

}
////////////////////////////////////////////////////////////////////////////////////////////////

/**
* mico中的flash已经初始化
*/
int halFlashInit(void) {
    return 0;
}
/**
* mico中的flash操作不需要deinit
*/
int halFlashDeinit(void) {
    return 0;
}

/**
* 打开flash，mico不需要操作
*/
void *halFlashOpen(void) {
    
    return (void *) 0xffffffff;
}

/**
* 关闭flash，mico不需要操作
*/
int halFlashClose(void *dev) {
    
    return 0;
}

/**
 * 擦除flash，直接调用mico的底层函数即可
 */
int halFlashErase(void *dev, uint32_t startAddr, uint32_t size) {
    //debug("earse flash, dev = %p, startAddr = %p, size = %d",dev,startAddr,size);
    uint32_t _addr = startAddr;
    return MicoFlashErase(MICO_PARTITION_LELINK_PARAM, _addr, size);
}

/**
 * 写flash
 */
int halFlashWrite(void *dev, const uint8_t *data, int len, uint32_t startAddr, int32_t offsetToBegin) {
    //debug("write flash, dev = %p, data = %p, len = %d, startAddr = %d ",dev,data,len,startAddr + offsetToBegin);
    uint32_t _addr = startAddr + offsetToBegin;
    return MicoFlashWrite(MICO_PARTITION_LELINK_PARAM, &_addr, data, len);
}
/**
 * 读取flash
 */
int halFlashRead(void *dev, uint8_t *data, int len, uint32_t startAddr, int32_t offsetToBegin) {
    //debug("read flash, dev = %p, data = %p, len = %d, startAddr = %d %d", dev, data,len, startAddr, offsetToBegin);
    mico_logic_partition_t* ota_partition = MicoFlashGetInfo( MICO_PARTITION_OTA_TEMP );
    if(ota_partition->partition_start_addr == startAddr) {
      uint32_t _addr = offsetToBegin; 
      MicoFlashRead(MICO_PARTITION_OTA_TEMP, &_addr, data, len);
    } else {
      uint32_t _addr = startAddr + offsetToBegin; 
      MicoFlashRead(MICO_PARTITION_LELINK_PARAM, &_addr, data, len);
    }
    return len;
}
////////////////////////////////////////////////////////////////////////////////////////////////

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
    
    return (void *) 0xffffffff;
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
    
    return len;
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
    
    return len;
}
////////////////////////////////////////////////////////////////////////////////////////////////

void halPrint(const char *log) {   
    printf("[%d]",mico_get_time());
    printf(log);
}
