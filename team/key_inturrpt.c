
#include <stdint.h>

// BASE
#define RCC_BASE 0x40023800UL
#define GPIOB_BASE 0x40020400UL
#define GPIOC_BASE 0x40020800UL
#define SYSCFG_BASE 0x40013800UL
#define EXTI_BASE 0x40013C00UL
#define NVIC_ISER_BASE 0xE000E100UL

// RCC
#define RCC_AHB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x30UL))
#define RCC_APB2ENR (*(volatile uint32_t *)(RCC_BASE + 0x44UL))

// GPIOB - LD3 (PB14)
#define GPIOB_MODER (*(volatile uint32_t *)(GPIOB_BASE + 0x00UL))
#define GPIOB_ODR (*(volatile uint32_t *)(GPIOB_BASE + 0x14UL))

// GPIOC - B1 (PC13)
#define GPIOC_MODER (*(volatile uint32_t *)(GPIOC_BASE + 0x00UL))
#define GPIOC_PUPDR (*(volatile uint32_t *)(GPIOC_BASE + 0x0CUL))

// SYSCFG
#define SYSCFG_EXTICR4 (*(volatile uint32_t *)(SYSCFG_BASE + 0x14UL))

// EXTI
#define EXTI_IMR (*(volatile uint32_t *)(EXTI_BASE + 0x00UL))
#define EXTI_RTSR (*(volatile uint32_t *)(EXTI_BASE + 0x08UL))
#define EXTI_FTSR (*(volatile uint32_t *)(EXTI_BASE + 0x0CUL))
#define EXTI_PR (*(volatile uint32_t *)(EXTI_BASE + 0x14UL))

// NVIC
//EXTI에서 발생한 인터럽트 요청을 NVIC로 전달할지, 차단할지 결정하는 레지스터
#define NVIC_ISER1 (*(volatile uint32_t *)(NVIC_ISER_BASE + 0x04UL))

// Handler
void EXTI15_10_IRQHandler(void)
{
    // EXTI13 Pending 확인
    // 1이면 pendding이 발생
    if (EXTI_PR & (1U << 13))
    {
        // Pending Flag 해제 (Write 1 to Clear)
        EXTI_PR = (1U << 13);

        // LD3(PB14) 토글
        GPIOB_ODR ^= (1U << 14);
    }
}
void gpio_init(void)
{
    // GPIOB / GPIOC Clock Enable
    RCC_AHB1ENR |= (1U << 1); // GPIOB
    RCC_AHB1ENR |= (1U << 2); // GPIOC

    // SYSCFG Clock Enable
    // STM32 MCU 내부의 시스템 연결 및 설정
    RCC_APB2ENR |= (1U << 14);

    // PB14 Output Mode (01)
    GPIOB_MODER &= ~(3U << 28);
    GPIOB_MODER |= (1U << 28);

    // LD3 초기 OFF
    GPIOB_ODR &= ~(1U << 14);

    // PC13 Input Mode (00)
    GPIOC_MODER &= ~(3U << 26);

    // 보드 외부 Pull-down 사용 → No Pull
    GPIOC_PUPDR &= ~(3U << 26);
}
void inturrpt_init()
{
    // EXTICR4[7:4] = 0010 (Port C)
    // EXTI12~15 GPIO 선택
    // GPIO의 PC13
    SYSCFG_EXTICR4 &= ~(0xFU << 4);
    SYSCFG_EXTICR4 |= (2U << 4);

    // Rising Edge 감지 (LOW → HIGH)
    //상승만 감지설정
    //상승 에지 감지 설정
    EXTI_RTSR |= (1U << 13);
    //하강 에지 감지 설정
    EXTI_FTSR &= ~(1U << 13);

    // 기존 EXTI13 Pending 제거
    // 해당 Pending 비트를 0으로 해제
    // 새로운 Rising Edge 감지 -> pending =1
    EXTI_PR = (1U << 13);

    // EXTI13 Interrupt Enable
    EXTI_IMR |= (1U << 13);

    // NVIC IRQ40 Enable
    // IRQ40 → ISER1 Bit8
    // IRQ 번호 = 40
    // 40 ÷ 32 = 1 (레지스터 번호)
    // 40 % 32 = 8 (비트 번호)
    // NVIC의 ISER 레지스터 하나가 32
    NVIC_ISER1 = (1U << 8);
}
int main(void)
{
    gpio_init();
    inturrpt_init();
    while (1)
    {
    }
}
