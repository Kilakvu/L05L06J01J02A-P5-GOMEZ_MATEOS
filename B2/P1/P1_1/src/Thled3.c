#include "Thread.h"
#include "main.h"

void App_LED3_Thread(void *argument)
{
  (void)argument;

  for (;;) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
    osDelay(287U);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
    osDelay(287U);
  }
}
