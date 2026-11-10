
#include <stdint.h>

/*
 * STM32F429ZI
 *
 * TIM2 Polling + USART3 UART
 *
 * TIM2   : 1 second
 * USART3 : 9600bps, 8N1
 * UART TX: PD8 (AF7, ST-LINK VCP)
 *
 * Clock  : HSI 16MHz
 *
 * Output:
 * . . . . . (1 second interval)
 *
 * HAL / SysTick / Interrupt 사용 안 함
 */

// =====================================
// 1. Base Address
// =====================================

#define RCC_BASE 0x40023800UL
#define GPIOD_BASE 0x40020C00UL
#define TIM2_BASE 0x40000000UL
#define USART3_BASE 0x40004800UL

// =====================================
// 2. RCC
// =====================================

#define RCC_CR (*(volatile uint32_t *)(RCC_BASE + 0x00UL))
#define RCC_CFGR (*(volatile uint32_t *)(RCC_BASE + 0x08UL))
#define RCC_AHB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x30UL))
#define RCC_APB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x40UL))

// =====================================
// 3. GPIOD
// =====================================

#define GPIOD_MODER (*(volatile uint32_t *)(GPIOD_BASE + 0x00UL))
#define GPIOD_OTYPER (*(volatile uint32_t *)(GPIOD_BASE + 0x04UL))
#define GPIOD_AFRH (*(volatile uint32_t *)(GPIOD_BASE + 0x24UL))

// =====================================
// 4. TIM2
// =====================================

#define TIM2_CR1 (*(volatile uint32_t *)(TIM2_BASE + 0x00UL))
#define TIM2_SR (*(volatile uint32_t *)(TIM2_BASE + 0x10UL))
#define TIM2_EGR (*(volatile uint32_t *)(TIM2_BASE + 0x14UL))
#define TIM2_CNT (*(volatile uint32_t *)(TIM2_BASE + 0x24UL))
#define TIM2_PSC (*(volatile uint32_t *)(TIM2_BASE + 0x28UL))
#define TIM2_ARR (*(volatile uint32_t *)(TIM2_BASE + 0x2CUL))

// =====================================
// 5. USART3
// =====================================

#define USART3_SR (*(volatile uint32_t *)(USART3_BASE + 0x00UL))
#define USART3_DR (*(volatile uint32_t *)(USART3_BASE + 0x04UL))
#define USART3_BRR (*(volatile uint32_t *)(USART3_BASE + 0x08UL))
#define USART3_CR1 (*(volatile uint32_t *)(USART3_BASE + 0x0CUL))
#define USART3_CR2 (*(volatile uint32_t *)(USART3_BASE + 0x10UL))
#define USART3_CR3 (*(volatile uint32_t *)(USART3_BASE + 0x14UL))

// =====================================
// 6. Clock Initialization
// =====================================

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

// =====================================
// 7. TIM2 Initialization
// =====================================

void TIM2_Init(void)
{
    // TIM2 Clock Enable
    RCC_APB1ENR |= (1U << 0);

    // Stop TIM2
    TIM2_CR1 = 0U;

    /*
     * TIM2 Clock = 16MHz
     *
     * PSC = 15999
     * 16MHz / 16000 = 1000Hz
     *
     * ARR = 999
     * 1000Hz / 1000 = 1Hz
     */
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

// =====================================
// 8. TIM2 Polling
// =====================================

void TIM2_Wait_1s(void)
{
    // Wait for Update Flag
    while ((TIM2_SR & (1U << 0)) == 0U)
    {
    }

    // Clear UIF
    TIM2_SR = 0U;
}

// =====================================
// 9. USART3 Initialization
// =====================================

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

    /*
     * PCLK1 = 16MHz
     * Baudrate = 9600
     * Oversampling = 16
     *
     * USARTDIV = 16MHz / 9600
     *           = 1666.666...
     *
     * BRR = 0x0683
     */
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

// =====================================
// 11. Main
// =====================================

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
