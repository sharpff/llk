#ifndef __HAL_HEADER_H__
#define __HAL_HEADER_H__

#include <stdio.h>
#include "FreeRTOS.h"
#include "sockets.h"
#include "task.h"
#include "queue.h"
#include "nvdm.h"

//#define HW_AES

int leLedRead(uint8_t *data, int* dataLen);
int leLedWrite(const uint8_t *data, int dataLen);
void leLedBlueFastBlink(void);
void leLedBlueSlowBlink(void);
void leLedSetDefault(void);
void leLedReset(void);
void leLedInit(void);
int leGetConfigMode(void);
int leSetConfigMode(uint8_t mode);
#endif
