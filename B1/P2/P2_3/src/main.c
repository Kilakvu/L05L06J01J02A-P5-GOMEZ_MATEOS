#include "main.h"

#define ENABLE_JOYSTICK 0
#define TIM2_COUNTER_HZ 1000000U
#define TIM2_PRESCALER 83U
#define INITIAL_FREQUENCY_HZ 1000U
#define MIN_FREQUENCY_HZ 1U
#define MAX_FREQUENCY_HZ 16000U

TIM_HandleTypeDef htim2;
volatile uint32_t current_frequency_hz = INITIAL_FREQUENCY_HZ;

#if ENABLE_JOYSTICK
static ADC_HandleTypeDef hadc1;
#endif

static void SystemClock_Config(void);
static void GPIO_Init(void);
static void TIM2_Init(void);
static void Set_Output_Frequency(uint32_t frequency_hz);
static void Error_Handler(void);

#if ENABLE_JOYSTICK
static void ADC1_Init(void);
static void Joystick_Service(void);
#endif

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    SystemCoreClockUpdate();
    GPIO_Init();
    TIM2_Init();

#if ENABLE_JOYSTICK
    ADC1_Init();
#endif

    while (1)
    {
#if ENABLE_JOYSTICK
        Joystick_Service();
#endif
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
    __HAL_RCC_GPIOC_CLK_ENABLE();

    gpio_config.Pin = GPIO_PIN_11;
    gpio_config.Mode = GPIO_MODE_AF_PP;
    gpio_config.Pull = GPIO_NOPULL;
    gpio_config.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio_config.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init(GPIOB, &gpio_config);

    gpio_config.Pin = GPIO_PIN_13;
    gpio_config.Mode = GPIO_MODE_IT_RISING;
    gpio_config.Pull = GPIO_PULLDOWN;
    gpio_config.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &gpio_config);

    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 2U, 0U);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

static void TIM2_Init(void)
{
    TIM_OC_InitTypeDef output_compare = {0};

    __HAL_RCC_TIM2_CLK_ENABLE();
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = TIM2_PRESCALER;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = (TIM2_COUNTER_HZ / (2U * INITIAL_FREQUENCY_HZ)) - 1U;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_OC_Init(&htim2) != HAL_OK)
    {
        Error_Handler();
    }

    output_compare.OCMode = TIM_OCMODE_TOGGLE;
    output_compare.Pulse = htim2.Init.Period;
    output_compare.OCPolarity = TIM_OCPOLARITY_HIGH;
    output_compare.OCFastMode = TIM_OCFAST_DISABLE;

    if (HAL_TIM_OC_ConfigChannel(&htim2, &output_compare, TIM_CHANNEL_4) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_TIM_OC_Start(&htim2, TIM_CHANNEL_4) != HAL_OK)
    {
        Error_Handler();
    }
}

static void Set_Output_Frequency(uint32_t frequency_hz)
{
    uint32_t counts_per_half_period = TIM2_COUNTER_HZ / (2U * frequency_hz);

    if ((counts_per_half_period < 2U) || (frequency_hz > MAX_FREQUENCY_HZ))
    {
        return;
    }

    (void)HAL_TIM_OC_Stop(&htim2, TIM_CHANNEL_4);
    __HAL_TIM_SET_AUTORELOAD(&htim2, counts_per_half_period - 1U);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, counts_per_half_period - 1U);
    __HAL_TIM_SET_COUNTER(&htim2, 0U);
    if (HAL_TIM_GenerateEvent(&htim2, TIM_EVENTSOURCE_UPDATE) != HAL_OK)
    {
        Error_Handler();
    }
    __HAL_TIM_SET_COUNTER(&htim2, 0U);
    (void)HAL_TIM_OC_Start(&htim2, TIM_CHANNEL_4);
    current_frequency_hz = frequency_hz;
}

void EXTI15_10_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
}

void HAL_GPIO_EXTI_Callback(uint16_t gpio_pin)
{
    static uint32_t last_press_tick;
    const uint32_t now = HAL_GetTick();

    if ((gpio_pin == GPIO_PIN_13) && ((now - last_press_tick) >= 200U))
    {
        last_press_tick = now;
        Set_Output_Frequency((current_frequency_hz == 1000U) ? 2500U : 1000U);
    }
}

#if ENABLE_JOYSTICK
static void ADC1_Init(void)
{
    ADC_ChannelConfTypeDef channel_config = {0};
    GPIO_InitTypeDef gpio_config = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_ADC1_CLK_ENABLE();

    gpio_config.Pin = GPIO_PIN_1;
    gpio_config.Mode = GPIO_MODE_ANALOG;
    gpio_config.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &gpio_config);

    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.ScanConvMode = DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 1U;
    hadc1.Init.DMAContinuousRequests = DISABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;

    if (HAL_ADC_Init(&hadc1) != HAL_OK)
    {
        Error_Handler();
    }

    channel_config.Channel = ADC_CHANNEL_1;
    channel_config.Rank = 1U;
    channel_config.SamplingTime = ADC_SAMPLETIME_84CYCLES;
    if (HAL_ADC_ConfigChannel(&hadc1, &channel_config) != HAL_OK)
    {
        Error_Handler();
    }
}

static void Joystick_Service(void)
{
    static uint8_t gesture_latched;
    uint32_t adc_value;

    if (HAL_ADC_Start(&hadc1) != HAL_OK)
    {
        return;
    }
    if (HAL_ADC_PollForConversion(&hadc1, 10U) != HAL_OK)
    {
        (void)HAL_ADC_Stop(&hadc1);
        return;
    }

    adc_value = HAL_ADC_GetValue(&hadc1);
    (void)HAL_ADC_Stop(&hadc1);

    if ((adc_value < 1000U) && (gesture_latched == 0U))
    {
        if (current_frequency_hz <= (MAX_FREQUENCY_HZ / 2U))
        {
            Set_Output_Frequency(current_frequency_hz * 2U);
        }
        gesture_latched = 1U;
    }
    else if ((adc_value > 3000U) && (gesture_latched == 0U))
    {
        if (current_frequency_hz >= (MIN_FREQUENCY_HZ * 2U))
        {
            Set_Output_Frequency(current_frequency_hz / 2U);
        }
        gesture_latched = 1U;
    }
    else if ((adc_value >= 1500U) && (adc_value <= 2600U))
    {
        gesture_latched = 0U;
    }
}
#endif

static void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}