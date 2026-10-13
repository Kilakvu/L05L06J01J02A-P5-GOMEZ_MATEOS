#include "Thread.h"
#include "main.h"

osThreadId_t g_tid_led1 = NULL;
osThreadId_t g_tid_led2 = NULL;
osThreadId_t g_tid_led3 = NULL;
static uint32_t led1_cycles = 0U;

static void Init_LedPin(GPIO_TypeDef *port, uint16_t pin)
{
  GPIO_InitTypeDef gpio = {0};

  gpio.Pin = pin;
  gpio.Mode = GPIO_MODE_OUTPUT_PP;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;

  HAL_GPIO_Init(port, &gpio);
  HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
}

void App_LED1_Thread(void *argument)
{
  (void)argument;

  for (;;) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
    osDelay(200U);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
    osDelay(800U);

    led1_cycles++;
    if ((led1_cycles % 5U) == 0U) {
      osThreadFlagsSet(g_tid_led2, THREAD_LED2_FLAG);
    }
  }
}

void Init_Threads(void)
{
  __HAL_RCC_GPIOB_CLK_ENABLE();
  Init_LedPin(GPIOB, GPIO_PIN_0);
  Init_LedPin(GPIOB, GPIO_PIN_7);
  Init_LedPin(GPIOB, GPIO_PIN_14);

  g_tid_led1 = osThreadNew(App_LED1_Thread, NULL, NULL);
  g_tid_led2 = osThreadNew(App_LED2_Thread, NULL, NULL);
  g_tid_led3 = osThreadNew(App_LED3_Thread, NULL, NULL);

  if ((g_tid_led1 == NULL) || (g_tid_led2 == NULL) || (g_tid_led3 == NULL)) {
    Error_Handler();
  }
}
