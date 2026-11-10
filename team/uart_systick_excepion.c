
#include <stdint.h>

/*
 * STM32F429ZI
 *
 * SysTick Polling   : 1000ms마다 '.'
 * SysTick Exception : 500ms마다 '*'
 *
 * SysTick : 1ms
 * USART3  : 115200bps, 8N1
 * TX      : PD8, AF7, ST-LINK VCP
 * Clock   : HSI 16MHz
 *
 * HAL 함수 사용 안 함
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

// =====================================
// 1. Shared Variables
// =====================================

// Exception Handler에서 증가
volatile uint32_t systick_ms = 0;

// Exception에서 발생한 '*' 이벤트 개수
volatile uint32_t star_events = 0;

// =====================================
// 2. Clock Initialization
// =====================================

void Clock_Init(void)
{
    // HSI Enable
    RCC_CR |= (1U << 0);

    while ((RCC_CR & (1U << 1)) == 0U)
    {
    }

    // SYSCLK = HSI
    RCC_CFGR &= ~(3U << 0);

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
// 3. USART3 Initialization
// =====================================

void USART3_Init(void)
{
    // GPIOD Clock Enable
    RCC_AHB1ENR |= (1U << 3);

    // USART3 Clock Enable
    RCC_APB1ENR |= (1U << 18);

    // PD8 Alternate Function
    GPIOD_MODER &= ~(3U << 16);
    GPIOD_MODER |=  (2U << 16);

    // Push-Pull
    GPIOD_OTYPER &= ~(1U << 8);

    // AF7
    GPIOD_AFRH &= ~(0xFU << 0);
    GPIOD_AFRH |=  (7U << 0);

    // USART Configuration
    USART3_CR1 = 0U;
    USART3_CR2 = 0U;
    USART3_CR3 = 0U;

    // PCLK1 = 16MHz, Baud = 115200
    USART3_BRR = 0x008BU;

    // USART Enable + TX Enable
    USART3_CR1 = (1U << 13) | (1U << 3);
}

// =====================================
// 4. UART Transmit
// =====================================

void USART3_SendChar(char c)
{
    while ((USART3_SR & (1U << 7)) == 0U)
    {
    }

    USART3_DR = (uint8_t)c;
}

// =====================================
// 5. SysTick Initialization
// =====================================

void SysTick_Init(void)
{
    // Disable
    SYST_CSR = 0U;

    // 16MHz / 1000 = 16000
    // SysTick 1ms
    SYST_RVR = 16000U - 1U;

    SYST_CVR = 0U;

    // CLKSOURCE = 1
    // TICKINT   = 1
    // ENABLE    = 1
    SYST_CSR = (1U << 2) |
               (1U << 1) |
               (1U << 0);
}

// =====================================
// 6. Exception User Function
// =====================================

// stm32f4xx_it.c에서 호출
void SysTick_UserHandler(void)
{
    static uint32_t exception_count = 0U;

    systick_ms++;
    exception_count++;

    if (exception_count >= 500U)
    {
        exception_count = 0U;
        USART3_SendChar('*');
    }
}

// =====================================
// 7. Main - Polling
// =====================================

int main(void)
{
    Clock_Init();
    USART3_Init();
    SysTick_Init();

    uint32_t last_dot_ms = 0U;

    while (1)
    {
        // 1000ms마다 '.' 출력
        if ((uint32_t)(systick_ms - last_dot_ms) >= 1000U)
        {
            last_dot_ms += 1000U;
            USART3_SendChar('.');
        }
    }
}