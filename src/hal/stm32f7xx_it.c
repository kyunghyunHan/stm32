#include "stm32f7xx_hal.h"

/* Sole project owner of the HAL 1 ms time base. */
void SysTick_Handler(void)
{
    HAL_IncTick();
    HAL_SYSTICK_IRQHandler();
}
