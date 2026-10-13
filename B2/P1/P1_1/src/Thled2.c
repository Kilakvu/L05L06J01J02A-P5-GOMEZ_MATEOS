#include "Thread.h"
#include "main.h"

void App_LED2_Thread(void *argument)
{
  (void)argument;

  for (;;) {
    osThreadFlagsWait(THREAD_LED2_FLAG, osFlagsWaitAny, osWaitForever);

    for (uint32_t i = 0U; i < 20U; i++) {
      HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);
      osDelay(137U);
    }

    osThreadFlagsSet(g_tid_led3, THREAD_LED3_FLAG);
  }
}
