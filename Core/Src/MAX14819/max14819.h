#ifndef MAX14819_H
#define MAX14819_H

#include "stm32h5xx_hal.h"  

#define MAX14819_CS_PORT    GPIOA
#define MAX14819_CS_PIN     GPIO_PIN_4
#define MAX14819_IRQ_PORT   GPIOA
#define MAX14819_IRQ_PIN    GPIO_PIN_3
#define MAX14819_REVID      0x0A
#define MAX14819A_REVID     0x0E

typedef enum {
    MAX14819_PORT_A = 0, MAX14819_PORT_B = 1
} MAX14819_Port_t;

typedef enum {
    MAX14819_CLIM_100mA = 0x00,
    MAX14819_CLIM_200mA = 0x01,
    MAX14819_CLIM_300mA = 0x10, 
    MAX14819_CLIM_500mA = 0x11
} MAX14819_CurrentLimit_t;

uint8_t MAX14819_Init();

uint8_t MAX14819_ReadRegister(MAX14819_Register_t reg);
uint8_t MAX14819_ReadPortReg(MAX14819_Register_t reg, MAX14819_Port_t port);
void  MAX14819_WriteRegister(MAX14819_Register_t reg, uint8_t val);
void  MAX14819_WritePortReg(MAX14819_Register_t reg, MAX14819_Port_t port, uint8_t val);

void MAX14819_PowerOnPort(MAX14819_Port_t port);
void MAX14819_PowerOffPort(MAX14819_Port_t port);

void MAX14819_ActivatePort(MAX14819_Port_t port, uint32_t boot_time, MAX14819_CurrentLimit_t clim);

#endif  //MAX14819_H