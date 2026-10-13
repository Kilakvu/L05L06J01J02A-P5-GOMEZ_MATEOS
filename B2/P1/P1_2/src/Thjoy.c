#include "Thread.h"
#include "main.h"

static osTimerId_t g_debounce_timer = NULL;

static void UpdateGestureLeds(uint8_t gesture)
{
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, (gesture & 0x01U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, (gesture & 0x02U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, (gesture & 0x04U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void DebounceTimerCallback(void *argument)
{
  (void)argument;

  if (System_IsLocked() != 0U) {
    return;
  }

  uint8_t gesture = 0U;

  if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10) == GPIO_PIN_SET) {
    gesture = 1U;
  } else if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11) == GPIO_PIN_SET) {
    gesture = 2U;
  } else if (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_12) == GPIO_PIN_SET) {
    gesture = 3U;
  } else if (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_14) == GPIO_PIN_SET) {
    gesture = 4U;
  } else if (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_15) == GPIO_PIN_SET) {
    gesture = 5U;
  }

  if (gesture != 0U) {
    UpdateGestureLeds(gesture);
  }
}

void App_Joystick_Thread(void *argument)
{
  (void)argument;

  g_debounce_timer = osTimerNew(DebounceTimerCallback, osTimerOnce, NULL, NULL);
  if (g_debounce_timer == NULL) {
    Error_Handler();
  }

  for (;;) {
    uint32_t flags = osThreadFlagsWait(THREAD_JOYSTICK_FLAG, osFlagsWaitAny, osWaitForever);

    if (((flags & osFlagsError) == 0U) && ((flags & THREAD_JOYSTICK_FLAG) != 0U)) {
      if ((osTimerIsRunning(g_debounce_timer) != 0U) &&
          (osTimerStop(g_debounce_timer) != osOK)) {
        Error_Handler();
      }
      if (osTimerStart(g_debounce_timer, 50U) != osOK) {
        Error_Handler();
      }
    }
  }
}