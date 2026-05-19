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

uint8_t SPI_Transmit(GPIO_TypeDef* cs_port, uint16_t cs_pin, void* data, uint16_t len);
uint16_t SPI_Receive(GPIO_TypeDef* cs_port, uint16_t cs_pin, void* data, uint16_t len);
uint16_t SPI_TransmitReceive(GPIO_TypeDef* cs_port, uint16_t cs_pin, void* tx_data, void* rx_data, uint16_t len);

SPI_HandleTypeDef* SPI_GetHandle();

#endif