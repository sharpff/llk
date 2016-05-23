[this is a description for fw script.](www.baidu.com)

function: s1GetCvtType()
description:
the function is for customer to config their IO. it support multiple IO.
0x01 - UART, refer to IO_TYPE_UART
0x02 - GPIO, refer to IO_TYPE_GPIO
0x04 - PIPE, refer to IO_TYPE_PIPE
0x08 - SOCKET, refer to IO_TYPE_SOCKET
param: none
return: json string, descripts what kind of convertion types it supports.
-- whatCvtType: it is a mask for IO type.
-- id:
    for UART, XXX
    for GPIO, refer to the halIO.c.
E.g. 
*for single type: only for UART, the "whatCvtType" is 0x01.
 {
"whatCvtType":1,
"uart":[
    {
        "id":1, 
        "baud":"115200-8N1"
    }
    ]
}
*for multiple type: uart(0x1) & gpio(0x2), the "whatCvtType" is 0x03(0x01 | 0x02).
{
"whatCvtType":1,
"uart":[
    {
        "id":1, 
        "baud":"115200-8N1"
    }
    ],
"gpio":[
        {
            "id":1,
            "dir":0,
            "mode":2,
            "type":1,
            "longTime":10,
            "shortTime":3
        },
        {
            "id":2,
            "dir":1,
            "mode":0,
            "state":1,
            "blink":2,
            "type":1,
            "longTime":10,
            "shortTime":1
        }
    ]
}


function s1GetQueries(queryType)
description: 
it is only for the ENGINE to retrive the cmd for it's INTERNAL logic.
param: queryType, identified what exactly type is in querying.
it is a var in 4 bytes. it likes 0xFFFFFFFF.
0x000000XX is used for status indication. E.g. to get the cmd for blinking the light in monitor mode.
0x0000XX00 is used for status of device itself.
0x00XX0000 is used for status of sub device. In the most of time, the cmd should be sent by remote, but comming from the IO.
0xXX000000 is RESERVED
return: json, it descripts what kind of convertion types it supports
E.g. 


function s1GetValidKind(data)
description:
get the valid data kind for ENGINE from the data that comes from IO.
param: data, is a bin array, read from the IO.
return: integer. less than 0(included 0) means invalid, otherwise engine will follow the next sequence(s1CvtPri2Std).

function s1CvtStd2Pri(json)
description:
it converts the standard lelink cmd to the individual data for IO.
param: json, is a standard lelink string(json as normal).
return: individual data 

function s1CvtPri2Std(bin)
description:
it converts the individual data to the standard lelink info.
param: bin, is an individual data from IO
return: standard lelink string

