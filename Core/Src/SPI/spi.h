#ifndef SPI_H
#define SPI_H

#include "stm32h5xx_hal.h"

typedef enum {
    SPI_DMA_NONE = 0,
    SPI_DMA_TX = 1 << 0,
    SPI_DMA_RX = 1 << 1,
    SPI_DMA_TX_RX = SPI_DMA_TX | SPI_DMA_RX
} SPI_DMA_t;

void SPI_Init(uint32_t mode, uint32_t data_size, uint32_t baudrate_psc, SPI_DMA_t dma_line);

uint8_t SPI_Transmit(void* data, uint16_t len);
uint16_t SPI_Receive(void* data, uint16_t len);
uint16_t SPI_TransmitReceive(void* tx_data, void* rx_data, uint16_t len);

void SPI_Select(GPIO_TypeDef* port, uint16_t pin);
void SPI_Release(GPIO_TypeDef* port, uint16_t pin);

SPI_HandleTypeDef* SPI_GetHandle();

#endif