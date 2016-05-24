## Lelink SDK 使用说明

Lelink SDK for Linux 

Version: 0.1

Author: feiguoyou@le.com

Copyright © 2004-2016 乐视网（letv.com）All rights reserved.


## 1. 前言

本文档适用于Lelink在Linux上的开发移植, 同时需要配合[Lelink开发包](./lelink.linux_arm.sdk)使用。

## 2. Lelink开发包目录结构说明

```bash
    lelink.linux_arm.sdk
    ├── app
    │   └── linux_arm
    │       ├── 0x1c2000.bin
    │       ├── 0x1c3000.bin
    │       ├── 0x1c8000.bin
    │       ├── debug.mak
    │       ├── test.lua
    │       ├── genProfile.sh
    │       ├── main.c
    │       └── Makefile
    ├── hal
    │   └── linux_arm
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
    │       └── Debug-linux_arm
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
```

* ### [app/linux_arm/](app/linux_arm/)

    包含Lelink启动的使用示例及编译方法, 下面对该目录下的文档简要说明:

    - 0x1c2000.bin 认证文件, 由乐视统一分配, 固定不变。

    - 0x1c3000.bin 配置文件, 主要用于网络配置. 在每次配置完成，该文件分被程序更新。

    - 0x1c8000.bin 固件脚本文件配置，由test.lua配合生成。

    - test.lua 固件脚本，使用lua编写，根据不同产品由厂家提供。

    - genProfile.sh 生成程序运行时的配置文件(0x1c2000.bin、0x1c3000.bin、0x1c8000.bin)

    - main.c Lelink 的启动示例。

    - Makefile 编译参考。


* ### [hal/linux_arm/](hal/linux_arm/)

    该目录中包含多个C文件，在做Lelink移植时主要工作即是实现这些C中的函数。


* ### [lib/lelink/Debug-linux_arm/](lib/lelink/Debug-linux_arm/)

    包括两个库，在编译时使用，使用的编译器是arm-none-linux-gnueabi-gcc，版本信息如下:
```bash
    Target: arm-none-linux-gnueabi
    Thread model: posix
    gcc version 4.6.1 (Sourcery CodeBench Lite 2011.09-70) 
```

* ### [sw/](sw/)

    Lelink的头文件，其中主要使用(airconfig.h)的接口，完成wifi的配置。

* ### [tool/](tool/)

    开发及测试工具


## 3. 示例使用说明

目前的开发包中，app/linux_arm下。使用make编译，会生成ARM平台下的Debug/linux可执行文件。

示例程序是一个模拟实现窗帘的功能(只是简单示例，仅供参考)。


## 4. 移植工作

移植包括两大部分,分别是系统移植和WIFI配置移植。

### 4.1 系统移植

由于示例中已经按照linux标准实现大部分功能，所以主要工作是完成hal/linux_arm/halIO.c中的以下接口函数:
```c
    void *halPipeOpen(char *name);
    int halPipeClose(void *dev);
    int halPipeRead(void *dev, uint8_t *buf, uint32_t len);
    int halPipeWrite(void *dev, const uint8_t *buf, uint32_t len);
```
以上函数的实现说请参考文件中的注释部分。


### 4.2 WIFI配置移植

要实现该部分功能，要求wifi芯片可以进入monitor模式。另外，要求wifi芯片具有模拟AP的能力。

主要实现的功能代码放在hal/linux_arm/halAirConfig.c
```c
    int halDoConfig(void *ptr, int ptrLen);
    int halDoConfiguring(void *ptr, int ptrLen);
    int halDoApConnect(void *ptr, int ptrLen);
    int halDoApConnecting(void *ptr, int ptrLen);
    int halSoftApStart(char *ssid, char *wpa2_passphrase);
    int halSoftApStop(void);
```
以上函数的实现说请参考文件中的注释部分。

### 4.3 其它

hal/linux_arm/下的其它功能可以进一步根据系统平台优化。

另外在hal/linux_arm/halNetwork.c中，有两个函数得到网络的ip及mac。现在默认是得到"wlan0"的，如不是该名称请相应更改。
```c
    int halGetSelfAddr(char *ip, int size, int *port);
    int halGetMac(uint8_t *mac, int len);
```

## 5. 工具使用

### 5.1、app/linux_arm/genProfile.sh
    
该工具会生成(0x1c2000.bin、0x1c3000.bin、0x1c8000.bin), 当固件脚本(test.lua)修改后，需要执行该工具。

### 5.2 tool/linux

App模拟器, 要求在请在app/linux_arm/目录下启动

启动参数EXEC [target uuid] [target ip] [ctrl1] [ctrl2]

>  例如: ./tool/linux 10000100101000010007C80E77ABCD50 192.168.3.104 \{\"ctrl\":\{\"pwr\":1}\} \{\"ctrl\":\{\"pwr\":0}\}

>  说明：

>  ctrl1, ctrl2作为控制命令，根据模拟器的控制指令交替发出。

>  控制命令：

>  1 -  设备发现

>  2 -  设备局域网控制

>  3 -  设备状态获取

>  6 -  设备信息上报

>  7 -  设备远程控制，远程控制要求现将设备信息上报。

### 5.3 tool/luaTest

固件脚本测试工具，以对app/linux_arm/test.lua的测试(关于该文件的编写规则，请参考示例文件及注释)。

> 例如: too/luaTest app/linux_arm/test.lua "s1GetCvtType"


