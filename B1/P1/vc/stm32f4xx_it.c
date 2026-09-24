#include "main.h"
#include "stm32f4xx_it.h"

void EXTI15_10_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
}

void SysTick_Handler(void)
{
    HAL_IncTick();
}
