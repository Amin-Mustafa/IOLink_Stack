#include "../GPIO/gpio.h"
#include "../System/system.h"
#include "../System/priorities.h"

typedef struct {
    GPIO_TypeDef* port;
    uint16_t pin;
    uint32_t alternate;
} SPI_MSP_GPIO_t;

typedef struct {
    int irqn;
    SPI_MSP_GPIO_t gpio1, gpio2;
} SPI_MSP_Context_t;

static SPI_MSP_GPIO_t spi3_mosi = {
    .port = GPIOB,
    .pin  = GPIO_PIN_2,
    .alternate = GPIO_AF7_SPI3
};

static SPI_MSP_GPIO_t spi3_sck_miso = {
    .port = GPIOC,
    .pin  = GPIO_PIN_10 | GPIO_PIN_11,
    .alternate = GPIO_AF6_SPI3
};

static void init_spi_gpio(SPI_MSP_GPIO_t gpio) {
    if(!gpio.port) return;

    GPIO_EnableClock(gpio.port);

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = gpio.pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStruct.Alternate = gpio.alternate;
    HAL_GPIO_Init(gpio.port, &GPIO_InitStruct);
}

static void deinit_spi_gpio(SPI_MSP_GPIO_t gpio) {
    if(!gpio.port) return;

    HAL_GPIO_DeInit(gpio.port, gpio.pin);
}

void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi) {
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    /** Initializes the peripherals clock
     */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPI3;
    PeriphClkInitStruct.Spi3ClockSelection = RCC_SPI3CLKSOURCE_PLL1Q;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /* Peripheral clock enable */
    __HAL_RCC_SPI3_CLK_ENABLE();

    /**SPI3 GPIO Configuration
    PB2     ------> SPI3_MOSI
    PC10     ------> SPI3_SCK
    PC11     ------> SPI3_MISO
    */
    init_spi_gpio(spi3_mosi);
    init_spi_gpio(spi3_sck_miso);

    /* SPI3 interrupt Init */
    HAL_NVIC_SetPriority(SPI3_IRQn, SPI3_IRQ_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(SPI3_IRQn);

}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi) {
    //disable clk
    __HAL_RCC_SPI3_CLK_DISABLE();

    //deinit gpio
    deinit_spi_gpio(spi3_mosi);
    deinit_spi_gpio(spi3_sck_miso);

    //deinit dma
    HAL_DMA_DeInit(hspi->hdmarx);
    HAL_DMA_DeInit(hspi->hdmatx);
    
    //disable irq
    HAL_NVIC_DisableIRQ(SPI3_IRQn);
}