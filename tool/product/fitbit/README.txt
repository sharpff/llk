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
CIE includes clusters:
0x0502: WD, warning device
0x0501: ACE, Ancillary Control Equip
0x0500: IAS Zone, Intruder Alarm Systems

CIE(Control and Indicating Equip) Device ID: 
0x0402, 0x0403

c(command): MGR 05-join, 06-leave
aM(address mode): 00-invalid, 01-group, 02-short, 03-mac 
e(endpoint): endpoint FF is broadcast

reset to factory:
FE 0D 29 00 0B 02 00 0B FF FF 06 02 00 00 02 3C 00 00

GET co's mac
hd l  cT cT    addr  e  all   pL aM       c  t     cs
REQ:
FE 04 25 01 00 00 00 00 20 
BACK:
FE 01 65 01 00 65 
RSP:
hd l  cT cT r  |-------MAC-----------| addr     
FE 0D 45 81 00 A7 38 43 03 00 4B 12 00 00 00 8F 00 C0 

*CLASSIC_JOIN_PERMIT_JOIN:
REQ:
hd l  cT cT    addr  e  all   pL aM       c  t     cs
FE 0D 29 00 0B 02 00 0B FF FF 06 02 00 00 05 3C 01 1A 
FE0D29000B02000BFFFF06020000053C011A
RSP:
   l        ret
FE 01 69 00 00 68 

/*** Foundation Command IDs ***/
#define ZCL_CMD_READ                                    0x00
#define ZCL_CMD_READ_RSP                                0x01
#define ZCL_CMD_WRITE                                   0x02
#define ZCL_CMD_WRITE_UNDIVIDED                         0x03
#define ZCL_CMD_WRITE_RSP                               0x04
#define ZCL_CMD_WRITE_NO_RSP                            0x05
#define ZCL_CMD_CONFIG_REPORT                           0x06
#define ZCL_CMD_CONFIG_REPORT_RSP                       0x07
#define ZCL_CMD_READ_REPORT_CFG                         0x08
#define ZCL_CMD_READ_REPORT_CFG_RSP                     0x09
#define ZCL_CMD_REPORT                                  0x0a
#define ZCL_CMD_DEFAULT_RSP                             0x0b
#define ZCL_CMD_DISCOVER                                0x0c
#define ZCL_CMD_DISCOVER_RSP                            0x0d
X: 10 - need rsp, 11 - no need rsp(ctrl)
Y: ZCL CMD
hd l  cT cT    addr  e  CIE   l  aM X  sI Y           |-------MAC-----------| cs
FE 16 29 00 0B 3F A4 01 00 05 0F 02 10 01 02 10 00 F0 A7 38 43 03 00 4B 12 00 00 

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
FE0D45C1675F675F5298C30C004B12008055FE0D45C1675F675F5298C30C004B12008055
fe0d45c10b5f0b5fe7d4f400008d15008e58

*设备leave ind：
hd l  cT cT sAddr |-------MAC-----------| |-mask-| cs
FE 0D 45 C9 DE A9 74 09 E1 7E 33 76 AF 60 00 00 00 9E
FE0D45C9DEA97409E17E3376AF600000009E


*恢复Gateway到出厂设置：
01 02 10 11 02 10 02 10 11 03
01 00    11 00    00    11 03
RSP mass, too many info
01 80 06    00    01    86 01    03
01 80 02 16 02 10 02 11 86 02 11 03 


*active request:
*active response:
hd l  cT cT sAddr ret      c  v  cs
FE 07 45 85 67 5F 00 67 5F 01 01 C7
FE074585675F00675F0101C7

*simple descriptor request:
*simple descriptor response:
iC: in cluster
oC: out cluster
iV: in cluster val
oV: out cluster val
hd l  cT cT sAddr ret      l  e  pid   did      iC iV    oC oV    oV    oV    oV    oV    oV    cs
FE 1C 45 84 67 5F 00 67 5F 16 01 04 01 02 04 00 01 00 00 06 00 00 06 00 09 00 01 00 02 05 00 05 C2
FE1C4584675F00675F1601040102040001000006000006000900010002050005C2

*node descriptor request:
*node descriptor response:
lT: logical type, 00-co, 01-router, 02-end device
hd l  cT cT sAddr ret      lT       manu                          cs
FE 12 45 82 67 5F 00 67 5F 02 40 80 00 00 50 A0 00 00 00 A0 00 00 47
FE124582675F00675F024080000050A0000000A0000047


*sensor类：
/*** Data Types ***/
#define ZCL_DATATYPE_NO_DATA                            0x00
#define ZCL_DATATYPE_DATA8                              0x08
#define ZCL_DATATYPE_DATA16                             0x09
#define ZCL_DATATYPE_DATA24                             0x0a
#define ZCL_DATATYPE_DATA32                             0x0b
#define ZCL_DATATYPE_DATA40                             0x0c
#define ZCL_DATATYPE_DATA48                             0x0d
#define ZCL_DATATYPE_DATA56                             0x0e
#define ZCL_DATATYPE_DATA64                             0x0f
#define ZCL_DATATYPE_BOOLEAN                            0x10
#define ZCL_DATATYPE_BITMAP8                            0x18
#define ZCL_DATATYPE_BITMAP16                           0x19
#define ZCL_DATATYPE_BITMAP24                           0x1a
#define ZCL_DATATYPE_BITMAP32                           0x1b
#define ZCL_DATATYPE_BITMAP40                           0x1c
#define ZCL_DATATYPE_BITMAP48                           0x1d
#define ZCL_DATATYPE_BITMAP56                           0x1e
#define ZCL_DATATYPE_BITMAP64                           0x1f
#define ZCL_DATATYPE_UINT8                              0x20
#define ZCL_DATATYPE_UINT16                             0x21
#define ZCL_DATATYPE_UINT24                             0x22
#define ZCL_DATATYPE_UINT32                             0x23
#define ZCL_DATATYPE_UINT40                             0x24
#define ZCL_DATATYPE_UINT48                             0x25
#define ZCL_DATATYPE_UINT56                             0x26
#define ZCL_DATATYPE_UINT64                             0x27
#define ZCL_DATATYPE_INT8                               0x28
#define ZCL_DATATYPE_INT16                              0x29
#define ZCL_DATATYPE_INT24                              0x2a
#define ZCL_DATATYPE_INT32                              0x2b
#define ZCL_DATATYPE_INT40                              0x2c
#define ZCL_DATATYPE_INT48                              0x2d
#define ZCL_DATATYPE_INT56                              0x2e
#define ZCL_DATATYPE_INT64                              0x2f
#define ZCL_DATATYPE_ENUM8                              0x30
#define ZCL_DATATYPE_ENUM16                             0x31
#define ZCL_DATATYPE_SEMI_PREC                          0x38
#define ZCL_DATATYPE_SINGLE_PREC                        0x39
#define ZCL_DATATYPE_DOUBLE_PREC                        0x3a
#define ZCL_DATATYPE_OCTET_STR                          0x41
#define ZCL_DATATYPE_CHAR_STR                           0x42
#define ZCL_DATATYPE_LONG_OCTET_STR                     0x43
#define ZCL_DATATYPE_LONG_CHAR_STR                      0x44
#define ZCL_DATATYPE_ARRAY                              0x48
#define ZCL_DATATYPE_STRUCT                             0x4c
#define ZCL_DATATYPE_SET                                0x50
#define ZCL_DATATYPE_BAG                                0x51
#define ZCL_DATATYPE_TOD                                0xe0
#define ZCL_DATATYPE_DATE                               0xe1
#define ZCL_DATATYPE_UTC                                0xe2
#define ZCL_DATATYPE_CLUSTER_ID                         0xe8
#define ZCL_DATATYPE_ATTR_ID                            0xe9
#define ZCL_DATATYPE_BAC_OID                            0xea
#define ZCL_DATATYPE_IEEE_ADDR                          0xf0
#define ZCL_DATATYPE_128_BIT_SEC_KEY                    0xf1
#define ZCL_DATATYPE_UNKNOWN                            0xff
rC: report counts 
dT: data Type (refer to Data Types)
#define CMD_SS_IAS_ZONE_STATUS_ENROLL_REQUEST 0x81
#define CMD_SS_IAS_ZONE_STATUS_CHANGE_NOTIFICATION 0x80
#define CMD_SS_IAS_WD_NOTIFICATION 0x82
ss: Security & safe (attr id)
v: value(refer to 传感器相关数据释义-V1.02.pdf, high 4bit is no used. 0x0080 is low bettery)
hd l  cT cT sAddr e  clu   rC ss    dT v     
FE 0B 49 83 B0 16 01 00 05 01 80 00 21 01 00 C2 
FE0B4983B016010005018000210100C2

*ctrl类：                
sI: seqID, need to be diff for every ctrl.
v: value, 01-open, 00-close, 02-toggle
REQ:
LIGHT - ONOFF:
hd l  cT cT    addr  e  onOff l  aM    sI v  cs
FE 0B 29 00 0B 0B 5f 01 06 00 04 02 11 01 01 6D
FE 0B 29 00 0B EC 12 0B 06 00 04 02 11 01 01 CD  open HUE
FE 0B 29 00 0B EC 12 0B 06 00 04 02 11 01 00 CC  close HUE
LIGHT - LEVEL:
#define COMMAND_LEVEL_MOVE_TO_LEVEL                       0x00
#define COMMAND_LEVEL_MOVE                                0x01
#define COMMAND_LEVEL_STEP                                0x02
#define COMMAND_LEVEL_STOP                                0x03
#define COMMAND_LEVEL_MOVE_TO_LEVEL_WITH_ON_OFF           0x04
#define COMMAND_LEVEL_MOVE_WITH_ON_OFF                    0x05
#define COMMAND_LEVEL_STEP_WITH_ON_OFF                    0x06
#define COMMAND_LEVEL_STOP_WITH_ON_OFF                    0x07
c: command move to level - 00, move - 01
l: level value, 0 ~ FF
t: time, 100ms * n, philip used n = 4.
hd l  cT cT    addr  e  level l  aM    sI c  v  t     cs
fe 0e 29 00 0b ec 12 0b 08 00 07 02 11 02 04 00 64 00 a7 move to level => 00
fe 0e 29 00 0b ec 12 0b 08 00 07 02 11 02 04 ff 64 00 58 move to level => ff
LIGHT - color:
#define COMMAND_LIGHTING_MOVE_TO_HUE                                     0x00
#define COMMAND_LIGHTING_MOVE_HUE                                        0x01
#define COMMAND_LIGHTING_STEP_HUE                                        0x02
#define COMMAND_LIGHTING_MOVE_TO_SATURATION                              0x03
#define COMMAND_LIGHTING_MOVE_SATURATION                                 0x04
#define COMMAND_LIGHTING_STEP_SATURATION                                 0x05
#define COMMAND_LIGHTING_MOVE_TO_HUE_AND_SATURATION                      0x06
#define COMMAND_LIGHTING_MOVE_TO_COLOR                                   0x07
#define COMMAND_LIGHTING_MOVE_COLOR                                      0x08
#define COMMAND_LIGHTING_STEP_COLOR                                      0x09
#define COMMAND_LIGHTING_MOVE_TO_COLOR_TEMPERATURE                       0x0a
#define COMMAND_LIGHTING_ENHANCED_MOVE_TO_HUE                            0x40
#define COMMAND_LIGHTING_ENHANCED_MOVE_HUE                               0x41
#define COMMAND_LIGHTING_ENHANCED_STEP_HUE                               0x42
#define COMMAND_LIGHTING_ENHANCED_MOVE_TO_HUE_AND_SATURATION             0x43
#define COMMAND_LIGHTING_COLOR_LOOP_SET                                  0x44
#define COMMAND_LIGHTING_STOP_MOVE_STEP                                  0x47
c: HUE & SAT(refer to COMMAND_LIGHTING_XXX)
S: SAT 0 ~ FE
H: color (360dot)
888 => (v of move to level with onoff), H, S
hd l  cT cT    addr  e  color l  aM    sI c  H  S  t     cs
FE 0F 29 00 0B EC 12 0B 00 03 08 02 11 00 06 00 FE 00 00 00

RSP:
FE 01 69 00 00 68

Color Temprature:
cT: color Temprature (2000K ~ 6500K(7500K)) colorTemperature = (uint16)(1000000L/(uint32)colorTemperature);
hd l  cT cT    addr  e  temp  l  aM    sI c  cT    t     cs
fe 0f 29 00 0b ec 12 0b 00 03 08 02 11 00 0a 4d 01 00 00 86 warmer
fe 0f 29 00 0b ec 12 0b 00 03 08 02 11 00 0a a6 00 00 00 6c colder

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

void zllSocWriteCIEAddr(uint16_t dstAddr, uint8_t endpoint, uint8_t *addr)
{
  uint8_t cmd[27],i=0,cmdLength;

  i=0;
  cmd[i++] = 0xFE;
  cmd[i++] = 22;//----7+0x12;   /*RPC payload Len */          
  cmd[i++] =  0x29; /*MT_RPC_CMD_AREQ + MT_RPC_SYS_APP */          
  cmd[i++] =0x00; /*MT_APP_MSG  */          
  cmd[i++] =0x0B; /*Application Endpoint */          
  cmd[i++] =0x00;//(dstAddr & 0x00ff);
  cmd[i++] =0x00;//(dstAddr & 0xff00) >> 8;
  cmd[i++] =0x00;//endpoint; /*Dst EP */          
  cmd[i++] =(ZCL_CLUSTER_ID_SS_IAS_ZONE& 0x00ff);
  cmd[i++] =(ZCL_CLUSTER_ID_SS_IAS_ZONE& 0xff00) >> 8;
  cmd[i++] =15;//-----0x12; //Data Len
  cmd[i++] =0x02;//addrMode; 
  cmd[i++] =0x10; //0x00 ZCL frame control field.  not specific to a cluster (i.e. a SCL founadation command)
  cmd[i++] =0x00;//-----0x00;//transSeqNumber++;
  cmd[i++] =ZCL_CMD_WRITE;
  cmd[i++] =0x10;       //attr low  CIE addr
  cmd[i++] =0;        //attr hi
  cmd[i++] =ZCL_DATATYPE_IEEE_ADDR;
  cmd[i++] = *addr++;
  cmd[i++] = *addr++;
  cmd[i++] = *addr++;
  cmd[i++] = *addr++;
  cmd[i++] = *addr++;
  cmd[i++] = *addr++;
  cmd[i++] = *addr++;
  cmd[i++] = *addr++;
  cmd[i++] = 0;
  cmd[5] = (dstAddr & 0x00ff);
  cmd[6] = (dstAddr & 0xff00) >> 8;
  cmd[7] = endpoint; /*Dst EP */  
  cmd[13] = transSeqNumber++;

  cmdLength=sizeof(cmd);
  calcFcs(cmd, cmdLength);
  UARTwrite((const char *)cmd, cmdLength);
/*  for(i=0;i<cmdLength;i++)
    {
    printf("%02x ",cmd[i]);
    }*/
  free(cmd);
}
******************** only for CIE ******************
void zclSocGetZoneType(uint16_t dstAddr, uint8_t endpoint,uint8_t addrMode)
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
    (ZCL_CLUSTER_ID_SS_IAS_ZONE & 0x00ff),
    (ZCL_CLUSTER_ID_SS_IAS_ZONE & 0xff00) >> 8,
    0x06, //Data Len
      0x00,//addrMode, 
      0x00, //0x00 ZCL frame control field.  not specific to a cluster (i.e. a SCL founadation command)
      0x00,//transSeqNumber++,
    ZCL_CMD_READ,
    (ATTRID_SS_IAS_ZONE_TYPE & 0x00ff),
    (ATTRID_SS_IAS_ZONE_TYPE & 0xff00) >> 8,
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

void zllSocSetHueSat(uint8_t hue, uint8_t sat, uint16_t time, uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode)
{
  uint8_t cmd[] = { 
    0xFE, 
    15, //RPC payload Len
    0x29, //MT_RPC_CMD_AREQ + MT_RPC_SYS_APP
    0x00, //MT_APP_MSG
    0x0B, //Application Endpoint         
      0x00,//(dstAddr & 0x00ff),
      0x00,//(dstAddr & 0xff00) >> 8,
      0x00,//endpoint, /*Dst EP */          
    (ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL & 0x00ff),
    (ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL & 0xff00) >> 8,
    0x08, //Data Len
      0x00,//addrMode, 
    0x11, //ZCL Header Frame Control
      0x00,//transSeqNumber++,
    0x06, //ZCL Header Frame Command (COMMAND_LEVEL_MOVE_TO_HUE_AND_SAT)
    0x00,//hue, //HUE - fill it in later
    0x00,/t, //SAT - fill it in later
    0x00,//(time & 0xff),
    0x00,//(time & 0xff00) >> 8,
    0x00 //fcs
  }; 
      cmd[5]=(dstAddr & 0x00ff);
      cmd[6]=(dstAddr & 0xff00) >> 8;
      cmd[7]=endpoint; /*Dst EP */          
          cmd[11]=addrMode;
    cmd[13]=transSeqNumber++;
    cmd[15]=hue; //HUE - fill it in later
    cmd[16]=sat; //SAT - fill it in later
    cmd[17]=(time & 0xff);
    cmd[18]=(time & 0xff00) >> 8;

  calcFcs(cmd, sizeof(cmd));
  UARTwrite((const char *)cmd, sizeof(cmd));
  if(addrMode==afAddr16Bit)
    {
    SetEpHue(dstAddr,endpoint,hue);
    SetEpSat(dstAddr,endpoint,sat);
    }   
}

void zllSocSetColorTemperature(uint16_t saddr,uint8_t endpoint, uint8_t addrMode,uint16_t value)
{
  uint8_t cmd[] = {
    0xFE,
    0x0F,
    0x29,
    0x00,
    0x0B,
    0,
    0,
    0,
    (0x0300&0xff),
    ((0x0300&0xff00)/256),
    0x08,
    0x02,
    0x11,
    0,
    0x0a,
    0,
    0,
    (0&0xff),
    ((0&0xff00)/256),
    0
  };
  cmd[5] = (saddr&0xff);
  cmd[6] = ((saddr&0xff00)/256);
  cmd[7] = endpoint;
  cmd[13] = transSeqNumber++;
  cmd[11] = addrMode;
  cmd[15] = (value&0xff);
  cmd[16] = ((value&0xff00)/256);
  calcFcs(cmd, sizeof(cmd));
  UARTwrite((const char *)cmd, sizeof(cmd));

