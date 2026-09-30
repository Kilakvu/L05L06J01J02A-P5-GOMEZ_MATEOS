#include "main.h"

#define USE_MBED_RGB 0

static volatile uint32_t frequency_mode = 0U;

static void SystemClock_Config(void);
static void GPIO_Init(void);
static void Update_Leds(void);
static void Error_Handler(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    SystemCoreClockUpdate();
    GPIO_Init();

    while (1)
    {
        Update_Leds();
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
    osc_config.PLL.PLLN = 192U;
    osc_config.PLL.PLLP = RCC_PLLP_DIV2;
    osc_config.PLL.PLLQ = 4U;

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

    if (HAL_RCC_ClockConfig(&clock_config, FLASH_LATENCY_3) != HAL_OK)
    {
        Error_Handler();
    }
}

static void GPIO_Init(void)
{
    GPIO_InitTypeDef gpio_config = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
#if USE_MBED_RGB
    __HAL_RCC_GPIOD_CLK_ENABLE();
#endif

#if USE_MBED_RGB
    gpio_config.Pin = GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13;
    gpio_config.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_config.Pull = GPIO_NOPULL;
    gpio_config.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOD, &gpio_config);
#else
    gpio_config.Pin = GPIO_PIN_0 | GPIO_PIN_7 | GPIO_PIN_14;
    gpio_config.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_config.Pull = GPIO_NOPULL;
    gpio_config.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &gpio_config);
#endif

    gpio_config.Pin = GPIO_PIN_13;
    gpio_config.Mode = GPIO_MODE_IT_RISING;
    gpio_config.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &gpio_config);

    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 2U, 0U);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

static void Update_Leds(void)
{
    static uint32_t last_ld1 = 0U;
    static uint32_t last_ld2 = 0U;
    static uint32_t last_ld3 = 0U;
    const uint32_t now = HAL_GetTick();
    uint32_t period_ld1;
    uint32_t period_ld2;
    uint32_t period_ld3;

    if (frequency_mode == 0U)
    {
        period_ld1 = 500U;
        period_ld2 = 1000U;
        period_ld3 = 2000U;
    }
    else if (frequency_mode == 1U)
    {
        period_ld1 = 250U;
        period_ld2 = 500U;
        period_ld3 = 1000U;
    }
    else
    {
        period_ld1 = 125U;
        period_ld2 = 250U;
        period_ld3 = 500U;
    }

    if ((now - last_ld1) >= period_ld1)
    {
        last_ld1 = now;
#if USE_MBED_RGB
        HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
#else
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
#endif
    }

    if ((now - last_ld2) >= period_ld2)
    {
        last_ld2 = now;
#if USE_MBED_RGB
        HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);
#else
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);
#endif
    }

    if ((now - last_ld3) >= period_ld3)
    {
        last_ld3 = now;
#if USE_MBED_RGB
        HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_11);
#else
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14);
#endif
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t gpio_pin)
{
    if (gpio_pin == GPIO_PIN_13)
    {
        frequency_mode++;
        if (frequency_mode >= 3U)
        {
            frequency_mode = 0U;
        }
    }
}

static void Error_Handler(void)
{
    while (1)
    {
    }
}
