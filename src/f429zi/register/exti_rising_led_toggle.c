
#include <stdint.h>

#define RCC_BASE        0x40023800UL
#define GPIOB_BASE      0x40020400UL
#define GPIOC_BASE      0x40020800UL
#define SYSCFG_BASE     0x40013800UL
#define EXTI_BASE       0x40013C00UL
#define NVIC_ISER_BASE  0xE000E100UL

// RCC
#define RCC_AHB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x30UL))
#define RCC_APB2ENR (*(volatile uint32_t *)(RCC_BASE + 0x44UL))

// GPIOB - LD2(PB7)
#define GPIOB_MODER (*(volatile uint32_t *)(GPIOB_BASE + 0x00UL))
#define GPIOB_ODR   (*(volatile uint32_t *)(GPIOB_BASE + 0x14UL))

// GPIOC - B1(PC13)
#define GPIOC_MODER (*(volatile uint32_t *)(GPIOC_BASE + 0x00UL))
#define GPIOC_PUPDR (*(volatile uint32_t *)(GPIOC_BASE + 0x0CUL))

// SYSCFG
#define SYSCFG_EXTICR4 (*(volatile uint32_t *)(SYSCFG_BASE + 0x14UL))

// EXTI
#define EXTI_IMR  (*(volatile uint32_t *)(EXTI_BASE + 0x00UL))
#define EXTI_RTSR (*(volatile uint32_t *)(EXTI_BASE + 0x08UL))
#define EXTI_FTSR (*(volatile uint32_t *)(EXTI_BASE + 0x0CUL))
#define EXTI_PR   (*(volatile uint32_t *)(EXTI_BASE + 0x14UL))

// NVIC
#define NVIC_ISER1 (*(volatile uint32_t *)(NVIC_ISER_BASE + 0x04UL))


// 인터럽트 핸들러
void EXTI15_10_IRQHandler(void)
{
    if (EXTI_PR & (1U << 13))
    {
        // Pending Flag 제거
        EXTI_PR = (1U << 13);

        // LD2(PB7) 토글
        GPIOB_ODR ^= (1U << 7);
    }
}

int main(void)
{
    // 1. GPIOB, GPIOC Clock Enable
    RCC_AHB1ENR |= (1U << 1);
    RCC_AHB1ENR |= (1U << 2);

    // 2. SYSCFG Clock Enable
    RCC_APB2ENR |= (1U << 14);

    // 3. PB7 Output Mode (01)
    GPIOB_MODER &= ~(3U << 14);
    GPIOB_MODER |=  (1U << 14);

    // LED 초기 OFF
    GPIOB_ODR &= ~(1U << 7);

    // 4. PC13 Input Mode (00)
    GPIOC_MODER &= ~(3U << 26);

    // 보드에 외부 Pull-down이 있으므로 No Pull
    GPIOC_PUPDR &= ~(3U << 26);

    // 5. EXTI13 -> GPIOC 연결
    SYSCFG_EXTICR4 &= ~(0xFU << 4);
    SYSCFG_EXTICR4 |=  (2U << 4);

    // 6. Rising Edge 설정
    EXTI_RTSR |=  (1U << 13);
    EXTI_FTSR &= ~(1U << 13);

    // 7. Pending Flag 제거
    EXTI_PR = (1U << 13);

    // 8. EXTI13 Interrupt Enable
    EXTI_IMR |= (1U << 13);

    // 9. NVIC Enable (IRQ40)
    NVIC_ISER1 = (1U << 8);

    while (1)
    {
        // 메인 루프
        // 버튼 입력은 인터럽트로 처리
    }
}
