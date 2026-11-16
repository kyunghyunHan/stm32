// GPIO로 LED를 주기적으로 깜빡이는 실습

#include <stdint.h>

#define RCC_BASE 0x40023800UL
#define RCC_AHB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x30UL))
#define GPIOB_BASE 0x40020400UL
#define GPIOB_MODER (*(volatile uint32_t *)(GPIOB_BASE + 0x00UL))
#define GPIOB_ODR (*(volatile uint32_t *)(GPIOB_BASE + 0x14UL))

// Time

#define SYSTICK_BASE 0xE000E010UL
#define SYSTICK_CTRL (*(volatile uint32_t *)(SYSTICK_BASE + 0x00UL))

#define SYSTICK_LOAD (*(volatile uint32_t *)(SYSTICK_BASE + 0x04UL))

#define SYSTICK_VAL (*(volatile uint32_t *)(SYSTICK_BASE + 0x08UL))

#define CPU_CLOCK_HZ 16000000UL

void delay_ms(uint32_t ms)
{

    SYSTICK_LOAD = (CPU_CLOCK_HZ / 1000U) - 1U;

    SYSTICK_VAL = 0U;

    //Polling방식
    SYSTICK_CTRL = (1U << 2) | (1U << 0);

    for (uint32_t i = 0; i < ms; i++)
    {

        while ((SYSTICK_CTRL & (1U << 16)) == 0U)
        {
        }
    }

    SYSTICK_CTRL = 0U;
}

int main(void)
{
    // GPIOB Clock 활성
    RCC_AHB1ENR |= (1U << 1);

    GPIOB_MODER &= ~(3U << 14);
    GPIOB_MODER |= (1U << 14);

    // GPIOB_ODR Bit 7 = 0
    GPIOB_ODR &= ~(1U << 7);

    while (1)
    {
        // XOR 연산을 사용.
        // 계속 ON/OFF
        GPIOB_ODR ^= (1U << 7);
        // 1000ms = 약 1초
        delay_ms(1000);
    }
}
