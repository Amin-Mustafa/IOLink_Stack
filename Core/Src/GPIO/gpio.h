#ifndef GPIO_H
#define GPIO_H

#include "stm32h5xx_hal.h"

/**
 * @brief Enable clock for a GPIO port
 * 
 * @param port
 * 
 * Example
 * @code
 * GPIO_EnableClock(GPIOA);
 * @endcode
 */
void GPIO_EnableClock(GPIO_TypeDef* port);

/**
 * @brief Disable the clock for a GPIO port
 * 
 * @param port 
 * 
 * Example
 * @code
 * GPIO_DisableClock(GPIOA);
 * @endcode
 */
void GPIO_DisableClock(GPIO_TypeDef* port);

/**
 * @brief Initialize a GPIO port for input/output
 * 
 * @param port Port to be initialized (e.g., GPIOA)
 * @param pin Pin(s) to be initialized (e.g., GPIO_PIN_10)
 * @param mode Input/output mode (e.g., GPIO_MODE_OUTPUT_PP). 
 * Refer to \ref stm32h5xx_hal_gpio.h for the mode macros
 * @param speed GPIO switching speed (e.g., GPIO_SPEED_FREQ_LOW).
 * Refer to \ref stm32h5xx_hal_gpio.h for the speed macros
 * @param pupd Internal pull-up or pull-down resistors. Use @code GPIO_PULLUP @endcode 
 * for pull-up, @code GPIO_PULLDOWN @endcode for pull-down, 
 * and @code GPIO_NOPULL @endcode for no pull  
 */
void GPIO_InitPin(GPIO_TypeDef* port, uint16_t pin, uint32_t mode, uint32_t speed, uint32_t pupd);

#endif