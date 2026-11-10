
#include <stdint.h>

/*
 * STM32F429ZI
 *
 * TIM6 Polling   : 500ms마다 '!'
 * TIM6 Interrupt : 500ms마다 '@'
 *
 * TIM6   : 1ms
 * USART3 : 115200bps, 8N1
 * TX     : PD8, AF7, ST-LINK VCP
 * Clock  : HSI 16MHz
 *
 * HAL 함수 사용 안 함
 */

#define RCC_BASE 0x40023800UL
#define GPIOD_BASE 0x40020C00UL
#define USART3_BASE 0x40004800UL
#define TIM6_BASE 0x40001000UL
#define NVIC_ISER1 (*(volatile uint32_t *)0xE000E104UL)

// RCC
#define RCC_CR (*(volatile uint32_t *)(RCC_BASE + 0x00UL))
#define RCC_CFGR (*(volatile uint32_t *)(RCC_BASE + 0x08UL))
#define RCC_AHB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x30UL))
#define RCC_APB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x40UL))

// GPIOD
#define GPIOD_MODER (*(volatile uint32_t *)(GPIOD_BASE + 0x00UL))
#define GPIOD_OTYPER (*(volatile uint32_t *)(GPIOD_BASE + 0x04UL))
#define GPIOD_AFRH (*(volatile uint32_t *)(GPIOD_BASE + 0x24UL))

// USART3
#define USART3_SR (*(volatile uint32_t *)(USART3_BASE + 0x00UL))
#define USART3_DR (*(volatile uint32_t *)(USART3_BASE + 0x04UL))
#define USART3_BRR (*(volatile uint32_t *)(USART3_BASE + 0x08UL))
#define USART3_CR1 (*(volatile uint32_t *)(USART3_BASE + 0x0CUL))
#define USART3_CR2 (*(volatile uint32_t *)(USART3_BASE + 0x10UL))
#define USART3_CR3 (*(volatile uint32_t *)(USART3_BASE + 0x14UL))

// TIM6
#define TIM6_CR1 (*(volatile uint32_t *)(TIM6_BASE + 0x00UL))
#define TIM6_DIER (*(volatile uint32_t *)(TIM6_BASE + 0x0CUL))
#define TIM6_SR (*(volatile uint32_t *)(TIM6_BASE + 0x10UL))
#define TIM6_EGR (*(volatile uint32_t *)(TIM6_BASE + 0x14UL))
#define TIM6_CNT (*(volatile uint32_t *)(TIM6_BASE + 0x24UL))
#define TIM6_PSC (*(volatile uint32_t *)(TIM6_BASE + 0x28UL))
#define TIM6_ARR (*(volatile uint32_t *)(TIM6_BASE + 0x2CUL))

// TIM6 인터럽트에서 증가하는 시간
volatile uint32_t tim6_ms = 0U;

/* 1. Clock 초기화: HSI 16MHz 사용 */
void Clock_Init(void)
{
    // RCC_CR bit 0 (HSION) = 1
    // 내부 HSI 16MHz 오실레이터 활성화
    RCC_CR |= (1U << 0);

    // RCC_CR bit 1 (HSIRDY)
    // HSI 클럭이 안정화될 때까지 대기
    while ((RCC_CR & (1U << 1)) == 0U)
    {
    }

    // RCC_CFGR bit [1:0] (SW) = 00
    // 시스템 클럭(SYSCLK)을 HSI 16MHz로 선택
    RCC_CFGR &= ~(3U << 0);

    // RCC_CFGR bit [3:2] (SWS) = 00
    // 실제 시스템 클럭이 HSI로 전환됐는지 확인
    while ((RCC_CFGR & (3U << 2)) != 0U)
    {
    }

    // RCC_CFGR bit [7:4] (HPRE) = 0000
    // AHB Prescaler = /1
    // HCLK = SYSCLK / 1 = 16MHz (CPU, AHB)
    RCC_CFGR &= ~(0xFU << 4);

    // RCC_CFGR bit [12:10] (PPRE1) = 000
    // APB1 Prescaler = /1
    // PCLK1 = HCLK / 1 = 16MHz (USART3, TIM6 등)
    RCC_CFGR &= ~(7U << 10);

    // RCC_CFGR bit [15:13] (PPRE2) = 000
    // APB2 Prescaler = /1
    // PCLK2 = HCLK / 1 = 16MHz (USART1, SPI1 등)
    RCC_CFGR &= ~(7U << 13);
}
/* 2. USART3 초기화 */
void USART3_Init(void)
{
    // GPIOD Clock Enable
    RCC_AHB1ENR |= (1U << 3);

    // USART3 Clock Enable
    RCC_APB1ENR |= (1U << 18);

    // PD8: Alternate Function
    GPIOD_MODER &= ~(3U << 16);
    GPIOD_MODER |= (2U << 16);

    // Push-Pull
    GPIOD_OTYPER &= ~(1U << 8);

    // PD8 = AF7 (USART3_TX)
    GPIOD_AFRH &= ~(0xFU << 0);
    GPIOD_AFRH |= (7U << 0);

    USART3_CR1 = 0U;
    USART3_CR2 = 0U;
    USART3_CR3 = 0U;

    // PCLK1 = 16MHz, Baud = 115200
    USART3_BRR = 0x008BU;

    // USART Enable + TX Enable
    USART3_CR1 = (1U << 13) | (1U << 3);
}

/* 3. UART 문자 전송 */
void USART3_SendChar(char c)
{
    // TXE = 1 대기
    while ((USART3_SR & (1U << 7)) == 0U)
    {
    }

    USART3_DR = (uint8_t)c;
}

/* 4. TIM6 초기화 */
void TIM6_Init(void)
{
    // TIM6 Clock Enable (APB1)
    RCC_APB1ENR |= (1U << 4);

    // Timer Disable
    TIM6_CR1 = 0U;
    TIM6_DIER = 0U;

    /*
     * TIM6 Clock = 16MHz
     *
     * Prescaler = 15
     * 16MHz / (15 + 1) = 1MHz
     *
     * ARR = 999
     * 1000카운트 = 1ms
     */

    TIM6_PSC = 15U;
    TIM6_ARR = 999U;
    TIM6_CNT = 0U;

    // Prescaler 적용을 위한 Update Event
    TIM6_EGR = (1U << 0);

    // 초기 Update Flag 제거
    TIM6_SR = 0U;

    // Update Interrupt Enable
    TIM6_DIER |= (1U << 0);

    // TIM6_DAC_IRQn = 54
    // NVIC_ISER1 bit 22
    NVIC_ISER1 = (1U << 22);

    // Counter Enable
    TIM6_CR1 |= (1U << 0);
}

/* 5. TIM6 인터럽트 사용자 함수 */
void TIM6_UserHandler(void)
{
    static uint32_t exception_count = 0U;

    // UIF 확인
    if ((TIM6_SR & (1U << 0)) != 0U)
    {
        // UIF Clear
        TIM6_SR = 0U;

        // 1ms 증가
        tim6_ms++;

        exception_count++;

        // 500ms마다 '@'
        if (exception_count >= 500U)
        {
            exception_count = 0U;
            USART3_SendChar('@');
        }
    }
}

/* 6. Main: Polling */
int main(void)
{
    Clock_Init();
    USART3_Init();
    TIM6_Init();

    uint32_t last_poll_ms = 0U;

    while (1)
    {
        // 500ms마다 '!'
        if ((uint32_t)(tim6_ms - last_poll_ms) >= 500U)
        {
            last_poll_ms += 500U;
            USART3_SendChar('!');
        }
    }
}
