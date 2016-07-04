## this is a description for fw script.

```function:```
s1GetCvtType()
```description:```
the function is for customer to config their IO. it support multiple IO.
0x01 - UART, refer to IO_TYPE_UART
0x02 - GPIO, refer to IO_TYPE_GPIO
0x04 - PIPE, refer to IO_TYPE_PIPE
0x08 - SOCKET, refer to IO_TYPE_SOCKET
param: none
```return:```
json string, descripts what kind of convertion types it supports.

### params description.
"whatCvtType":
>it is a mask for IO type.

"id":
>for UART, it is used for figure out the diff uart.
>for GPIO, refer to the halIO.c.

"uart":

"baud": 
>E.g. "baud":"115200-8N1" means Speed - 115200; Data Bits - 8; Parity - N; Stop Bits - 1

"gpio":

"dir": [0-1]
>0 - input; 1 - output

"state" [0-2]
>idle state. 0 - low; 1 - high; 2 - blink

"mode": [0-2]
>GPIO pin mode configuration. 0 - default; 1 - pullup; 2 - pulldown

"blink": [1-+]
>frequency of blinking, the smallest unit of the polling interval.
>E.g. the "blink" is 3 and the polling interval is 300ms. so, the interval is equal to 3*300.

"type": [0-1]
>0 - stdio; 1 - reset output/input

"longTime": [1-+]
>valid only in "type" is 1, the value should be lagger than "shortTime".

"shortTime": [1-+]
>valid only in "type" is 1, the value should be less than "longTime".


```
E.g. 
for single type: only for UART, the "whatCvtType" is 0x01.
{
"whatCvtType":1,
"uart":[
    {
        "id":1, 
        "baud":"115200-8N1"
    }
    ]
}
for multiple type: uart(0x1) & gpio(0x2), the "whatCvtType" is 0x03(0x01 | 0x02).
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
```

```function:```
s1GetQueries(queryType)
```description:``` 
it is only for the ENGINE to retrive the cmd for it's INTERNAL logic.
param: queryType, identified what exactly type is in querying.
it is a var in 4 bytes. it likes 0xFFFFFFFF.
0x000000XX is used for status indication. E.g. to get the cmd for blinking the light in monitor mode.
0x0000XX00 is used for status of device itself.
0x00XX0000 is used for status of sub device. In the most of time, the cmd should be sent by remote, but comming from the IO.
0xXX000000 is RESERVED
```return:```
json, it descripts what kind of convertion types it supports


```function:```
s1GetValidKind(data)
```description:```
get the valid data kind for ENGINE from the data that comes from IO.
WHATKIND_MAIN_DEV_RESET = 1
WHATKIND_MAIN_DEV_DATA = 2
WHATKIND_SUB_DEV_RESET = 10
WHATKIND_SUB_DEV_DATA = 11
WHATKIND_SUB_DEV_JOIN = 12
WHATKIND_SUB_DEV_LEAVE = 13
param: data, is a bin array, read from the IO.
```return:```
integer. less than 0(included 0) means invalid, otherwise engine will follow the next sequence(s1CvtPri2Std).

```function:```
s1CvtStd2Pri(json)
```description:```
it converts the standard lelink cmd to the individual data for IO.
param: json, is a standard lelink string(json as normal).
```return:```
individual data 

```function:```
s1CvtPri2Std(bin)
```description:```
it converts the individual data to the standard lelink info.
param: bin, is an individual data from IO
it needs to be explain more for GPIO case. the param(bin) is a byte type. it has been seperated as 2 parts. 0xF0 & bin[1] is 'id', refer to GPIO's id. 0x0F & bin[1] is its value for the id of GPIO.
```return:```
standard lelink string

