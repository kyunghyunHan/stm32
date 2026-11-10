
#include <stdint.h>

/*
 * STM32F429ZI - TIM6 Polling + Interrupt
 *
 * TIM6 Polling   : 500ms마다 '!'
 * TIM6 Interrupt : 500ms마다 '@'
 *
 * TIM6 Clock : 16MHz
 * TIM6 Period: 500ms
 *
 * USART3 : 115200bps, 8N1
 * TX     : PD8, AF7, ST-LINK VCP
 * Clock  : HSI 16MHz
 *
 * HAL 사용 안 함
 * TIM2 / TIM7 사용 안 함
 * TIM6 하나만 사용
 */

#define RCC_BASE       0x40023800UL
#define GPIOD_BASE     0x40020C00UL
#define USART3_BASE    0x40004800UL
#define TIM6_BASE      0x40001000UL

#define NVIC_ISER1     (*(volatile uint32_t *)0xE000E104UL)

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

// TIM6
#define TIM6_CR1       (*(volatile uint32_t *)(TIM6_BASE + 0x00UL))
#define TIM6_DIER      (*(volatile uint32_t *)(TIM6_BASE + 0x0CUL))
#define TIM6_SR        (*(volatile uint32_t *)(TIM6_BASE + 0x10UL))
#define TIM6_EGR       (*(volatile uint32_t *)(TIM6_BASE + 0x14UL))
#define TIM6_CNT       (*(volatile uint32_t *)(TIM6_BASE + 0x24UL))
#define TIM6_PSC       (*(volatile uint32_t *)(TIM6_BASE + 0x28UL))
#define TIM6_ARR       (*(volatile uint32_t *)(TIM6_BASE + 0x2CUL))


/* 1. Clock 초기화 */
void Clock_Init(void)
{
    // HSI 16MHz 활성화
    RCC_CR |= (1U << 0);

    // HSI Ready
    while ((RCC_CR & (1U << 1)) == 0U)
    {
    }

    // SYSCLK = HSI
    RCC_CFGR &= ~(3U << 0);

    while ((RCC_CFGR & (3U << 2)) != 0U)
    {
    }

    // AHB /1
    RCC_CFGR &= ~(0xFU << 4);

    // APB1 /1
    RCC_CFGR &= ~(7U << 10);

    // APB2 /1
    RCC_CFGR &= ~(7U << 13);
}


/* 2. USART3 초기화 */
void USART3_Init(void)
{
    // GPIOD Clock Enable
    RCC_AHB1ENR |= (1U << 3);

    // USART3 Clock Enable
    RCC_APB1ENR |= (1U << 18);

    // PD8 Alternate Function
    GPIOD_MODER &= ~(3U << 16);
    GPIOD_MODER |=  (2U << 16);

    // PD8 Push-Pull
    GPIOD_OTYPER &= ~(1U << 8);

    // PD8 AF7 = USART3_TX
    GPIOD_AFRH &= ~(0xFU << 0);
    GPIOD_AFRH |=  (7U << 0);

    // USART 설정
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


/* 4. TIM6 초기화: 500ms */
void TIM6_Init(void)
{
    // TIM6 Clock Enable
    RCC_APB1ENR |= (1U << 4);

    // TIM6 정지
    TIM6_CR1 = 0U;
    TIM6_DIER = 0U;

    /*
     * TIM6CLK = 16MHz
     *
     * PSC = 15999
     * 16MHz / 16000 = 1000Hz
     *
     * Counter는 1ms마다 증가
     *
     * ARR = 499
     * 500카운트 = 500ms
     */

    TIM6_PSC = 15999U;
    TIM6_ARR = 499U;
    TIM6_CNT = 0U;

    // Prescaler 적용
    TIM6_EGR = (1U << 0);

    // 초기 UIF 제거
    TIM6_SR = 0U;

    // TIM6 Update Interrupt Enable
    TIM6_DIER |= (1U << 0);

    // TIM6_DAC_IRQn = 54
    // NVIC_ISER1 bit 22
    NVIC_ISER1 = (1U << 22);

    // TIM6 Counter 시작
    TIM6_CR1 |= (1U << 0);
}


/* 5. TIM6 Interrupt: 500ms마다 '@' */
void TIM6_UserHandler(void)
{
    // TIM6 Update Interrupt 확인
    if (TIM6_SR & (1U << 0))
    {
        // UIF Clear
        TIM6_SR = 0U;

        // 500ms마다 '@'
        USART3_SendChar('@');
    }
}


/* 6. Main: 순수 TIM6 CNT Polling */
int main(void)
{
    Clock_Init();
    USART3_Init();
    TIM6_Init();

    uint32_t previous_cnt = TIM6_CNT;

    while (1)
    {
        // TIM6 현재 카운터 직접 읽기
        uint32_t current_cnt = TIM6_CNT;

        // CNT가 499 -> 0으로 돌아왔는지 확인
        if (current_cnt < previous_cnt)
        {
            // 500ms마다 '!'
            USART3_SendChar('!');
        }

        previous_cnt = current_cnt;
    }
}
