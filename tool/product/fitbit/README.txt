zigbee-cluster-library-spcification.pdf
Cluster ID:
1) 0x0000: 基本类
2) 0x0006：ON/OFF类
3) 0x0008：Level类
4) 0x0001：电源管理类
5) 0x0406：PIR，人体识别类
6) 0x0402：温度测量类
7) 0x0405：湿度测量类
8) 0x0201: 温控器类

0x0009: alarm
0x0502: WD, warning device
0x0501: ACE, Ancillary Control Equip
0x0500: IAS Zone, Intruder Alarm Systems

Device ID: 
0x0402: CIE Control and Indicating Equip (*****includes clusters 0x0502, 0x0500, 0x)

c(command): MGR 05-join, 06-leave
aM(address mode): 00-invalid, 01-group, 02-short, 03-mac 

*CLASSIC_JOIN_PERMIT_JOIN:
REQ: 
hd l  cT cT    addr  e  all   pL aM       c  t     cs
FE 0D 29 00 0B 02 00 0B FF FF 06 02 00 00 05 3C 01 1A 
RSP:
   l        ret
FE 01 69 00 00 68 

mask:
0x01,Send aRequest? 0x00 - leave silence. 0x01 - leave with Indicating
0x00,ReJoin?
0x01,Remove Child?

hd l  cT cT    addr           pL |-mask-| c  |-------MAC-----------| cs
*mgr leave:
FE 13 29 00 0B 00 00 0C FF FF 0C 01 00 00 06 00 00 00 00 00 00 00 00 36

*mgr leave rsp:
FE 01 69 00 00 68

*新设备入网：
hd l  cT cT sAddr       |-------MAC-----------|    cs
dev announce
FE 0D 45 C1 67 5F 67 5F 52 98 C3 0C 00 4B 12 00 80 55



*设备leave ind：
hd l  cT cT sAddr |-------MAC-----------| |-mask-| cs
FE 0D 45 C9 DE A9 74 09 E1 7E 33 76 AF 60 00 00 00 9E

# switcher:
# AA 00 11 82 04 01 45 01 01 00 3B 9A 01 1A 04 77 DF F8 00 00 C8 
*debug split:
01004D000BD7317100158D0000F4D4E78E03
01004D000B5ABE4A60AF76337EE1097480030100920006D402317101010203 

atmel:
E6CF 60AF76337EE10974
mi:
FAE7 00158D0000F3A0CE
light:
057d C06FA000E44CE36F

                     i8                      i16      


*恢复Gateway到出厂设置：
01 02 10 11 02 10 02 10 11 03
01 00    11 00    00    11 03
RSP mass, too many info
01 80 06    00    01    86 01    03
01 80 02 16 02 10 02 11 86 02 11 03 



*sensor类：
   msgT  len   cs sq short e  clu      attrE t  v
btn pressed:
01 81 02 00 0B 62 04 B4 4C 01 00 06 00 00 00 10 01 03 
01 81 02    00    0B 62 04    B4 4C 01    00    06    00    00    00    10 01    03 
01 81 02 12 02 10 0B 62 02 14 B4 4C 02 11 02 10 02 16 02 10 02 10 02 10 10 02 11 03
0181021202100B620214B44C02110210021602100210021010021103
heart:
01 81 02 00 0B AA 91 6E C7 01 04 06 00 00 00 18 01 03 
01 81 02    00    0B AA 91 6E C7 01    04    06    00    00    00    18 01    03 
01 81 02 12 02 10 0B AA 91 6E C7 02 11 02 14 02 16 02 10 02 10 02 10 18 02 11 03
0181021202100BAA916EC702110214021602100210021018021103

01 81 02 00 0B 2D 02 85 30 05 00 06 00 00 00 10 01 03
01 81 02 12 02 10 0B 2D 02 12 85 30 02 15 02 10 02 16 02 10 02 10 02 10 10 02 11 03
01 81 02    00    0B 2D 02    85 30 05    00    06    00    00    00    10 01    03
0181021202100B2D0212853002150210021602100210021010021103

*ctrl类：                
sI: seqID, need to be diff for every ctrl.
v: value, 01-open, 00-close, 02-toggle
REQ:
LIGHT - ONOFF:
hd l  cT cT    addr  e  onOff l  aM    sI v  cs
FE 0B 29 00 0B 0B 5f 01 06 00 04 02 11 01 01 6D
LIGHT - LEVEL:
c: command move to level - 00, move - 01
l: level value, 0 ~ FF
t: time, 100ms * n, philip used n = 4.
hd l  cT cT    addr  e  level l  aM    sI c  lv t     cs
fe 0e 29 00 0b 0b 5f 01 08 00 07 02 11 01 00 01 00 00 65 
LIGHT - HUE:
c: HUE & SAT
hd l  cT cT    addr  e  level l  aM    sI c  H  S  t     cs
FE 0F 29 00 0B 0B 5f 01 00 03 08 02 01 DD 06 80 80 00 00 XX

RSP:
FE 01 69 00 00 68




*active request:
*active response:
hd l  cT cT sAddr ret      c  v  cs
FE 07 45 85 67 5F 00 67 5F 01 01 C7

*simple descriptor request:
*simple descriptor response:
iC: in cluster
oC: out cluster
iV: in cluster val
oV: out cluster val
hd l  cT cT sAddr ret      l  e  pid   did   v  iC iV    oC oV    oV    oV    oV    oV    oV    cs
FE 1C 45 84 67 5F 00 67 5F 16 01 04 01 02 04 00 01 00 00 06 00 00 06 00 09 00 01 00 02 05 00 05 C2


*node descriptor request:
*node descriptor response:
lT: logical type, 00-co, 01-router, 02-end device
hd l  cT cT sAddr ret      lT       manu                          cs
FE 12 45 82 67 5F 00 67 5F 02 40 80 00 00 50 A0 00 00 00 A0 00 00 47

                     i8                      i16      

std2pri:
profile id
did ---------------- cluster // 脚本中转换
mac ---------------- address // 引擎转换 getAddrFromMac()
endpoint ----------- // ignore


pri2std:
profile id
did ---------------- cluster // 脚本中转换
mac ---------------- getMacFromAddr() // 如果是设备状态通知的情况
endpoint

{"sDev":{"pid":"","ept":[],"mac":"7409E17E3376AF60"}}
*endpoint response:
{"sDev":{"pid":"","ept":[1,2,3],"mac":"7409E17E3376AF60"}}
*descriptor response:
{"sDevStatus":{"btn":1},"sDev":{"info":[{"ept":1,"did":"0101","clu":["0000","0004","0004","0004","0004"]}],"man":"1234","pid":"0104","mac":"7409E17E3376AF60"}}


"{\"status\":{\"pwr\":1,\"mark\":2,\"actions\":3},\"cloud\":1,\"uuid\":\"d05bca44feb34aeca2dd111111111111\",\"ip\":\"192.168.1.100\",\"ver\":\"3-1.0.0.git.004-161-g7b08f3c.1-1-1.1\",\"ssid\":\"\",\"lock\":0,\"uuids\":[],\"sDevStatus\":{\"btn\":1},\"sDev\":{\"des\":[{\"ept\":1,\"did\":\"0101\",\"clu\":[\"0000\",\"0004\",\"0004\",\"0004\",\"0004\"]}],\"man\":\"1234\",\"pid\":\"0104\",\"mac\":\"7409E17E3376AF60\"}}"
{
    "status":{
        "pwr":1,
        "mark":2,
        "actions":3
    },
    "cloud":1,
    "uuid":"d05bca44feb34aeca2dd111111111111",
    "ip":"192.168.1.100",
    "ver":"3-1.0.0.git.004-161-g7b08f3c.1-1-1.1",
    "ssid":"",
    "lock":0,
    "uuids":[

    ],
    "sDevStatus":{
        "btn":1
    },
    "sDev":{
        "des":[
            {
                "ept":1,
                "did":"0101",
                "clu":[
                    "0000",
                    "0004",
                    "0004",
                    "0004",
                    "0004"
                ]
            }
        ],
        "man":"1234",
        "pid":"0104",
        "mac":"7409E17E3376AF60"
    }
}


"{\"sDevCtrl\":{\"pwr\":1},\"sDev\":{\"ept\":1,\"did\":\"0101\",\"clu\":\"0006\",\"mac\":\"7409E17E3376AF60\"}}"
{
    "sDevCtrl":{
        "pwr":1
    },
    "sDev":
    {
        "ept":1,
        "did":"0101",
        "clu":"0006",
        "mac":"7409E17E3376AF60"
    }
}


hello:



*新设备入网：
   msgT  len   cs short |-------MAC-----------| 
01 00 4D 00 0B 8D 38 CF 00 12 4B 00 07 6A 88 00 80 03
01 00 4D 00 0B 8D 38 CF 00 12 4B 00 07 6A 88 00 80 03

01 87 01 00 02 84 00 00 03
01 87 01 00 02 84 00 00 03
*simple descriptor response:
   msgT  len   cs       short l  e  pid   did   *v cn     
01 80 43 00 19 70 43 00 38 CF 10 08 01 04 03 02 00 03 00 00 00 03 04 02 01 00 00 00 03 04 02 03
*node descriptor response:
   msgT  len   cs sq s  short man
01 80 42 00 11 F2 44 00 38 CF 00 00 00 A0 00 A0 00 00 00 80 50 40 02 03
*sensor类：
   msgT  len   cs sq short e  clu      attrE t  v
01 81 02 00 0C 64 01 38 CF 08 04 02 00 00 00 29 0A 30 03
01 81 02 00 0C D3 03 38 CF 08 04 05 00 00 00 21 10 90 03

01 81 02 00 0B 51 04 E2 2D 05 00 06 00 00 00 10 01 03
01 81 02 00 0B 50 05 E2 2D 05 00 06 00 00 00 10 01 03

IAS
01 84 01 00 0D 2B 0C 01 05 00 02 9F 36 00 01 00 00 00 01 03
01 84 01 00 0D 2B 0D 01 05 00 02 9F 36 00 00 00 00 00 01 03



void zllSocGetState(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode)
{     
    uint8_t cmd[] = {
      0xFE,                                                                                      
      13,   /*RPC payload Len */          
      0x29, /*MT_RPC_CMD_AREQ + MT_RPC_SYS_APP */          
      0x00, /*MT_APP_MSG  */          
      0x0B, /*Application Endpoint */          
      0x00,//(dstAddr & 0x00ff),
      0x00,//(dstAddr & 0xff00) >> 8,
      0x00,//endpoint, /*Dst EP */          
      (ZCL_CLUSTER_ID_GEN_ON_OFF & 0x00ff),
      (ZCL_CLUSTER_ID_GEN_ON_OFF & 0xff00) >> 8,
      0x06, //Data Len
      0x00,//addrMode, 
      0x00, //0x00 ZCL frame control field.  not specific to a cluster (i.e. a SCL founadation command)
      0x00,//transSeqNumber++,
      ZCL_CMD_READ,
      (ATTRID_ON_OFF & 0x00ff),
      (ATTRID_ON_OFF & 0xff00) >> 8,
      0x00       //FCS - fill in later
    };
      cmd[5]=(dstAddr & 0x00ff);
      cmd[6]=(dstAddr & 0xff00) >> 8;
      cmd[7]=endpoint; /*Dst EP */          
          cmd[11]=addrMode;
    cmd[13]=transSeqNumber++;

    calcFcs(cmd, sizeof(cmd));
    
    UARTwrite((const char *)cmd, sizeof(cmd));
} 
void zllSocGetLevel(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode)
{
    uint8_t cmd[] = {
      0xFE,                                                                                      
      13,   /*RPC payload Len */          
      0x29, /*MT_RPC_CMD_AREQ + MT_RPC_SYS_APP */          
      0x00, /*MT_APP_MSG  */          
      0x0B, /*Application Endpoint */          
      0x00,//(dstAddr & 0x00ff),
      0x00,//(dstAddr & 0xff00) >> 8,
      0x00,//endpoint, /*Dst EP */          
      (ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL & 0x00ff),
      (ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL & 0xff00) >> 8,
      0x06, //Data Len
      0x00,//addrMode, 
      0x00, //0x00 ZCL frame control field.  not specific to a cluster (i.e. a SCL founadation command)
      0x00,//transSeqNumber++,
      ZCL_CMD_READ,
      (ATTRID_LEVEL_CURRENT_LEVEL & 0x00ff),
      (ATTRID_LEVEL_CURRENT_LEVEL & 0xff00) >> 8,
      0x00       //FCS - fill in later
    };
      cmd[5]=(dstAddr & 0x00ff);
      cmd[6]=(dstAddr & 0xff00) >> 8;
      cmd[7]=endpoint; /*Dst EP */          
          cmd[11]=addrMode;
    cmd[13]=transSeqNumber++;
      
    calcFcs(cmd, sizeof(cmd));
    
    UARTwrite((const char *)cmd, sizeof(cmd));
} 
