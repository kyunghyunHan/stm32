// GPIO로 LED를 주기적으로 깜빡이는 실습
#include <stdint.h>

#define RCC_BASE 0x40023800UL

#define RCC_AHB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x30UL))

#define GPIOA_BASE 0x40020000UL

// GPIOA Mode Register
#define GPIOA_MODER (*(volatile uint32_t *)(GPIOA_BASE + 0x00UL))

// GPIOA Output Data Register
#define GPIOA_ODR (*(volatile uint32_t *)(GPIOA_BASE + 0x14UL))

#define SYSTICK_BASE 0xE000E010UL

#define SYSTICK_CTRL (*(volatile uint32_t *)(SYSTICK_BASE + 0x00UL))
#define SYSTICK_LOAD (*(volatile uint32_t *)(SYSTICK_BASE + 0x04UL))
#define SYSTICK_VAL (*(volatile uint32_t *)(SYSTICK_BASE + 0x08UL))

#define CPU_CLOCK_HZ 16000000UL

void delay_ms(uint32_t ms)
{
    // 1ms에 해당하는 Reload 값 설정
    SYSTICK_LOAD = (CPU_CLOCK_HZ / 1000U) - 1U;

    SYSTICK_VAL = 0U;

    SYSTICK_CTRL = (1U << 2) | (1U << 0);

    for (uint32_t i = 0; i < ms; i++)
    {

        while ((SYSTICK_CTRL & (1U << 16)) == 0U)
        {
        }
    }

    // SysTick 비활성화
    SYSTICK_CTRL = 0U;
}

int main(void)
{

    RCC_AHB1ENR |= (1U << 0);

    // PA5 MODER 초기화
    GPIOA_MODER &= ~(3U << 10);

    // PA5 Output(01)
    GPIOA_MODER |= (1U << 10);

    while (1)
    {
        // PA5 상태 반전
        GPIOA_ODR ^= (1U << 5);

        // 3초 지연
        delay_ms(3000);
    }
}

