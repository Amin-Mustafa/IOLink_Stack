#ifndef SYSTEM_H
#define SYSTEM_H

#include "stm32h5xx_hal.h"

void SystemClock_Config(void);
void MPU_Config(void);
void ICache_Init(void);
void DWT_Init(void);
void Error_Handler(void);

void DWT_Delay_us(uint32_t us);
inline uint32_t DWT_GetTick_us(void) {
    return DWT->CYCCNT;
}

#endif