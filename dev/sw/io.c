#include "leconfig.h"
#include "io.h"
extern void *halFlashOpen(void);

int ioWrite(int type, const uint8_t *data, int dataLen) {
    return 0;
}

int ioRead(int type, uint8_t *data, int dataLen) {
    return 0;
}


// static uint8_t ginIsPrivateCfgChanged = 1;
// static PrivateCfg ginPrivateCfg;
int lelinkFlashWritePrivateCfg(const PrivateCfg *privateCfg) {
    int ret = 0;
    void *hdl = NULL;
    PrivateCfg *tmp;

    hdl = (void *)halFlashOpen();
    tmp = (PrivateCfg *)privateCfg;
    if (NULL == hdl) {
        return -1;
    }

    ret = halFlashErase(hdl, FLASH_ADDR_PRIVATE_CFG, FLASH_SIZE_PRIVATE_CFG);
    if (0 > ret) {
        return -2;
    }
    // LELOG("flashWritePrivateCfg halFlashErase [0x%x] [0x%x][0x%x]\r\n", hdl, FLASH_ADDR_PRIVATE_CFG, FLASH_SIZE_PRIVATE_CFG);

    tmp->csum = crc8(&(tmp->data), sizeof(tmp->data));
    ret = halFlashWrite(hdl, tmp, sizeof(PrivateCfg), FLASH_ADDR_PRIVATE_CFG);
    if (0 > ret) {
        return -3;
    }
    
    // LELOG("flashWritePrivateCfg halFlashWrite [0x%x] [0x%x][0x%x]\r\n", hdl, FLASH_ADDR_PRIVATE_CFG, FLASH_SIZE_PRIVATE_CFG);
    halFlashClose(hdl);
    return 0;
}

int lelinkFlashReadPrivateCfg(PrivateCfg *privateCfg) {
    int ret = 0;
    void *hdl;

    hdl = (void *)halFlashOpen();
    if (NULL == hdl) {
        return -1;
    }

    ret = halFlashRead(hdl, privateCfg, sizeof(PrivateCfg), FLASH_ADDR_PRIVATE_CFG);
    if (0 > ret) {
        return -2;
    }

    // LELOG("flashReadPrivateCfg [0x%x] [0x%x][0x%x]\r\n", hdl, FLASH_SIZE_PRIVATE_CFG, FLASH_ADDR_PRIVATE_CFG);
    halFlashClose(hdl);

    return 0;
}

// int flashWritePrivateCfg(const PrivateCfg *privateCfg) {
//     int ret = 0;
//     uint8_t buf[GET_SIZE(sizeof(PrivateCfg))] = {0};
//     void *hdl = (void *)halFlashOpen();
//     if (NULL == hdl) {
//         return -1;
//     }

//     ret = halFlashErase(hdl, FLASH_ADDR_PRIVATE_CFG, FLASH_SIZE_PRIVATE_CFG);
//     if (0 > ret) {
//         return -2;
//     }
//     LELOG("flashWritePrivateCfg halFlashErase [0x%x] [0x%x][0x%x]\r\n", hdl, FLASH_ADDR_PRIVATE_CFG, FLASH_SIZE_PRIVATE_CFG);

//     memcpy(buf, privateCfg, sizeof(PrivateCfg));
//     ret = halFlashWrite(hdl, buf, sizeof(buf), FLASH_ADDR_PRIVATE_CFG);
//     if (0 > ret) {
//         return -3;
//     }
//     LELOG("flashWritePrivateCfg halFlashWrite [0x%x] [0x%x][0x%x]\r\n", hdl, FLASH_ADDR_PRIVATE_CFG, FLASH_SIZE_PRIVATE_CFG);
//     halFlashClose(hdl);
//     return 0;
// }

// int flashReadPrivateCfg(PrivateCfg *privateCfg) {
//     int ret = 0;
//     uint8_t buf[GET_SIZE(sizeof(PrivateCfg))] = {0};
//     void *hdl = (void *)halFlashOpen();
//     if (NULL == hdl) {
//         return -1;
//     }

//     ret = halFlashRead(hdl, buf, sizeof(buf), FLASH_ADDR_PRIVATE_CFG);
//     if (0 > ret) {
//         return -2;
//     }
//     memcpy(privateCfg, buf, sizeof(PrivateCfg));
//     LELOG("flashReadPrivateCfg [0x%x] [0x%x][0x%x]\r\n", hdl, FLASH_SIZE_PRIVATE_CFG, FLASH_ADDR_PRIVATE_CFG);
//     halFlashClose(hdl);

//     return 0;
// }
