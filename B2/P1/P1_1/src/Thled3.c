#include "Thread.h"
#include "main.h"

void App_LED3_Thread(void *argument)
{
  (void)argument;

  for (;;) {
    osThreadFlagsWait(THREAD_LED3_FLAG, osFlagsWaitAny, osWaitForever);

    for (uint32_t i = 0U; i < 30U; i++) {
      HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14);
      osDelay(287U);
    }

    osThreadFlagsSet(g_tid_led2, THREAD_LED2_FLAG);
  }
}
