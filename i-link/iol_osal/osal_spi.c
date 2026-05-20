#include "osal_spi.h"
#include "../../Core/Src/SPI/spi.h"
#include "../../Core/Src/GPIO/gpio.h"
#include "stm32h563xx.h"

#define MAX14819_CS_PORT    GPIOA
#define MAX14819_CS_PIN     GPIO_PIN_4

void * _iolink_pl_hw_spi_init (const char * spi_slave_name) {
    GPIO_InitPin(MAX14819_CS_PORT, MAX14819_CS_PIN, GPIO_MODE_OUTPUT_PP, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_NOPULL);
    SPI_Init(SPI_MODE_MASTER, SPI_DATASIZE_8BIT, SPI_BAUDRATEPRESCALER_32, SPI_DMA_NONE);

    return (void*)SPI_GetHandle();
}

void _iolink_pl_hw_spi_transfer (void* fd, void* data_read, const void* data_written, size_t n_bytes_to_transfer) {
    HAL_GPIO_WritePin(MAX14819_CS_PORT, MAX14819_CS_PIN, GPIO_PIN_RESET);

    SPI_TransmitReceive(data_written, data_read, n_bytes_to_transfer);

    HAL_GPIO_WritePin(MAX14819_CS_PORT, MAX14819_CS_PIN, GPIO_PIN_SET);
}

void _iolink_pl_hw_spi_close (void * fd) {
    HAL_SPI_DeInit(SPI_GetHandle());
}
