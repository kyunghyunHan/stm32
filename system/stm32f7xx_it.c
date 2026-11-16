// STM32F7 HAL의 SysTick 시간 기준을 처리하는 코드
#include "stm32f7xx_hal.h"

/* Sole project owner of the HAL 1 ms time base. */
void SysTick_Handler(void)
{
    HAL_IncTick();
    HAL_SYSTICK_IRQHandler();
}
