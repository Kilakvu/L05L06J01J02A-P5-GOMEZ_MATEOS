#include "Thread.h"
#include "main.h"

void App_LED2_Thread(void *argument)
{
  (void)argument;

  for (;;) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
    osDelay(137U);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
    osDelay(137U);
  }
}
