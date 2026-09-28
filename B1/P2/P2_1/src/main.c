#include "main.h"

#define TIM7_PRESCALER 8399U
#define TIM7_PERIOD 14999U

TIM_HandleTypeDef htim7;
volatile uint32_t tim7_interrupt_count;

static void SystemClock_Config(void);
static void GPIO_Init(void);
static void TIM7_Init(void);
static void Error_Handler(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    SystemCoreClockUpdate();
    GPIO_Init();
    TIM7_Init();

    while (1)
    {
    }
}

static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef osc_config = {0};
    RCC_ClkInitTypeDef clock_config = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    osc_config.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    osc_config.HSEState = RCC_HSE_ON;
    osc_config.PLL.PLLState = RCC_PLL_ON;
    osc_config.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    osc_config.PLL.PLLM = 8U;
    osc_config.PLL.PLLN = 336U;
    osc_config.PLL.PLLP = RCC_PLLP_DIV2;
    osc_config.PLL.PLLQ = 7U;

    if (HAL_RCC_OscConfig(&osc_config) != HAL_OK)
    {
        Error_Handler();
    }

    clock_config.ClockType = RCC_CLOCKTYPE_SYSCLK |
                             RCC_CLOCKTYPE_HCLK |
                             RCC_CLOCKTYPE_PCLK1 |
                             RCC_CLOCKTYPE_PCLK2;
    clock_config.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clock_config.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clock_config.APB1CLKDivider = RCC_HCLK_DIV4;
    clock_config.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&clock_config, FLASH_LATENCY_5) != HAL_OK)
    {
        Error_Handler();
    }
}

static void GPIO_Init(void)
{
    GPIO_InitTypeDef gpio_config = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();
    gpio_config.Pin = GPIO_PIN_0;
    gpio_config.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_config.Pull = GPIO_NOPULL;
    gpio_config.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &gpio_config);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
}

static void TIM7_Init(void)
{
    __HAL_RCC_TIM7_CLK_ENABLE();

    htim7.Instance = TIM7;
    htim7.Init.Prescaler = TIM7_PRESCALER;
    htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim7.Init.Period = TIM7_PERIOD;
    htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
    {
        Error_Handler();
    }

    HAL_NVIC_SetPriority(TIM7_IRQn, 2U, 0U);
    HAL_NVIC_EnableIRQ(TIM7_IRQn);

    if (HAL_TIM_Base_Start_IT(&htim7) != HAL_OK)
    {
        Error_Handler();
    }
}

void TIM7_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim7);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM7)
    {
        tim7_interrupt_count++;
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
    }
}

static void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}