
#include "leconfig.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"

#include "hal_uart.h"
#include "hal_gpio.h"
#include "io.h"
#include "flash_map.h"
#include "fota_76x7.h"


#define BOOTLOADER_MAX_MESSAGE_LENGTH 255
#define DELAYMS_FOR_IO 5
#define memcpy os_memcpy
#define memset os_memset

static int eUART_Write(uartHandler_t* handler, uint8_t *pu8Data, int iLength);
static int eBL_WriteMessage(uartHandler_t* handler, uint8_t eType, uint8_t u8HeaderLength, uint8_t *pu8Header, uint8_t u8Length, uint8_t *pu8Data) {
    uint8_t u8CheckSum = 0;
    int n;

    uint8_t au8Msg[256];

    APPLOG("u8HeaderLength[%d] u8Length[%d]", u8HeaderLength, u8Length);
    /* total message length cannot be > 255 bytes */
    if(u8HeaderLength + u8Length >= 0xfe)
    {
        APPLOGE("Length too big");
        return -1;
    }

    /* Message length */
    au8Msg[0] = u8HeaderLength + u8Length + 2;

    /* Message type */
    au8Msg[1] = (uint8_t)eType;

    /* Message header */
    memcpy(&au8Msg[2], pu8Header, u8HeaderLength);

    /* Message payload */
    memcpy(&au8Msg[2 + u8HeaderLength], pu8Data, u8Length);

    // APPPRINTF("Tx: ");
    for(n = 0; n < u8HeaderLength + u8Length + 2; n++)
    {
        // APPPRINTF("%02x ", au8Msg[n]);
        u8CheckSum ^= au8Msg[n];
    }

    /* Message checksum */
    au8Msg[u8HeaderLength + u8Length + 2] = u8CheckSum;
    // APPPRINTF("%02x ", u8CheckSum);
    // APPPRINTF("\r\n");
    /* Write whole message to UART */
    return eUART_Write(handler, au8Msg, u8HeaderLength + u8Length + 3);
}

static int eUART_Read(uartHandler_t* handler, int iTimeoutMicroseconds, int iBufferLen, uint8_t *pu8Buffer, int *piBytesRead) {
    int ret = 0, count = 0, singleMS = DELAYMS_FOR_IO, tmpLen = 0;
    do {
        // APPLOGW("eUART_Read test[%d/%d]", tmpLen, iBufferLen);
        ret = halUartRead(handler, pu8Buffer + tmpLen, iBufferLen - tmpLen);
        if (0 >= ret) {
            // APPLOGW("eUART_Read count[%d]", count);
            halDelayms(singleMS);
            count++;
        } else {
            tmpLen += ret;
        }
    } while (tmpLen < iBufferLen && count*singleMS*1000 < iTimeoutMicroseconds);
    *piBytesRead = tmpLen;
    if (tmpLen > 0) {
        int n = 0;        
        APPLOGW("eUART_Read len[%d]", tmpLen);
        APPPRINTF("eUART_Read: ");
        for (n = 0; n < tmpLen; n++) {
            APPPRINTF("%02x ", pu8Buffer[n]);
        }
        APPPRINTF("\r\n");
    }
    return tmpLen > 0 ? 0 : -1;
}

static int eUART_Write(uartHandler_t* handler, uint8_t *pu8Data, int iLength) {
    int iBytesWritten;
    int iTotalBytesWritten = 0;
    
    if(pu8Data == NULL) {
        return -1;
    }

    do {
        iBytesWritten = halUartWrite(handler, &pu8Data[iTotalBytesWritten], iLength - iTotalBytesWritten);
        if(0 >= iBytesWritten) {
            halDelayms(DELAYMS_FOR_IO);
        } else {
            iTotalBytesWritten += iBytesWritten;
        }
    } while(iTotalBytesWritten < iLength);

    APPLOG("eUART_Write iLength[%d] iTotalBytesWritten[%d]", iLength, iTotalBytesWritten);
    return 0;
}

static uint8_t eBL_ReadMessage(uartHandler_t* handler, int iTimeoutMicroseconds, uint8_t *peType, uint8_t *pu8Length, uint8_t *pu8Data)
{

    int n;
    int eStatus;
    uint8_t au8Msg[BOOTLOADER_MAX_MESSAGE_LENGTH];
    uint8_t u8CalculatedCheckSum = 0;
    uint8_t u8Length = 0;
    uint8_t eResponse = 0;
    int iAttempts = 0;
    int iBytesRead = 0;
    int iTotalBytesRead = 0;

    APPLOG("eBL_ReadMessage iTimeoutMicroseconds %d", iTimeoutMicroseconds);

    /* Get the length byte */
    if((eUART_Read(handler, iTimeoutMicroseconds, 1, &u8Length, &iBytesRead) != 0) || (iBytesRead != 1))
    {
        APPLOGE("Error getting length");
        return 0xf6/*E_BL_RESPONSE_NO_RESPONSE*/;
    }

    APPLOG("Incoming message length %d", u8Length);

    /* Message must have at least 3 bytes, maximum is implicit */
    if (u8Length < 3)
    {
        APPLOGE("u8Length error");
        return 0xf6/*E_BL_RESPONSE_NO_RESPONSE*/;
    }

    /* Add length to checksum */
    u8CalculatedCheckSum ^= u8Length;

    do
    {
        APPLOG("eBL_ReadMessage iTimeoutMicroseconds[%d] u8Length[%d] iTotalBytesRead[%d] iBytesRead[%d]", 
            iTimeoutMicroseconds, u8Length, iTotalBytesRead, iBytesRead);
        /* Get the rest of the message */
        eStatus = eUART_Read(handler, iTimeoutMicroseconds, u8Length - iTotalBytesRead, &au8Msg[iTotalBytesRead], &iBytesRead);
        if(eStatus != 0) {
            APPLOG("Error reading message from UART [%d]", eStatus);
            return 0xf6/*E_BL_RESPONSE_NO_RESPONSE*/;
        }

        iTotalBytesRead += iBytesRead;
        iAttempts++;

    } while ((iTotalBytesRead < u8Length) && (iBytesRead > 0 || iAttempts < 10));

    if(iTotalBytesRead != u8Length)
    {
        APPPRINTF("Got %d bytes but expected %d after %d attempts: ", iTotalBytesRead, u8Length, iAttempts);
        for(n = 0; n < iTotalBytesRead; n++)
        {
            APPPRINTF("%02x ", au8Msg[n]);
        }
        APPPRINTF("\r\n");

        return 0xf6/*E_BL_RESPONSE_NO_RESPONSE*/;
    }

    /* Add rest of message to checksum */
    APPPRINTF("Rx: %02x ", u8Length);
    for(n = 0; n < u8Length; n++)
    {
        APPPRINTF("%02x ", au8Msg[n]);
        u8CalculatedCheckSum ^= au8Msg[n];
    }
    APPPRINTF("\r\n");

    if(u8CalculatedCheckSum != 0x00)
    {
        APPLOG("Checksum bad, got %02x expected %02x", u8CalculatedCheckSum, 0);
        return 0xfc/*E_BL_RESPONSE_CRC_ERROR*/;
    }

    *peType = au8Msg[0];
    eResponse = au8Msg[1];
    if (pu8Length)
    {
        *pu8Length = u8Length - 3;

        if (pu8Data)
        {
            memcpy(pu8Data, &au8Msg[2], *pu8Length);
        }
    }

    APPLOG("Got response 0x%02x", eResponse);

    return eResponse;
}

static uint8_t eBL_Request(uartHandler_t* handler, int iTimeoutMicroseconds, uint8_t eTxType, uint8_t u8HeaderLen, uint8_t *pu8Header, uint8_t u8TxLength, uint8_t *pu8TxData,
                          uint8_t *peRxType, uint8_t *pu8RxLength, uint8_t *pu8RxData)
{
    /* Check data is not too long */
    if(u8TxLength > 0xfd)
    {
        APPLOGE("Data too long");
        // return E_BL_RESPONSE_ERROR;
        return 0xf0;
    }

    /* Send message */
    if(eBL_WriteMessage(handler, eTxType, u8HeaderLen, pu8Header, u8TxLength, pu8TxData) != 0)
    {
        // return E_BL_RESPONSE_ERROR;
        APPLOGE("eBL_WriteMessage");
        return 0xf0;
    }
    // halDelayms(100);
    // TODO: 读取不完整的补丁

    if (0x27 == eTxType || 0x07 == eTxType || 0x09 == eTxType || 0x2c == eTxType) {
        uint8_t buf[8] = {0};
        int readBytes = 0;
        int size = sizeof(buf);
        if (0x09 == eTxType) {
            size = 4;
        }
        eUART_Read(handler, 2*1000*1000, size, buf, &readBytes);
        *peRxType = buf[1];
        // APPLOG("TODO: partial data?");
        return buf[2];
    }

    /* Get the response to the request */
    return eBL_ReadMessage(handler, iTimeoutMicroseconds, peRxType, pu8RxLength, pu8RxData);
}

static int eBL_CheckResponse(const char *pcFunction, int eResponse, uint8_t eRxType, uint8_t eExpectedRxType)
{
    APPLOG("%s: Response %02x", pcFunction, eResponse);
    switch (eResponse)
    {
        case(0x00):
            break;
        case (0xff/*E_BL_RESPONSE_NOT_SUPPORTED*/):
            return -1; // E_PRG_UNSUPPORTED_OPERATION;
        case (0xfe/*E_BL_RESPONSE_WRITE_FAIL*/):
            return -2; // E_PRG_ERROR_WRITING;
        case (0xf9/*E_BL_RESPONSE_READ_FAIL*/):
            return -3; // E_PRG_ERROR_READING;
        default:
            return -4; // E_PRG_COMMS_FAILED;
    }

    if (eRxType != eExpectedRxType)
    {
        APPLOG("%s: Got type 0x%02x, expected 0x%02x", pcFunction, eRxType, eExpectedRxType);
        return -5; // E_PRG_COMMS_FAILED;
    }
    return 0;
}

static void *OpenUART(uartHandler_t *hdl, int baud) {
    hal_gpio_init(36);
    hal_pinmux_set_function(36, HAL_GPIO_36_UART2_RX_CM4);
    hal_gpio_deinit(36);
    hal_gpio_init(37);
    hal_pinmux_set_function(37, HAL_GPIO_37_UART2_TX_CM4);
    hal_gpio_deinit(37);
    hdl->id = 1;
    // hdl->baud = 1000000; not support
    hdl->baud = baud;
    hdl->dataBits = 8;
    hdl->stopBits = 1;
    hdl->parity = 0;
    return halUartOpen(hdl);
}

static int eBL_FlashSelectDevice(uartHandler_t* handler, uint8_t u8ManufacturerID, uint8_t u8DeviceID, uint8_t u8ChipSelect)
{
    int n;
    int eResponse = 0;
    uint8_t eRxType = 0;
    uint8_t au8Buffer[6];
    uint8_t u8FlashType = 8;
    int eStatus;

    au8Buffer[0] = u8FlashType;
    au8Buffer[1] = 0;
    au8Buffer[2] = 0;
    au8Buffer[3] = 0;
    au8Buffer[4] = 0;

    APPLOG("eBL_FlashSelectDevice... u8ManufacturerID[0x%02x] u8DeviceID[0x%02x] u8ChipSelect[0x%02x]", u8ManufacturerID, u8DeviceID, u8ChipSelect);
    eResponse = eBL_Request(handler, 1*1000*1000, 0x2c/*E_BL_MSG_TYPE_FLASH_SELECT_TYPE_REQUEST*/, 5, au8Buffer, 0, NULL, &eRxType, NULL, NULL);
    return eBL_CheckResponse(__FUNCTION__, eResponse, eRxType, 0x2d/*E_BL_MSG_TYPE_FLASH_SELECT_TYPE_RESPONSE*/);
}

static int eBL_FlashErase(uartHandler_t* handler)
{
    int eResponse = 0;
    uint8_t eRxType = 0;

    APPLOG("eBL_FlashErase...");
    eResponse = eBL_Request(handler, 10*1000*1000, 0x07/*E_BL_MSG_TYPE_FLASH_ERASE_REQUEST*/, 0, NULL, 0, NULL, &eRxType, NULL, NULL);
    return eBL_CheckResponse(__FUNCTION__, eResponse, eRxType, 0x08/*E_BL_MSG_TYPE_FLASH_ERASE_RESPONSE*/);
}

static int eBL_FlashWrite(uartHandler_t* handler, uint32_t u32Address, uint8_t u8Length, uint8_t *pu8Buffer)
{
    uint8_t au8CmdBuffer[4];
    int eResponse = 0;
    uint8_t eRxType = 0;

    if(u8Length > 0xfc || pu8Buffer == NULL)
    {
        return -1;
    }

    au8CmdBuffer[0] = (uint8_t)(u32Address >> 0)  & 0xff;
    au8CmdBuffer[1] = (uint8_t)(u32Address >> 8)  & 0xff;
    au8CmdBuffer[2] = (uint8_t)(u32Address >> 16) & 0xff;
    au8CmdBuffer[3] = (uint8_t)(u32Address >> 24) & 0xff;

    APPLOG("eBL_FlashWrite...");
    eResponse = eBL_Request(handler, 1*1000*1000, 0x09/*E_BL_MSG_TYPE_FLASH_PROGRAM_REQUEST*/, 4, au8CmdBuffer, u8Length, pu8Buffer, &eRxType, NULL, NULL);
    return eBL_CheckResponse(__FUNCTION__, eResponse, eRxType, 0x0a/*E_BL_MSG_TYPE_FLASH_PROGRAM_RESPONSE*/);
}

static int eBL_SetBaudrate(uartHandler_t* handler, uint32_t u32Baudrate)
{
    int eResponse = 0;
    uint8_t eRxType = 0;
    uint8_t au8Buffer[6];
    uint32_t u32Divisor = 9;

    // APPLOG("Set BL Baud rate to %d", u32Baudrate);

    // Divide 1MHz clock by baudrate to get the divisor
    // u32Divisor = (uint32_t)roundf(1000000.0 / (float)u32Baudrate);

    au8Buffer[0] = (uint8_t)u32Divisor;
    au8Buffer[1] = 0;
    au8Buffer[2] = 0;
    au8Buffer[3] = 0;
    au8Buffer[4] = 0;

    APPLOG("eBL_SetBaudrate... u32Divisor[%d]", u32Divisor);
    eResponse = eBL_Request(handler, 1000*1000, 0x27/*E_BL_MSG_TYPE_SET_BAUD_REQUEST*/, 1, au8Buffer, 0, NULL, &eRxType, NULL, NULL);
    return eBL_CheckResponse(__FUNCTION__, eResponse, eRxType, 0x28/*E_BL_MSG_TYPE_SET_BAUD_RESPONSE*/);
}

static int eBL_Reset(uartHandler_t* handler)
{
    int eResponse = 0;
    uint8_t eRxType = 0;

    eResponse = eBL_Request(handler, 1*1000*1000, 0x14/*E_BL_MSG_TYPE_RESET_REQUEST*/, 0, NULL, 0, NULL, &eRxType, NULL, NULL);
    return eBL_CheckResponse(__FUNCTION__, eResponse, eRxType, 0x15/*E_BL_MSG_TYPE_RESET_RESPONSE*/);
}

static int eBL_MemoryWrite(uartHandler_t* handler, uint32_t u32Address, uint8_t u8Length, uint8_t u8BlockSize, uint8_t *pu8Buffer)
{
    uint8_t au8CmdBuffer[6];
    int eResponse = 0;
    uint8_t eRxType = 0;
    int i;

    if(u8Length > 0xfc || pu8Buffer == NULL)
    {
        return -1;
    }

    au8CmdBuffer[0] = (uint8_t)(u32Address >> 0)  & 0xff;
    au8CmdBuffer[1] = (uint8_t)(u32Address >> 8)  & 0xff;
    au8CmdBuffer[2] = (uint8_t)(u32Address >> 16) & 0xff;
    au8CmdBuffer[3] = (uint8_t)(u32Address >> 24) & 0xff;
    au8CmdBuffer[4] = u8Length;
    au8CmdBuffer[5] = 0;

    /* Convert the native byte buffer into chip format */
    if (u8BlockSize == sizeof(uint16_t))
    {
        for (i = 0; i < u8Length; i += sizeof(uint16_t))
        {
            uint16_t u16Short;
            memcpy(&u16Short, &pu8Buffer[i], sizeof(uint16_t));

            // if (psContext->sChipDetails.eEndianness == E_CHIP_LITTLE_ENDIAN)
            // {
            //     u16Short = htole16(u16Short);
            // }
            // else if (psContext->sChipDetails.eEndianness == E_CHIP_BIG_ENDIAN)
            // {
            //     u16Short = htobe16(u16Short);
            // }
            // else
            // {
            //     return E_PRG_BAD_PARAMETER;
            // }
            memcpy(&pu8Buffer[i], &u16Short, sizeof(uint16_t));
        }
    }
    else if (u8BlockSize == sizeof(uint32_t))
    {
        for (i = 0; i < u8Length; i += sizeof(uint32_t))
        {
            uint32_t u32Word;
            memcpy(&u32Word, &pu8Buffer[i], sizeof(uint32_t));

            // if (psContext->sChipDetails.eEndianness == E_CHIP_LITTLE_ENDIAN)
            // {
            //     u32Word = htole32(u32Word);
            // }
            // else if (psContext->sChipDetails.eEndianness == E_CHIP_BIG_ENDIAN)
            // {
            //     u32Word = htobe32(u32Word);
            // }
            // else
            // {
            //     return E_PRG_BAD_PARAMETER;
            // }
            memcpy(&pu8Buffer[i], &u32Word, sizeof(uint32_t));
        }
    }

    eResponse = eBL_Request(handler, 250*1000, 0x1d/*E_BL_MSG_TYPE_RAM_WRITE_REQUEST*/, 4, au8CmdBuffer, u8Length, pu8Buffer, &eRxType, NULL, NULL);
    return eBL_CheckResponse(__FUNCTION__, eResponse, eRxType, 0x1e/*E_BL_MSG_TYPE_RAM_WRITE_RESPONSE*/);
}

void haalCoOTASetFlag(uint32_t flag) {
    int ret = 0;
    PrivateCfg priCfg;
    APPLOG("haalCoOTASetFlag len[%d]", flag);

    ret = lelinkStorageReadPrivateCfg(&priCfg);
    if (0 > ret) {
        APPLOGE("haalCoOTASetFlag ret[%d] or csum Failed", ret);
        return;
    }
    priCfg.data.devCfg.sdevFWSize = flag;
    ret = lelinkStorageWritePrivateCfg(&priCfg);
    if (0 > ret) {
        APPLOGE("haalCoOTASetFlag write Failed[%d]", ret);
        return;
    }
    APPLOG("haalCoOTASetFlag Write done");
}

int CoOTAGetFlag(int *updateSize) {
    int ret = 0;
    PrivateCfg priCfg;
    ret = lelinkStorageInit(CM4_FLASH_LELINK_CUST_ADDR, FLASH_LELINK_CUST_SIZE, 0x1000);//CM4 buff slim:128KB + fota buff slim:128KB;->totalSize:0x40000
    if (0 > ret) {
        APPLOGE("lelinkStorageInit ret[%d]\r\n", ret);
        return 0;
    }
    ret = lelinkStorageReadPrivateCfg(&priCfg);
    if (0 > ret) {
        APPLOGW("CoOTAGetFlag ret[%d] or csum Failed", ret);
        return 0;
    }

    if (0xFFFFFFFF == priCfg.data.devCfg.sdevFWSize || 0 == priCfg.data.devCfg.sdevFWSize || 
        0x80000 < priCfg.data.devCfg.sdevFWSize) {
        APPLOGW("CoOTAGetFlag get size[%d|%x]", priCfg.data.devCfg.sdevFWSize, priCfg.data.devCfg.sdevFWSize);
        return 0;
    }

    *updateSize = priCfg.data.devCfg.sdevFWSize;
    APPLOG("CoOTAGetFlag Flag got size[%d|%x]", priCfg.data.devCfg.sdevFWSize, priCfg.data.devCfg.sdevFWSize);
    return 1;
}
// [LEAPP] haalCoOTAProcessing PULLDOWN over @halOTA.c:632
// [LEAPP] haalCoOTAProcessing to Open UART @halOTA.c:636
// SELECT 
// halUartWrite [8]: 07 2c 08 00 00 00 00 23 
// [LEAPP] eUART_Write iLength[8] iTotalBytesWritten[8] @halOTA.c:376
// [LEAPP[W]] eUART_Read len[4] @halOTA.c:349
// eUART_Read: 03 2d 00 2e 
// [LEAPP] Set divisor 9 @halOTA.c:584
// [LEAPP] u8HeaderLength[1] u8Length[0] @halOTA.c:298
// Tx: 03 27 09 
// BAUD 
// halUartWrite [4]: 03 27 09 2d 
// [LEAPP] eUART_Write iLength[4] iTotalBytesWritten[4] @halOTA.c:376
// [LEAPP[W]] eUART_Read len[4] @halOTA.c:349
// eUART_Read: 03 28 00 2b 
// [LEAPP] eBL_SetBaudrate: Response 00 @halOTA.c:505
// [LEAPP] u8HeaderLength[0] u8Length[0] @halOTA.c:298
// Tx: 02 07 
// ERASE
// halUartWrite [3]: 02 07 05 
// [LEAPP] eUART_Write iLength[3] iTotalBytesWritten[3] @halOTA.c:376
// [LEAPP[W]] eUART_Read len[4] @halOTA.c:349
// eUART_Read: 03 08 00 0b 
// [LEAPP] eBL_FlashErase: Response 00 @halOTA.c:505

static int OpenGPIO(gpioHandler_t *hdlGPIO, int id) {
    int halId = 0;
    hal_gpio_init(id);
    if (25 == id) {
        hal_pinmux_set_function(id, HAL_GPIO_25_GPIO25);
    } else if (26 == id) {
        hal_pinmux_set_function(id, HAL_GPIO_26_GPIO26);
    } else {
        return -1;
    }
    hal_gpio_deinit(id);
    hdlGPIO->id = id;
    hdlGPIO->dir = GPIO_DIR_OUTPUT;
    hdlGPIO->mode = GPIO_MODE_DEFAULT; // GPIO_MODE_PULLDOWN, GPIO_MODE_PULLUP, GPIO_MODE_DEFAULT
    if (0 > (halId = halGPIOOpen(hdlGPIO))) {
        return -2;
    }

    return halId;
}

void CoOTAReset(int bootOrNormal) {
    gpioHandler_t hdlGPIOMiso = {0};
    gpioHandler_t hdlGPIOReset = {0};
    int ret = 0, idMiso = 25, idReset = 26;
    APPLOG("CoOTAReset START");

    if (0 > OpenGPIO(&hdlGPIOMiso, idMiso)) {
        APPLOGE("CoOTAReset OpenGPIO MISO");
        return;
    }

    if (bootOrNormal) {
        APPLOG("CoOTAReset Boot");
        halGPIOWrite(&hdlGPIOMiso, 0);
        halDelayms(700);
    } else {
        APPLOG("CoOTAReset Normal");
        halGPIOWrite(&hdlGPIOMiso, 1);
        halDelayms(50);
    }
    
    // test only
    if (0)
    {
        int m = 2;
        do {
            int k = 0;
            halGPIORead(&hdlGPIOMiso, &k);
            APPLOG("halGPIORead hdlGPIOMiso[%d] val[%d]", hdlGPIOMiso.id, k);
            halDelayms(1000);
        } while (m--);
    }
    // halGPIOWrite(&hdlGPIOMiso, 1);

    // reset Co
    if (0 > OpenGPIO(&hdlGPIOReset, idReset)) {
        APPLOGE("CoOTAReset OpenGPIO RESET");
    }
    halGPIOWrite(&hdlGPIOReset, 0);
    halDelayms(50);
    // test only
    if (0)
    {
        int m = 2;
        do {
            int k = 0;
            halGPIORead(&hdlGPIOReset, &k);
            APPLOG("halGPIORead hdlGPIOReset[%d] val[%d]", hdlGPIOReset.id, k);
            halDelayms(1000);
        } while (m--);
    }
    halGPIOWrite(&hdlGPIOReset, 1);
    // test only
    if (0)
    {
        int m = 2;
        do {
            int k = 0;
            halGPIORead(&hdlGPIOReset, &k);
            APPLOG("halGPIORead recovery hdlGPIOReset[%d] val[%d]", hdlGPIOReset.id, k);
            halDelayms(1000);
        } while (m--);
    }

    halGPIOClose(&hdlGPIOReset);
    halDelayms(50);

    // recover the miso, keep in normal
    // test only
    if (0)
    {
        int m = 2;
        do {
            int k = 0;
            halGPIORead(&hdlGPIOMiso, &k);
            APPLOG("halGPIORead hdlGPIOMiso[%d] val[%d]", hdlGPIOMiso.id, k);
            halDelayms(1000);
        } while (m--);
    }
    halGPIOWrite(&hdlGPIOMiso, 1);

    // test only
    if (0)
    {
        int m = 2;
        do {
            int k = 0;
            halGPIORead(&hdlGPIOMiso, &k);
            APPLOG("halGPIORead recovery hdlGPIOMiso[%d] val[%d]", hdlGPIOMiso.id, k);
            halDelayms(1000);
        } while (m--);
    }
    halGPIOClose(&hdlGPIOMiso);

    APPLOG("CoOTAReset END");
}

void haalCoOTAProcessing(void) {
    void *uart = NULL;
    uint8_t buf[512] = {0};
    int readBytes = 0, ret = 0, baud = 115200, baudBoot = 38400;
    int n, updateSize = 0;
    uint8_t u8ChunkSize;
    uartHandler_t hdlUART = {0};
// 01 00    11 00    00    11 03
// 01 02 10 11 02 10 02 10 11 03
    // const uint8_t reset[] = {0x01, 0x02, 0x10, 0x11, 0x02, 0x10, 0x02, 0x10, 0x11, 0x03};
    // gpioHandler_t hdlGPIO = {0};
    APPLOG("haalCoOTAProcessing START");
    if (!CoOTAGetFlag(&updateSize)) {
        APPLOGW("CoOTAGetFlag NO");
        return;
    }

    // reset & into Boot Mode
    APPLOG("haalCoOTAProcessing boot mode START");
    CoOTAReset(1);
    APPLOG("haalCoOTAProcessing boot mode END");


    // clear the uart cache
    hdlUART.id = 1;
    halUartClose(&hdlUART);
    if (NULL == OpenUART(&hdlUART, baudBoot)) {
        APPLOGE("CoOTAGetFlag OpenUART Failed");
        return;
    }
    do {
        eUART_Read(&hdlUART, 100*1000, 1, &buf[0], &readBytes);
        APPLOGW("haalCoOTAProcessing halUartRead [%d]", readBytes);
    } while(readBytes > 0);

    ret = eBL_FlashSelectDevice(&hdlUART, 0xCC, 0xEE, 0);
    if (0 != ret) {
        halUartClose(&hdlUART);
        APPLOG("haalCoOTAProcessing eBL_FlashSelectDevice failed [%d]", ret);
        return;
    }
    // {
    //     int bytesRead = 0;
    //     uint8_t tmp[] = {0x07, 0x2c, 0x08, 0x00, 0x00, 0x00, 0x00, 0x23};
    //     eUART_Write(&hdlUART, tmp, sizeof(tmp));
    //     eUART_Read(&hdlUART, 1*1000*1000, sizeof(tmp), tmp, &bytesRead);
    // }

    ret = eBL_SetBaudrate(&hdlUART, baud);
    if (0 != ret) {
        halUartClose(&hdlUART);
        APPLOG("haalCoOTAProcessing eBL_SetBaudrate failed [%d]", ret);
        return;
    }
    halUartClose(&hdlUART);

    // reopen for uart update
    memset(&hdlUART, 0, sizeof(&hdlUART));
    if (NULL == OpenUART(&hdlUART, baud)) {
        APPLOGE("haalCoOTAProcessing halUartOpen failed for Boot");
        return;
    }

    ret = eBL_FlashErase(&hdlUART);
    if (0 != ret) {
        halUartClose(&hdlUART);
        APPLOG("haalCoOTAProcessing eBL_FlashErase failed [%d]", ret);
        return;
    }

    uint32_t u32ImageLength = updateSize - 4;
    uint32_t u32FlashOffset = 0;
#define chunkSize 128
    uint8_t chunk[chunkSize] = {0};
    // uint32_t startAddr = 0;
    uint32_t startAddr = CM4_FLASH_TMP_ADDR + 4;
    int eStatus = 0;
    void *hdlFlash = NULL;
    // fota_partition_t *p = NULL;
    hdlFlash = halFlashOpen();
    if (NULL == hdlFlash) {
        return;
    }
    // p = _fota_find_partition(CM4_FLASH_TMP_ADDR);
    // if (NULL == p)  {
    //     halFlashClose(hdlFlash);
    //     return;
    // }
    // startAddr = p->address + p->offset;
    for(n = 0; n < u32ImageLength; n += u8ChunkSize) {
        if((u32ImageLength - n) > chunkSize) {
            u8ChunkSize = chunkSize;
        }
        else {
            u8ChunkSize = u32ImageLength - n;
        }
        halFlashRead(hdlFlash, chunk, u8ChunkSize, startAddr, n);
        // eStatus = fota_read(FOTA_PARITION_TMP, chunk, u8ChunkSize);
        if (eStatus != FOTA_STATUS_OK) {
            APPLOGE("haalCoOTAProcessing fota_read Failed [%d]", eStatus);
            return;
        }
        APPLOG("haalCoOTAProcessing eBL_FlashWrite startAddr[%x + %x] u8ChunkSize[%d] ", startAddr, n, u8ChunkSize);
        if((eStatus = eBL_FlashWrite(&hdlUART, u32FlashOffset + n, u8ChunkSize, chunk)) != 0) {
            APPLOGE("haalCoOTAProcessing eBL_FlashWrite Failed [%d]", eStatus);
            // halFlashClose(hdlFlash);
            return;
        }
    }
    halFlashClose(hdlFlash);

    // 3. reset Co flag
    halUartClose(&hdlUART);
    haalCoOTASetFlag(0);
    CoOTAReset(0);
    APPLOG("haalCoOTAProcessing END");
    halReboot();
}