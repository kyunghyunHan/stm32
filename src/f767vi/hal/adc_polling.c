#include "stm32f7xx_hal.h"

volatile uint16_t adc_raw;
volatile float throttle_V;

static void check(HAL_StatusTypeDef status)
{
    if (status != HAL_OK) {
        __disable_irq();
        while (1) {}
    }
}

int main(void)
{
    check(HAL_Init());
    /* HSI 16 MHz, APB2 / 2 = 8 MHz ADC clock; polling, no DMA/ADC IRQ. */
    ADC_HandleTypeDef adc = {0};
    adc.Instance = ADC1;
    adc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
    adc.Init.Resolution = ADC_RESOLUTION_12B;
    adc.Init.ScanConvMode = DISABLE;
    adc.Init.ContinuousConvMode = DISABLE;
    adc.Init.DiscontinuousConvMode = DISABLE;
    adc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    adc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    adc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    adc.Init.NbrOfConversion = 1;
    adc.Init.DMAContinuousRequests = DISABLE;
    adc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    check(HAL_ADC_Init(&adc));
    ADC_ChannelConfTypeDef channel = {0};
    channel.Channel = ADC_CHANNEL_7;
    channel.Rank = ADC_REGULAR_RANK_1;
    channel.SamplingTime = ADC_SAMPLETIME_56CYCLES;
    check(HAL_ADC_ConfigChannel(&adc, &channel));
    while (1) {
        check(HAL_ADC_Start(&adc));
        check(HAL_ADC_PollForConversion(&adc, 100));
        adc_raw = (uint16_t)HAL_ADC_GetValue(&adc);
        throttle_V = (adc_raw * 3.3f) / 4095.0f;
        check(HAL_ADC_Stop(&adc));
    }
}
