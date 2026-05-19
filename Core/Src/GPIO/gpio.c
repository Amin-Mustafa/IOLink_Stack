#include "gpio.h"

void GPIO_EnableClock(GPIO_TypeDef* port) {
    switch((uint32_t)port) {
        case GPIOA_BASE: __HAL_RCC_GPIOA_CLK_ENABLE();  break;
        case GPIOB_BASE: __HAL_RCC_GPIOB_CLK_ENABLE();  break;
        case GPIOC_BASE: __HAL_RCC_GPIOC_CLK_ENABLE();  break;
        case GPIOD_BASE: __HAL_RCC_GPIOD_CLK_ENABLE();  break;
        case GPIOE_BASE: __HAL_RCC_GPIOE_CLK_ENABLE();  break;
        case GPIOF_BASE: __HAL_RCC_GPIOF_CLK_ENABLE();  break;
        case GPIOG_BASE: __HAL_RCC_GPIOG_CLK_ENABLE();  break;
        case GPIOH_BASE: __HAL_RCC_GPIOH_CLK_ENABLE();  break;
        case GPIOI_BASE: __HAL_RCC_GPIOI_CLK_ENABLE();  break;
        default: break;
    }
}

void GPIO_DisableClock(GPIO_TypeDef* port) {
    switch((uint32_t)port) {
        case GPIOA_BASE: __HAL_RCC_GPIOA_CLK_DISABLE();  break;
        case GPIOB_BASE: __HAL_RCC_GPIOB_CLK_DISABLE();  break;
        case GPIOC_BASE: __HAL_RCC_GPIOC_CLK_DISABLE();  break;
        case GPIOD_BASE: __HAL_RCC_GPIOD_CLK_DISABLE();  break;
        case GPIOE_BASE: __HAL_RCC_GPIOE_CLK_DISABLE();  break;
        case GPIOF_BASE: __HAL_RCC_GPIOF_CLK_DISABLE();  break;
        case GPIOG_BASE: __HAL_RCC_GPIOG_CLK_DISABLE();  break;
        case GPIOH_BASE: __HAL_RCC_GPIOH_CLK_DISABLE();  break;
        case GPIOI_BASE: __HAL_RCC_GPIOI_CLK_DISABLE();  break;
        default: break;
    }
}

void GPIO_InitPin(GPIO_TypeDef* port, uint16_t pin, uint32_t mode, uint32_t speed, uint32_t pupd) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_EnableClock(port);

    /*Configure GPIO pin */
    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Mode = mode;
    GPIO_InitStruct.Pull = pupd;
    GPIO_InitStruct.Speed = speed;
    HAL_GPIO_Init(port, &GPIO_InitStruct);
}
