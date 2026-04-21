#ifndef _IAP_H_
#define _IAP_H_

#include <stdint.h>
#include "IAP_FLASH_MAP.H"
#include "IAP_Bootloader.h"

extern void Uart_Send_Complete_Check(void);
void IAP_Init(void);
void JumpToApp(uint32_t addr);

#endif


