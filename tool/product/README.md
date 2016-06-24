## Lelink SDK 使用说明

Version: 0.2

Author: feiguoyou@le.com

Copyright © 2004-2016 乐视网（letv.com）All rights reserved.


## 1. 前言

本文档适用于Lelink在PLATFORM上的开发移植, 同时需要配合**Lelink**开发包使用。


## 2. Lelink开发包目录结构说明

    lelink.PLATFORM.sdk
    ├── app
    │   └── PLATFORM
    │       ├── 0x1c2000.bin
    │       ├── 0x1c3000.bin
    │       ├── 0x1c8000.bin
    │       ├── debug.mak
    │       ├── test.lua
    │       ├── genProfile.sh
    │       ├── main.c
    │       └── Makefile
    ├── hal
    │   └── PLATFORM
    │       ├── halAES.c
    │       ├── halAirConfig.c
    │       ├── halCallback.c
    │       ├── halConvertor.c
    │       ├── halHeader.h
    │       ├── halIO.c
    │       ├── halMD5.c
    │       ├── halNetwork.c
    │       ├── halOS.c
    │       ├── halOTA.c
    │       └── halRSA.c
    ├── lib
    │   └── lelink
    │       └── Debug-PLATFORM
    │           ├──liblelink.a
    │           └──libsengine.a
    ├── sw
    │   ├── airconfig.h
    │   ├── io.h
    │   ├── leconfig.h
    │   ├── mbedtls-2.2.0_crypto
    │   │   └── include
    │   ├── ota.h
    │   ├── protocol.h
    │   └── state.h
    └── tool
        ├── lelinkTool.py
        ├── linux
        ├── linuxAirConfig
        └── luaTest


### app/PLATFORM/

包含Lelink启动的使用示例及编译方法, 下面对该目录下的文档简要说明:

- 0x1c2000.bin 认证文件, 由乐视统一分配, 固定不变。

- 0x1c3000.bin 配置文件, 主要用于网络配置. 在每次配置完成，该文件分被程序更新。

- 0x1c8000.bin 固件脚本文件配置，由test.lua配合生成。

- test.lua 固件脚本，使用lua编写，根据不同产品由厂家提供。

- genProfile.sh 生成程序运行时的配置文件(0x1c2000.bin、0x1c3000.bin、0x1c8000.bin)

- main.c Lelink 的启动示例。

- Makefile 编译参考。


### hal/PLATFORM/

该目录中包含多个C文件，在做Lelink移植时主要工作即是实现这些C中的函数。


### lib/lelink/Debug-PLATFORM/

包括两个库(liblelink.a libsengine.a)，在编译时使用。


### sw/

Lelink的头文件，其中主要使用(airconfig.h)的接口，完成wifi的配置。


### tool/

开发及测试工具


## 3. 示例使用说明

目前的开发包中，app/linux\_PLATFORM下。使用make编译，会生成PLATFORM平台下的Debug/linux可执行文件。

示例程序是一个模拟实现窗帘的功能(只是简单示例，仅供参考)。


## 4. 移植工作

移植包括两大部分,分别是系统移植和WIFI配置移植。


### 4.1 系统移植

由于示例中已经按照linux标准实现大部分功能，所以主要工作是完成hal/linux\_PLATFORM/halIO.c中的以下接口函数:

```c
    void *halPipeOpen(char *name);
    int halPipeClose(void *dev);
    int halPipeRead(void *dev, uint8_t *buf, uint32_t len);
    int halPipeWrite(void *dev, const uint8_t *buf, uint32_t len);
```

以上函数的实现说请参考文件中的注释部分。


### 4.2 WIFI配置移植

要实现该部分功能，要求wifi芯片可以进入monitor模式。另外，要求wifi芯片具有模拟AP的能力。

主要实现的功能代码放在hal/linux\_PLATFORM/halAirConfig.c

```c
    int halDoConfig(void *ptr, int ptrLen);
    int halDoConfiguring(void *ptr, int ptrLen);
    int halDoApConnect(void *ptr, int ptrLen);
    int halDoApConnecting(void *ptr, int ptrLen);
    int halSoftApStart(char *ssid, char *wpa2_passphrase);
    int halSoftApStop(void);
```

*注: 以上函数的实现说请参考文件中的注释部分, 另外halDoConfig()调用后，需要另起线程处理monitor数据的监控及处理开启softap接收数据， 其中用到的函数的详细说明如下(其它未说明的函数使用请按照示例代码使用):*


* airconfig_do_sync()
```c
/*
 * 功能: 收集monitor模式下收到的802.11的数据信息
 *
 * 参数:
 *      item: target_item_t数据，即收到的一帧802.11数据的信息，详细参考数据结构target_item_t
 *      channel: 指示item的信息是来源于哪个802.11信道
 *      channel_locked: 配置过程中，用于收集信道的。当锁定完成后，该数组的结果中表示锁定了哪几个信道
 *      base: 用于返回基准值，用于下一步函数 airconfig_get_info()
 *
 * 返回值:
 *      1 - 表示锁定成功，下一步使用 airconfig_get_info() 进一步收集数据信息
 *
 * 注: 
 *      1, 调用该函数前，确保wifi数据信息是在monitor模式下收集到的
 *      2, 要求wifi收集数据的信道要在1-14之前，以约30ms的间隔切换
 *       
 */
int airconfig_do_sync(const target_item_t *item, int channel, 
                       int channel_locked[MAX_CHANNEL_CARE], uint16_t *base);
```

* airconfig_get_info()
```c
/*
 * 功能: 在成功使用 airconfig_do_sync() 锁定信道的前提下，进一步收集802.11数据信息
 *
 * 参数: 
 *      len: 收到802.11信息的DATA数据长度
 *      base: 由airconfig_do_sync()得到的基准值
 *      account: 在成功配置时，返回AP的信息，详细参考数据结构ap_passport_t 
 *      ssid: 如果通过知道该802.11的DATA数据来源的SSID, 则通过该参数传入，否则传入NULL
 *      len_ssid: ssid的字节长度
 *
 * 返回值:
 *      1 - 表示成功得到AP的信息，并将信息保存在account中
 *      2 - 注意处理超时，如果在该数据多次调用约2分钟后仍未成功，则应该回退到 airconfig_do_sync() 再次处理
 *
 * 注: 在调用该函数前，要求wifi信道的切换在锁定的几个间切换

 */
int airconfig_get_info(int len, int base, ap_passport_t *account, 
                        const char *ssid, int len_ssid);
```

* airconfig_reset()
```c
/*
 * 功能: 重置配置
 *
 * 返回值:
 *      0 - 重置成功，目前不会有失败的情况
 *
 * 注: 在结束一次配置后使用
 *
 */
int airconfig_reset(void);
```

* softApStarted(void);
```c
/*
 * 功能: 在softap模式下，接收AP的配置信息
 *
 * 返回值: 
 *      0 表示成功接收到AP的信息
 *
 * 注: 该函数返回条件是 1, 接收到AP信息; 2, 通过其它配置完成了AP配置
 *
 */
int softApStarted(void);
```


### 4.3 其它

hal/PLATFORM/下的其它功能可以进一步根据系统平台优化。

另外在hal/PLATFORM/halNetwork.c中，有两个函数得到网络的ip及mac。现在默认是得到"wlan0"的，如不是该名称请相应更改。
```c
    int halGetSelfAddr(char *ip, int size, int *port);
    int halGetMac(uint8_t *mac, int len);
```


## 5. 工具使用


### 5.1、app/PLATFORM/genProfile.sh
    
该工具会生成(0x1c2000.bin、0x1c3000.bin、0x1c8000.bin), 当固件脚本(test.lua)修改后，需要执行该工具。


### 5.2 tool/linux

App模拟器, 要求在请在app/PLATFORM/目录下启动

启动参数EXEC [target uuid] [target ip] [ctrl1] [ctrl2]

例如: ./tool/linux 10000100101000010007C80E77ABCD50 192.168.3.104 \{\"ctrl\":\{\"pwr\":1}\} \{\"ctrl\":\{\"pwr\":0}\}

说明：

ctrl1, ctrl2作为控制命令，根据模拟器的控制指令交替发出。

控制命令：

1 -  设备发现

2 -  设备局域网控制

3 -  设备状态获取

6 -  设备信息上报

7 -  设备远程控制，远程控制要求现将设备信息上报。

### 5.3 tool/luaTest

固件脚本测试工具，以对app/PLATFORM/test.lua的测试(关于该文件的编写规则，请参考示例文件及注释)。

例如: too/luaTest app/PLATFORM/test.lua "s1GetCvtType"


