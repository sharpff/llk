#ifndef __HAL_HEADER_H__
#define __HAL_HEADER_H__

#include <stdio.h>
#include "FreeRTOS.h"
#include "sockets.h"
#include "task.h"
#include "queue.h"
#include "nvdm.h"

void CoOTAProcessing(void);
void CoOTAReset(int bootOrNormal);
int CoOTAGetFlag(int *updateSize);
void CoOTASetFlag(uint32_t flag);

#endif
