// 타이머 폴링으로 UART 문자를 주기적으로 전송하는 실습

#include <stdint.h>

#define RCC_BASE 0x40023800UL
#define GPIOD_BASE 0x40020C00UL
#define TIM2_BASE 0x40000000UL
#define USART3_BASE 0x40004800UL

#define RCC_CR (*(volatile uint32_t *)(RCC_BASE + 0x00UL))
#define RCC_CFGR (*(volatile uint32_t *)(RCC_BASE + 0x08UL))
#define RCC_AHB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x30UL))
#define RCC_APB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x40UL))

#define GPIOD_MODER (*(volatile uint32_t *)(GPIOD_BASE + 0x00UL))
#define GPIOD_OTYPER (*(volatile uint32_t *)(GPIOD_BASE + 0x04UL))
#define GPIOD_AFRH (*(volatile uint32_t *)(GPIOD_BASE + 0x24UL))

#define TIM2_CR1 (*(volatile uint32_t *)(TIM2_BASE + 0x00UL))
#define TIM2_SR (*(volatile uint32_t *)(TIM2_BASE + 0x10UL))
#define TIM2_EGR (*(volatile uint32_t *)(TIM2_BASE + 0x14UL))
#define TIM2_CNT (*(volatile uint32_t *)(TIM2_BASE + 0x24UL))
#define TIM2_PSC (*(volatile uint32_t *)(TIM2_BASE + 0x28UL))
#define TIM2_ARR (*(volatile uint32_t *)(TIM2_BASE + 0x2CUL))

#define USART3_SR (*(volatile uint32_t *)(USART3_BASE + 0x00UL))
#define USART3_DR (*(volatile uint32_t *)(USART3_BASE + 0x04UL))
#define USART3_BRR (*(volatile uint32_t *)(USART3_BASE + 0x08UL))
#define USART3_CR1 (*(volatile uint32_t *)(USART3_BASE + 0x0CUL))
#define USART3_CR2 (*(volatile uint32_t *)(USART3_BASE + 0x10UL))
#define USART3_CR3 (*(volatile uint32_t *)(USART3_BASE + 0x14UL))

void Clock_Init(void)
{
    // HSI Enable
    RCC_CR |= (1U << 0);

    // HSI Ready
    while (!(RCC_CR & (1U << 1)))
    {
    }

    // SYSCLK = HSI
    RCC_CFGR &= ~(3U << 0);

    // Wait SYSCLK = HSI
    while ((RCC_CFGR & (3U << 2)) != 0U)
    {
    }

    // AHB = /1
    RCC_CFGR &= ~(0xFU << 4);

    // APB1 = /1
    RCC_CFGR &= ~(7U << 10);

    // APB2 = /1
    RCC_CFGR &= ~(7U << 13);
}

void TIM2_Init(void)
{
    // TIM2 Clock Enable
    RCC_APB1ENR |= (1U << 0);

    // Stop TIM2
    TIM2_CR1 = 0U;

    TIM2_PSC = 15999U;
    TIM2_ARR = 999U;

    // Apply Prescaler
    TIM2_EGR = 1U;

    // Clear UIF
    TIM2_SR = 0U;

    // Reset Counter
    TIM2_CNT = 0U;

    // Start TIM2
    TIM2_CR1 |= 1U;
}

void TIM2_Wait_1s(void)
{
    // Wait for Update Flag
    while ((TIM2_SR & (1U << 0)) == 0U)
    {
    }

    // Clear UIF
    TIM2_SR = 0U;
}

void USART3_Init(void)
{
    // GPIOD Clock Enable
    RCC_AHB1ENR |= (1U << 3);

    // USART3 Clock Enable (APB1 Bit 18)
    RCC_APB1ENR |= (1U << 18);

    // PD8 Alternate Function Mode (10)
    GPIOD_MODER &= ~(3U << 16);
    GPIOD_MODER |= (2U << 16);

    // PD8 Push-Pull
    GPIOD_OTYPER &= ~(1U << 8);

    // PD8 AF7 (USART3 TX)
    GPIOD_AFRH &= ~(0xFU << 0);
    GPIOD_AFRH |= (7U << 0);

    // Disable USART before configuration
    USART3_CR1 = 0U;
    USART3_CR2 = 0U;
    USART3_CR3 = 0U;

    USART3_BRR = 0x008BU;

    USART3_CR1 = (1U << 13) | (1U << 3);
}

void USART3_SendChar(char c)
{
    // Wait TXE = 1
    while ((USART3_SR & (1U << 7)) == 0U)
    {
    }

    USART3_DR = (uint8_t)c;
}

int main(void)
{
    Clock_Init();
    TIM2_Init();
    USART3_Init();

    while (1)
    {
        TIM2_Wait_1s();

        USART3_SendChar('.');
    }
}
