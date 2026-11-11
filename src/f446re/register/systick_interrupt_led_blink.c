
#include <stdint.h>

/*
 * STM32F446RE
 * PA5 : LD2 LED
 * CPU Clock : 16MHz
 * SysTick : 1ms Interrupt
 * LED : 3초마다 Toggle
 */

// RCC
#define RCC_BASE 0x40023800UL
#define RCC_AHB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x30UL))

// GPIOA
#define GPIOA_BASE 0x40020000UL
#define GPIOA_MODER (*(volatile uint32_t *)(GPIOA_BASE + 0x00UL))
#define GPIOA_ODR (*(volatile uint32_t *)(GPIOA_BASE + 0x14UL))

// SysTick
#define SYSTICK_BASE 0xE000E010UL
#define SYSTICK_CTRL (*(volatile uint32_t *)(SYSTICK_BASE + 0x00UL))
#define SYSTICK_LOAD (*(volatile uint32_t *)(SYSTICK_BASE + 0x04UL))
#define SYSTICK_VAL (*(volatile uint32_t *)(SYSTICK_BASE + 0x08UL))

#define CPU_CLOCK_HZ 16000000UL

// 인터럽트에서 사용할 카운터
static volatile uint32_t ms_count = 0;

// SysTick 인터럽트 핸들러
void SysTick_Handler(void)
{
    ms_count++;

    if (ms_count >= 3000U)
    {
        ms_count = 0;

        // LED 상태 반전
        GPIOA_ODR ^= (1U << 5);
    }
}

void systick_init(void)
{
    // 16MHz / 1000 = 16000
    // 1ms마다 카운터 만료
    SYSTICK_LOAD = (CPU_CLOCK_HZ / 1000U) - 1U;

    // 카운터 초기화
    SYSTICK_VAL = 0U;

    // Bit 2: CLKSOURCE = 1 (CPU Clock)
    // Bit 1: TICKINT   = 1 (Interrupt Enable)!!!
    // Bit 0: ENABLE    = 1 (SysTick Enable)
    SYSTICK_CTRL = (1U << 2) | (1U << 1) | (1U << 0);
}

int main(void)
{
    // GPIOA 클럭 활성화
    RCC_AHB1ENR |= (1U << 0);

    // PA5 Output 설정
    GPIOA_MODER &= ~(3U << 10);
    GPIOA_MODER |= (1U << 10);

    // LED 초기 OFF
    GPIOA_ODR &= ~(1U << 5);

    // SysTick 인터럽트 시작
    systick_init();

    while (1)
    {
        // 다른 작업을 수행할 수 있음.
        // 현재는 인터럽트가 발생할 때까지 대기
        // 저전력모드
        __asm volatile("wfi");
    }
}
