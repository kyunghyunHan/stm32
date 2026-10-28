#ifndef PROJECT_STM32F7XX_HAL_CONF_H
#define PROJECT_STM32F7XX_HAL_CONF_H
#define HAL_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_PWR_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED
#ifdef PROJECT_HAL_ADC
#define HAL_ADC_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#endif
#ifndef HSE_VALUE
#define HSE_VALUE 25000000U
#endif
#define HSE_STARTUP_TIMEOUT 100U
#define HSI_VALUE 16000000U
#define LSI_VALUE 32000U
#define LSE_VALUE 32768U
#define LSE_STARTUP_TIMEOUT 5000U
#define EXTERNAL_CLOCK_VALUE 12288000U
#define VDD_VALUE 3300U
#define TICK_INT_PRIORITY 15U
#define USE_RTOS 0U
#define PREFETCH_ENABLE 1U
#define ART_ACCELERATOR_ENABLE 1U
#define USE_HAL_ADC_REGISTER_CALLBACKS 0U
#include "stm32f7xx_hal_rcc.h"
#include "stm32f7xx_hal_gpio.h"
#include "stm32f7xx_hal_cortex.h"
#include "stm32f7xx_hal_flash.h"
#include "stm32f7xx_hal_pwr.h"
#ifdef PROJECT_HAL_ADC
#include "stm32f7xx_hal_dma.h"
#include "stm32f7xx_hal_adc.h"
#endif
#define assert_param(expr) ((void)0U)
#endif
