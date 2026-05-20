#include "osal_irq.h"
#include "GPIO/gpio.h"
#include "System/priorities.h"
#include <stddef.h>

#define MAX14819_IRQ_PORT   GPIOA
#define MAX14819_IRQ_PIN    GPIO_PIN_3

static void *max14819_isr_arg = NULL;
static void (*max14819_isr)(void *arg) = NULL;

int _iolink_setup_int (int gpio_pin, isr_func_t isr_func, void * arg) {
    max14819_isr_arg = arg;
    max14819_isr = isr_func;

    GPIO_InitPin(MAX14819_IRQ_PORT, MAX14819_IRQ_PIN, GPIO_MODE_IT_FALLING, 0, GPIO_PULLUP);

    HAL_NVIC_SetPriority(EXTI3_IRQn, MAX14819_IRQ_PRIORITY, 0); 
    HAL_NVIC_EnableIRQ(EXTI3_IRQn);
}

void EXTI3_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(MAX14819_IRQ_PIN);
    
    if (max14819_isr != NULL) {
        rtlabs_max14819_isr(max14819_isr_arg);
    }
}