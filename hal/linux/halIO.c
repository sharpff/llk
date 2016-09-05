#include "halHeader.h"
#include <errno.h>
#include <termios.h>
#include "leconfig.h"

#define DEVICE_ID1    "/dev/ttyUSB0" 
// #define DEVICE_ID2    "/dev/ttyUSB1" 
static void setSpeed(int fd, int speed); 
static int setParity(int fd, int databits, int stopbits, int parity);

void *halUartOpen(uartHandler_t* handler) {
    int fd;
    void *tmp = NULL;
    char PARITY = '\0';
    struct termios options;
    fd = open(DEVICE_ID1, O_RDWR | O_NONBLOCK);
    if (0 > fd) {   
        APPLOGE("halUartOpen failed [%d]", errno);
        // exit(1);
        return NULL;
    }

    // tcgetattr(fd,&options);  //获取串口设备的相关属性
    // cfsetispeed(&options, B115200); //设置串口的输入波特率为115200
    // cfsetospeed(&options, B115200); //设置串口的输出波特率为115200
    // options.c_cflag &= ~PARENB;
    // options.c_cflag |= CSTOPB;
    // options.c_cflag &= ~CSIZE;
    // options.c_cflag |= CS8;
    // options.c_lflag &= (~ICANON);
    // options.c_lflag &= (~ECHO);
    // tcsetattr(fd,TCSANOW, &options); //设置串口设备的相关属性

    setSpeed(fd, handler->baud);
    if(handler->parity == 0) // None
        PARITY = 'N';
    else if(handler->parity == 1) // Odd
        PARITY = 'O';
    else // Even
        PARITY = 'E';
    if (!setParity(fd, handler->dataBits, handler->stopBits, PARITY))  {  
        APPLOGE("halUartOpen Set Parity Error");
        close(fd);
        return NULL;  
    }
    APPLOG("halUartOpen ok [%d]", fd);
    memcpy(&tmp, &fd, sizeof(fd));
    memcpy(&handler->handler, &fd, sizeof(fd));
    return tmp;
}

int halUartClose(uartHandler_t* handler) {
    int fd;
    memcpy(&fd, &handler->handler, sizeof(fd));
    close(fd);
    APPLOG("halUartClose ret");
    return 0;
}

int halUartRead(uartHandler_t* handler, uint8_t *buf, uint32_t len) {
    int ret = 0, tmpLen = 0, fd;
    memcpy(&fd, &handler->handler, sizeof(fd));
    ret = read(handler->handler, buf, len);
    if (0 < ret) {
        do {
            tmpLen += ret;
            APPLOG("snap [%d]", ret);
            ret = read(fd, buf + tmpLen, len - tmpLen);
        } while (0 < ret);
        int i = 0;
        APPLOG("halUartRead tmpLen [%d]", tmpLen);
        for (i = 0; i < tmpLen; i++) {
            APPPRINTF("%02x ", buf[i]);
        }
        APPPRINTF("\r\n");
    }
    return tmpLen;
}

int halUartWrite(uartHandler_t* handler, const uint8_t *buf, uint32_t len) {
    int fd, ret = len;
    memcpy(&fd, &handler->handler, sizeof(fd));
    ret = write(fd, buf, len);
    // APPLOG("halUartWrite ret [%d]", ret);
    return ret;
}

void *halGPIOInit(void) {
    return (void *)0xffffffff;
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
    return (void *)0xffffffff;
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

int halFlashWrite(void *dev, const uint8_t *data, int len, uint32_t startAddr, int32_t offsetToBegin){
    int fd, ret, append;
    char fileName[64] = {0};
    sprintf(fileName, "./0x%x.bin", startAddr);
    fd = open(fileName, O_WRONLY | O_CREAT);
    if (0 >= fd) {
        APPLOG("WRITE FAILED [%d]\r\n", errno);
        return fd;
    }
    lseek(fd, offsetToBegin, SEEK_SET);
    ret = write(fd, data, len);
    append = GET_PAGE_SIZE(len) - len;
    if (0 < append) {
        uint8_t byte = 0xFF;
        while (append--) {
            ret += write(fd, &byte, 1);
        }
    }
    close(fd);
    APPLOG("WRITE OK [%s] size[0x%x]\r\n", fileName, ret);
    return ret;
}

int halFlashRead(void *dev, uint8_t *data, int len, uint32_t startAddr, int32_t offsetToBegin){
    int fd, ret;
    char fileName[64] = {0};
    sprintf(fileName, "./0x%x.bin", startAddr);
    fd = open(fileName, O_RDONLY);
    if (0 >= fd) {
        // printf("errno [%d]", errno);
        // APPLOG("READ FAILED [%d]", errno);
        return fd;
    }
    lseek(fd, offsetToBegin, SEEK_SET);
    ret = read(fd, data, len);
    close(fd);
    // APPLOG("READ OK [%s]", fileName);
    return ret;
}

void *halPipeOpen(char *name) {
    return (void *)0xffffffff;
}

int halPipeClose(void *dev) {
    return 0;
}

int halPipeRead(void *dev, uint8_t *buf, uint32_t len) {
    return 0;
}

int halPipeWrite(void *dev, const uint8_t *buf, uint32_t len) {
    return len;
}

int halGetMac(uint8_t *mac, int len) {
    if (6 > len || NULL == mac) {
        return -1;
    }

    mac[0] = 0x11;
    mac[1] = 0x11;
    mac[2] = 0x11;
    mac[3] = 0x11;
    mac[4] = 0x11;
    mac[5] = 0x11;
    return 0;
}


////////////////////////////////////////////////////////////////////////////////  
/** 
*@brief  设置串口通信速率 
*@param  fd     类型 int  打开串口的文件句柄 
*@param  speed  类型 int  串口速度 
*@return  void 
*/  
int speed_arr[] = {B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300,  
                   B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300, };  
int name_arr[] = {115200, 38400, 19200, 9600, 4800, 2400, 1200,  300,   
                  115200, 38400, 19200, 9600, 4800, 2400, 1200,  300, };  
static void setSpeed(int fd, int speed){  
  int   i;   
  int   status;   
  struct termios   Opt;  
  tcgetattr(fd, &Opt);   
  for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++) {   
    if  (speed == name_arr[i]) {       
      tcflush(fd, TCIOFLUSH);       
      cfsetispeed(&Opt, speed_arr[i]);    
      cfsetospeed(&Opt, speed_arr[i]);     
      status = tcsetattr(fd, TCSANOW, &Opt);    
      if  (status != 0) {          
        perror("tcsetattr fd1");    
        return;       
      }      
      tcflush(fd,TCIOFLUSH);     
    }    
  }  
}  

////////////////////////////////////////////////////////////////////////////////  
/** 
*@brief   设置串口数据位，停止位和效验位 
*@param  fd     类型  int  打开的串口文件句柄 
*@param  databits 类型  int 数据位   取值 为 7 或者8 
*@param  stopbits 类型  int 停止位   取值为 1 或者2 
*@param  parity  类型  int  效验类型 取值为N,E,O,,S 
*/  
static int setParity(int fd, int databits, int stopbits, int parity)  
{   
    struct termios options;   
    if  ( tcgetattr( fd,&options)  !=  0) {   
        APPLOGE("SetupSerial 1");       
        return(0);    
    }  
    options.c_cflag &= ~CSIZE;   
    switch (databits) /*设置数据位数*/  
    {     
    case 7:       
        options.c_cflag |= CS7;   
        break;  
    case 8:       
        options.c_cflag |= CS8;  
        break;     
    default:      
        fprintf(stderr,"Unsupported data size\n"); return (0);    
    }  
    switch (parity)   
    {     
        case 'n':  
        case 'N':      
            options.c_cflag &= ~PARENB;   /* Clear parity enable */  
            options.c_iflag &= ~INPCK;     /* Enable parity checking */   
            break;    
        case 'o':     
        case 'O':       
            options.c_cflag |= (PARODD | PARENB); /* 设置为奇效验*/    
            options.c_iflag |= INPCK;             /* Disnable parity checking */   
            break;    
        case 'e':    
        case 'E':     
            options.c_cflag |= PARENB;     /* Enable parity */      
            options.c_cflag &= ~PARODD;   /* 转换为偶效验*/       
            options.c_iflag |= INPCK;       /* Disnable parity checking */  
            break;  
        case 'S':   
        case 's':  /*as no parity*/     
            options.c_cflag &= ~PARENB;  
            options.c_cflag &= ~CSTOPB;break;    
        default:     
            fprintf(stderr,"Unsupported parity\n");      
            return (0);    
        }    
    /* 设置停止位*/    
    switch (stopbits)  
    {     
        case 1:      
            options.c_cflag &= ~CSTOPB;    
            break;    
        case 2:      
            options.c_cflag |= CSTOPB;    
           break;  
        default:      
             fprintf(stderr,"Unsupported stop bits\n");    
             return (0);   
    }   
    /* Set input parity option */   
    if (parity != 'n')     
        options.c_iflag |= INPCK;   
    tcflush(fd,TCIFLUSH);  
    options.c_cc[VTIME] = 150; /* 设置超时15 seconds*/     
    options.c_cc[VMIN] = 0; /* Update the options and do it NOW */  
    if (tcsetattr(fd,TCSANOW,&options) != 0)     
    {   
        APPLOGE("SetupSerial 3");     
        return (0);    
    }   
    options.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);  /*Input*/  
    options.c_oflag  &= ~OPOST;   /*Output*/  
    return (1);    
}

void halPrint(const char *log) {
    printf(log);
}