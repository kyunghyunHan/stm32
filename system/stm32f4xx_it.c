#include "stm32f4xx_hal.h"

/* Time base for the F446 HAL command example. */
void SysTick_Handler(void)
{
    HAL_IncTick();
    HAL_SYSTICK_IRQHandler();
}
