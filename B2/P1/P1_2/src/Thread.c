#include "Thread.h"
#include "main.h"

osThreadId_t g_tid_manager = NULL;
osThreadId_t g_tid_joy = NULL;

static osTimerId_t g_timer_3s = NULL;
static volatile uint8_t g_system_locked = 0U;

static void Init_OutputPin(GPIO_TypeDef *port, uint16_t pin)
{
  GPIO_InitTypeDef gpio = {0};

  gpio.Pin = pin;
  gpio.Mode = GPIO_MODE_OUTPUT_PP;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;

  HAL_GPIO_Init(port, &gpio);
  HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
}

static void Init_JoystickPin(GPIO_TypeDef *port, uint16_t pin)
{
  GPIO_InitTypeDef gpio = {0};

  gpio.Pin = pin;
  gpio.Mode = GPIO_MODE_IT_RISING;
  gpio.Pull = GPIO_PULLDOWN;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;

  HAL_GPIO_Init(port, &gpio);
}

uint8_t System_IsLocked(void)
{
  return g_system_locked;
}

static void Timer3s_Callback(void *argument)
{
  (void)argument;

  g_system_locked = 1U;
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (g_system_locked != 0U) {
    return;
  }

  if ((GPIO_Pin == GPIO_PIN_11) && (g_tid_manager != NULL)) {
    osThreadFlagsSet(g_tid_manager, THREAD_RIGHT_FLAG);
  }

  if (((GPIO_Pin == GPIO_PIN_10) || (GPIO_Pin == GPIO_PIN_11) ||
       (GPIO_Pin == GPIO_PIN_12) || (GPIO_Pin == GPIO_PIN_14) ||
       (GPIO_Pin == GPIO_PIN_15)) && (g_tid_joy != NULL)) {
    osThreadFlagsSet(g_tid_joy, THREAD_JOYSTICK_FLAG);
  }
}

static void Manager_Thread(void *argument)
{
  (void)argument;

  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
  if (osTimerStart(g_timer_3s, 3000U) != osOK) {
    Error_Handler();
  }

  for (;;) {
    uint32_t flags = osThreadFlagsWait(THREAD_RIGHT_FLAG, osFlagsWaitAny, osWaitForever);

    if (((flags & osFlagsError) == 0U) && ((flags & THREAD_RIGHT_FLAG) != 0U) &&
        (g_system_locked == 0U)) {
      osTimerStop(g_timer_3s);
      if (osTimerStart(g_timer_3s, 3000U) != osOK) {
        Error_Handler();
      }
    }
  }
}

void Init_Threads(void)
{
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_SYSCFG_CLK_ENABLE();

  Init_OutputPin(GPIOB, GPIO_PIN_0);
  Init_OutputPin(GPIOB, GPIO_PIN_7);
  Init_OutputPin(GPIOB, GPIO_PIN_14);

  Init_JoystickPin(GPIOB, GPIO_PIN_10);
  Init_JoystickPin(GPIOB, GPIO_PIN_11);
  Init_JoystickPin(GPIOE, GPIO_PIN_12);
  Init_JoystickPin(GPIOE, GPIO_PIN_14);
  Init_JoystickPin(GPIOE, GPIO_PIN_15);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5U, 0U);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  g_timer_3s = osTimerNew(Timer3s_Callback, osTimerOnce, NULL, NULL);
  if (g_timer_3s == NULL) {
    Error_Handler();
  }

  g_tid_manager = osThreadNew(Manager_Thread, NULL, NULL);
  if (g_tid_manager == NULL) {
    Error_Handler();
  }

  g_tid_joy = osThreadNew(App_Joystick_Thread, NULL, NULL);
  if (g_tid_joy == NULL) {
    Error_Handler();
  }
}