// 타이머 폴링으로 LED를 주기적으로 깜빡이는 실습

#include <stdint.h>

#define RCC_BASE    0x40023800UL
#define GPIOA_BASE  0x40020000UL
#define TIM2_BASE   0x40000000UL

#define RCC_AHB1ENR \
    (*(volatile uint32_t *)(RCC_BASE + 0x30UL))

#define RCC_APB1ENR \
    (*(volatile uint32_t *)(RCC_BASE + 0x40UL))

#define GPIOA_MODER \
    (*(volatile uint32_t *)(GPIOA_BASE + 0x00UL))

#define GPIOA_OTYPER \
    (*(volatile uint32_t *)(GPIOA_BASE + 0x04UL))

#define GPIOA_BSRR \
    (*(volatile uint32_t *)(GPIOA_BASE + 0x18UL))

#define TIM2_CR1 \
    (*(volatile uint32_t *)(TIM2_BASE + 0x00UL))

#define TIM2_SR \
    (*(volatile uint32_t *)(TIM2_BASE + 0x10UL))

#define TIM2_EGR \
    (*(volatile uint32_t *)(TIM2_BASE + 0x14UL))

#define TIM2_CNT \
    (*(volatile uint32_t *)(TIM2_BASE + 0x24UL))

#define TIM2_PSC \
    (*(volatile uint32_t *)(TIM2_BASE + 0x28UL))

#define TIM2_ARR \
    (*(volatile uint32_t *)(TIM2_BASE + 0x2CUL))

int main(void)
{

    // GPIOA 클럭 활성화 (GPIOAEN = 1)
    RCC_AHB1ENR |= (1U << 0);

    // TIM2 클럭 활성화 (TIM2EN = 1)
    RCC_APB1ENR |= (1U << 0);

    GPIOA_MODER &= ~(3U << 10);
    GPIOA_MODER |=  (1U << 10);

    GPIOA_OTYPER &= ~(1U << 5);

    GPIOA_BSRR = (1U << 21);

    TIM2_PSC = 16000U - 1U;

    TIM2_ARR = 1000U - 1U;

    TIM2_EGR = (1U << 0);

    TIM2_SR = ~(1U << 0);

    TIM2_CR1 |= (1U << 0);

    // 소프트웨어 LED 상태 변수
    uint32_t led_on = 0U;

    while (1)
    {

        if (TIM2_SR & (1U << 0))
        {

            TIM2_SR = ~(1U << 0);

            if (led_on)
            {

                GPIOA_BSRR = (1U << 21);
                led_on = 0U;
            }
            else
            {

                GPIOA_BSRR = (1U << 5);
                led_on = 1U;
            }
        }
    }
}
