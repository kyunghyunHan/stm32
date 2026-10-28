#include "stm32f7xx.h"
#include <stdint.h>

/*RCC
STM32에서 Clock과 Reset를 관리하는 하드웨어 주변장치
MCU내부의 GPIO, UART, SPI , TIMER등이 동작할수 있도록 클럭을 공급 ,제어

Clock Control
Reset Contrrol
*/
#define RCC_BASE 0x40023800UL



void delay_ms(uint32_t ms)
{
    // SysTick에 1ms에 해당하는 값 설정
    SystemCoreClockUpdate();
    SysTick->LOAD = SystemCoreClock / 1000U - 1U;

    // 현재 카운터 0으로 초기화
    SysTick->VAL = 0;

    // SysTick ON + CPU clock 사용
    SysTick->CTRL = (1 << 2) | (1 << 0);

    for (uint32_t i = 0; i < ms; i++)
    {
        // COUNTFLAG(bit 16)가 1이 될 때까지 기다림
        while ((SysTick->CTRL & (1 << 16)) == 0)
        {
        }
    }

    // SysTick OFF
    SysTick->CTRL = 0;
}

int main(void)
{
    // GPIOA 클럭 ON
    RCC->AHB1ENR |= (1 << 0);

    // PA5를 Output(01)으로 설정
    GPIOA->MODER &= ~(0x3 << 10);
    GPIOA->MODER |=  (0x1 << 10);

    while (1)
    {
        // PA5 상태 반전
        GPIOA->ODR ^= (1 << 5);

        // 3000ms = 3초
        delay_ms(3000);
    }
}
