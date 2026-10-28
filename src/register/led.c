#include "stm32f7xx.h"
#include <stdint.h>

/*RCC
STM32에서 Clock과 Reset를 관리하는 하드웨어 주변장치
MCU내부의 GPIO, UART, SPI , TIMER등이 동작할수 있도록 클럭을 공급 ,제어

Clock Control
Reset Contrrol
*/
#define RV_RCC_BASE 0x40023800UL

/*
RCC
- Reset and Clock Control
- STM32내부의 주변장치에 클럭을 공급하고 리셋을 제어하는 하드웨어
- GPIOA, GPIOB, GPIOC , UART , SPI, I2C , Timer
- 주변장치들이 필요한 클럭이 공급되어야 정상적으로 동작
*/

/*
AHB1ENR
AHB1 :Advenced High-performance Bus1
EN :Enable(활성화)
R :Register
- AHB1버스에 연결된 주변장치의 클럭을활성화 하는 레지스터

EN
0:해당 주변장치 클럭 비활성화
1:해당 주변장치 클럭 활성화

RCC_BASE       = 0x40023800
AHB1ENR Offset = 0x00000030
--------------------------------
RCC_AHB1ENR    = 0x40023830

*/
#define RCC_AHB1ENR (*(volatile uint32_t *)(RV_RCC_BASE + 0x30UL))

/*GPIOA 레지스터 */

#define RV_GPIOA_BASE 0x40020000UL
// GPIOA MODER offset = 0x00
#define GPIOA_MODER (*(volatile uint32_t *)(RV_GPIOA_BASE + 0x00UL))

// GPIOA ODR Offset  =0x14
#define GPIOA_ODR (*(volatile uint32_t *)(RV_GPIOA_BASE + 0x14UL))

/*SysTick 레지스터
ARM Cortex-M코어에 내장된 24비트 카운트 다운 타이머
dalay_ms 밀리초 단위 시간 지연
RTOS :작업 스케줄리응ㄹ 위한 주기적인 Tick
주기적인 인터럽트 발생
*/

#define SYSTICK_BASE 0xE000E010UL

#define SYSTICK_CTRL (*(volatile uint32_t *)(SYSTICK_BASE + 0x00UL))
#define SYSTICK_LOAD (*(volatile uint32_t *)(SYSTICK_BASE + 0x04UL))
#define SYSTICK_VAL (*(volatile uint32_t *)(SYSTICK_BASE + 0x08UL))

// 16MNz가정
#define CPU_CLOCK_HZ 16000000UL
void delay_ms(uint32_t ms)
{
    // SysTick에 1ms에 해당하는 값 설정
    SystemCoreClockUpdate();
    // 카운터 시작
    SysTick->LOAD = SystemCoreClock / 1000U - 1U;
    SYSTICK_LOAD = (CPU_CLOCK_HZ / 1000u) - 1U; // RV
    // 현재카운터를 초기화한다
    //  현재 카운터 0으로 초기화
    /*
    LOAD = 15999
    VAL  = 0       ← 초기화

    SysTick 활성화

    VAL = 15999
    VAL = 15998
    VAL = 15997
     ...
    VAL = 1
     VAL = 0       ← 카운트 완료

     다시 15999부터 반복
    */
    SysTick->VAL = 0;
    SYSTICK_VAL = 0; // RV

    // SysTick ON + CPU clock 사용
    SysTick->CTRL = (1 << 2) | (1 << 0);
    SYSTICK_CTRL = (1U << 2) | (1U << 0); // RV

    for (uint32_t i = 0; i < ms; i++)
    {
        // COUNTFLAG(bit 16)가 1이 될 때까지 기다림
        while ((SysTick->CTRL & (1 << 16)) == 0)
        {
        }

        // RV
        while ((SYSTICK_CTRL & (1U << 16)) == 0U)
        {
        }
    }

    // SysTick OFF
    SysTick->CTRL = 0;
    SYSTICK_CTRL = 0;
}

int main(void)
{
    // GPIOA 클럭 ON
    RCC->AHB1ENR |= (1 << 0);
    RCC_AHB1ENR |= (1U << 0); // RV

    // PA5를 Output(01)으로 설정
    GPIOA->MODER &= ~(0x3 << 10);
    GPIOA_MODER &= ~(0x3U << 10); // RV

    GPIOA->MODER |= (0x1 << 10);
    GPIOA_MODER |= (0x1U << 10); // RV

    while (1)
    {
        // PA5 상태 반전
        GPIOA->ODR ^= (1 << 5);
        GPIOA_ODR ^= (1U << 5); // RV

        // 3000ms = 3초
        delay_ms(3000);
    }
}
