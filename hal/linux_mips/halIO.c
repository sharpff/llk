#include "halHeader.h"
#include <errno.h>

void *halUartOpen(uartHandler_t* handler) {
    return (void *)0xffffffff;
}

int halUartClose(uartHandler_t* handler) {
    return 0;
}

int halUartRead(uartHandler_t* handler, uint8_t *buf, uint32_t len) {
    return 0;
}

int halUartWrite(uartHandler_t* handler, const uint8_t *buf, uint32_t len) {
    return len;
}

void *halGPIOInit(void) {
    return NULL;
}

int halGPIOClose(gpioHandler_t* handler) {
    return 0;
}

int halGPIOOpen(gpioHandler_t* handler) {
    return -1;
}

int halGPIORead(gpioHandler_t* handler, int *val) {
    return 0;
}

int halGPIOWrite(gpioHandler_t* handler, const int val) {
    return 0;
}

void halPWMWrite(pwmHandler_t *handler, uint32_t percent) {
    return;
}

void halPWMRead(pwmHandler_t *handler, uint32_t *percent) {
    return;
}

void halPWMSetFrequency(pwmHandler_t *handler) {
    return;
}

int halPWMClose(pwmHandler_t *handler) {
    return 0;
}

int halPWMOpen(pwmHandler_t *handler) {
    return 0;
}

void* halPWMInit(int clock) {
    return 0xffffffff;
}

void halCommonInit(commonManager_t* dev) {
    return;
}

int halFlashInit(void)
{
    return 0;
}

int halFlashDeinit(void)
{
    return 0;
}

static int ginMinSize = 0x1000; // 4k
#define GET_PAGE_SIZE(l) \
    ((((l - 1) / ginMinSize) + 1)*ginMinSize)
void *halFlashOpen(void)
{
    return (void *)0xffffffff;
}

int halFlashClose(void *dev)
{
    return 0;
}
int halFlashErase(void *dev, uint32_t startAddr, uint32_t size){
    return 0;
}

int halFlashWrite(void *dev, const uint8_t *data, int len, uint32_t startAddr){
    int fd, ret, append;
    char fileName[64] = {0};
    sprintf(fileName, "./0x%x.bin", startAddr);
    fd = open(fileName, O_WRONLY | O_CREAT);
    if (0 >= fd) {
        printf("WRITE FAILED [%d]\r\n", errno);
        return fd;
    }

    ret = write(fd, data, len);
    append = GET_PAGE_SIZE(len) - len;
    if (0 < append) {
        uint8_t byte = 0xFF;
        while (append--) {
            ret += write(fd, &byte, 1);
        }
    }
    close(fd);
    printf("WRITE OK [%s] size[0x%x]\r\n", fileName, ret);
    return ret;
}

int halFlashRead(void *dev, uint8_t *data, int len, uint32_t startAddr){
    int fd, ret;
    char fileName[64] = {0};
    sprintf(fileName, "./0x%x.bin", startAddr);
    fd = open(fileName, O_RDONLY);
    if (0 >= fd) {
        // printf("errno [%d]", errno);
        return fd;
    }
    ret = read(fd, data, len);
    close(fd);
    // printf("READ OK [%s]", fileName);
    return ret;
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
    printf("halPipeWrite(%d):%s\n", len, data);
    if(len > 20) { // {"ctrl":{"action":1}}
        sStatePwr = (sStatePwr + 1) % 3;
        sStatePercent = (sStatePwr == 1) ? 0 : 100;
    }
    return len;
}

void halPrint(const char *log) {
    printf(log);
}