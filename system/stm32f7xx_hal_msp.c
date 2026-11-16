// STM32F7 HAL에서 사용할 주변장치 클록과 ADC 핀을 초기화하는 코드
#include "stm32f7xx_hal.h"

void HAL_MspInit(void)
{
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_RCC_SYSCFG_CLK_ENABLE();
}

#ifdef PROJECT_HAL_ADC
void HAL_ADC_MspInit(ADC_HandleTypeDef *adc)
{
    if (adc->Instance == ADC1) {
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_ADC1_CLK_ENABLE();
        GPIO_InitTypeDef gpio = {0};
        gpio.Pin = GPIO_PIN_7;
        gpio.Mode = GPIO_MODE_ANALOG;
        gpio.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &gpio);
    }
}
#endif
