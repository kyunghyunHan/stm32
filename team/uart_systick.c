
#include <stdint.h>

/*
 * STM32F429ZI - SysTick Polling (16MHz)
 *
 * HSI     : 16MHz
 * SYSCLK  : 16MHz
 * HCLK    : 16MHz
 * PCLK1   : 16MHz
 * PCLK2   : 16MHz
 *
 * SysTick Clock : HCLK / 8 = 2MHz
 * SysTick       : 1000ms
 *
 * USART3 : 115200bps, 8N1
 * TX     : PD8, AF7, ST-LINK VCP
 *
 * Output : '.' every 1000ms
 *
 * HAL / SysTick Interrupt 사용 안 함
 */

#define RCC_BASE       0x40023800UL
#define GPIOD_BASE     0x40020C00UL
#define USART3_BASE    0x40004800UL
#define SYSTICK_BASE   0xE000E010UL

// RCC
#define RCC_CR         (*(volatile uint32_t *)(RCC_BASE + 0x00UL))
#define RCC_CFGR       (*(volatile uint32_t *)(RCC_BASE + 0x08UL))
#define RCC_AHB1ENR    (*(volatile uint32_t *)(RCC_BASE + 0x30UL))
#define RCC_APB1ENR    (*(volatile uint32_t *)(RCC_BASE + 0x40UL))

// GPIOD
#define GPIOD_MODER    (*(volatile uint32_t *)(GPIOD_BASE + 0x00UL))
#define GPIOD_OTYPER   (*(volatile uint32_t *)(GPIOD_BASE + 0x04UL))
#define GPIOD_AFRH     (*(volatile uint32_t *)(GPIOD_BASE + 0x24UL))

// USART3
#define USART3_SR      (*(volatile uint32_t *)(USART3_BASE + 0x00UL))
#define USART3_DR      (*(volatile uint32_t *)(USART3_BASE + 0x04UL))
#define USART3_BRR     (*(volatile uint32_t *)(USART3_BASE + 0x08UL))
#define USART3_CR1     (*(volatile uint32_t *)(USART3_BASE + 0x0CUL))
#define USART3_CR2     (*(volatile uint32_t *)(USART3_BASE + 0x10UL))
#define USART3_CR3     (*(volatile uint32_t *)(USART3_BASE + 0x14UL))

// SysTick
#define SYST_CSR       (*(volatile uint32_t *)(SYSTICK_BASE + 0x00UL))
#define SYST_RVR       (*(volatile uint32_t *)(SYSTICK_BASE + 0x04UL))
#define SYST_CVR       (*(volatile uint32_t *)(SYSTICK_BASE + 0x08UL))

#define CPU_CLOCK_HZ   16000000UL
#define SYSTICK_MS     1000U

// =====================================
// 1. Clock Initialization
// =====================================

void Clock_Init(void)
{
    // HSI 16MHz Enable
    RCC_CR |= (1U << 0);

    // HSIRDY 확인
    while ((RCC_CR & (1U << 1)) == 0U)
    {
    }

    // SYSCLK = HSI
    RCC_CFGR &= ~(3U << 0);

    // SYSCLK 전환 완료 확인
    while ((RCC_CFGR & (3U << 2)) != 0U)
    {
    }

    // AHB /1 -> HCLK = 16MHz
    RCC_CFGR &= ~(0xFU << 4);

    // APB1 /1 -> PCLK1 = 16MHz
    RCC_CFGR &= ~(7U << 10);

    // APB2 /1 -> PCLK2 = 16MHz
    RCC_CFGR &= ~(7U << 13);
}

// =====================================
// 2. USART3 Initialization
// =====================================

void USART3_Init(void)
{
    // GPIOD Clock Enable
    RCC_AHB1ENR |= (1U << 3);

    // USART3 Clock Enable
    RCC_APB1ENR |= (1U << 18);

    // PD8 Alternate Function Mode
    GPIOD_MODER &= ~(3U << 16);
    GPIOD_MODER |=  (2U << 16);

    // PD8 Push-Pull
    GPIOD_OTYPER &= ~(1U << 8);

    // PD8 AF7 = USART3_TX
    GPIOD_AFRH &= ~(0xFU << 0);
    GPIOD_AFRH |=  (7U << 0);

    // USART Configuration Reset
    USART3_CR1 = 0U;
    USART3_CR2 = 0U;
    USART3_CR3 = 0U;

    // PCLK1 = 16MHz
    // Baud = 115200bps
    USART3_BRR = 0x008BU;

    // UE = 1, TE = 1
    USART3_CR1 = (1U << 13) | (1U << 3);
}

// =====================================
// 3. UART Transmit
// =====================================

void USART3_SendChar(char c)
{
    // TXE = 1 대기
    while ((USART3_SR & (1U << 7)) == 0U)
    {
    }

    USART3_DR = (uint8_t)c;
}

// =====================================
// 4. SysTick Initialization
// =====================================

void SysTick_Init(void)
{
    // SysTick Disable
    SYST_CSR = 0U;

    /*
     * HCLK = 16MHz
     * SysTick Clock = HCLK / 8 = 2MHz
     *
     * 500ms  = 1,000,000 clocks
     * 1000ms = 2,000,000 clocks
     *
     * 24-bit Maximum LOAD = 16,777,215
     */

    // 기존 500ms 설정
    // SYST_RVR = 1000000U - 1U;

    // 변경: 1000ms 설정
    SYST_RVR =
        (CPU_CLOCK_HZ / 8U / 1000U * SYSTICK_MS) - 1U;

    // Current Value Clear
    SYST_CVR = 0U;

    // CLKSOURCE = 0 : HCLK/8
    // TICKINT   = 0 : Interrupt Disable
    // ENABLE    = 1 : Counter Enable
    SYST_CSR = (1U << 0);
}

// =====================================
// 5. Main
// =====================================

int main(void)
{
    Clock_Init();
    USART3_Init();
    SysTick_Init();

    while (1)
    {
        // COUNTFLAG = 1 -> 1000ms 경과
        if (SYST_CSR & (1U << 16))
        {
            USART3_SendChar('.');
        }
    }
}
