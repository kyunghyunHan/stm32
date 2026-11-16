// 타이머 인터럽트와 카운터 폴링으로 UART 문자를 전송하는 실습

#include <stdint.h>

#define RCC_BASE       0x40023800UL
#define GPIOD_BASE     0x40020C00UL
#define USART3_BASE    0x40004800UL
#define TIM6_BASE      0x40001000UL

#define NVIC_ISER1     (*(volatile uint32_t *)0xE000E104UL)

// RCC - Reset and Clock Control
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

// TIM6 - Basic Timer
#define TIM6_CR1       (*(volatile uint32_t *)(TIM6_BASE + 0x00UL))
#define TIM6_DIER      (*(volatile uint32_t *)(TIM6_BASE + 0x0CUL))
#define TIM6_SR        (*(volatile uint32_t *)(TIM6_BASE + 0x10UL))
#define TIM6_EGR       (*(volatile uint32_t *)(TIM6_BASE + 0x14UL))
#define TIM6_CNT       (*(volatile uint32_t *)(TIM6_BASE + 0x24UL))
#define TIM6_PSC       (*(volatile uint32_t *)(TIM6_BASE + 0x28UL))
#define TIM6_ARR       (*(volatile uint32_t *)(TIM6_BASE + 0x2CUL))

void Clock_Init(void)
{
    // 1. HSI 16MHz 활성화
    RCC_CR |= (1U << 0);

    // 2. HSI 준비 완료 확인
    while ((RCC_CR & (1U << 1)) == 0U)
    {
    }

    // 3. SYSCLK 소스를 HSI로 선택
    RCC_CFGR &= ~(3U << 0);

    // 4. SYSCLK가 실제 HSI로 전환되었는지 확인
    while ((RCC_CFGR & (3U << 2)) != 0U)
    {
    }

    // 5. AHB /1 → HCLK = 16MHz
    RCC_CFGR &= ~(0xFU << 4);

    // 6. APB1 /1 → PCLK1 = 16MHz
    RCC_CFGR &= ~(7U << 10);

    // 7. APB2 /1 → PCLK2 = 16MHz
    RCC_CFGR &= ~(7U << 13);
}

void USART3_Init(void)
{
    // 1. GPIOD Clock Enable
    RCC_AHB1ENR |= (1U << 3);

    // 2. USART3 Clock Enable
    RCC_APB1ENR |= (1U << 18);

    // 3. PD8 Alternate Function Mode (10)
    GPIOD_MODER &= ~(3U << 16);
    GPIOD_MODER |=  (2U << 16);

    // 4. PD8 Push-Pull
    GPIOD_OTYPER &= ~(1U << 8);

    // 5. PD8 AF7 = USART3_TX
    GPIOD_AFRH &= ~(0xFU << 0);
    GPIOD_AFRH |=  (7U << 0);

    // 6. USART 설정 초기화
    USART3_CR1 = 0U;
    USART3_CR2 = 0U;
    USART3_CR3 = 0U;

    // 7. PCLK1 = 16MHz, 115200bps
    // OVER8 = 0 (16배 오버샘플링)
    USART3_BRR = 0x008BU;

    // 8. USART Enable + Transmitter Enable
    // UE(bit13) = 1
    // TE(bit3)  = 1
    USART3_CR1 = (1U << 13) | (1U << 3);
}

void USART3_SendChar(char c)
{
    // TXE(bit7) = 1이 될 때까지 대기
    while ((USART3_SR & (1U << 7)) == 0U)
    {
    }

    // 문자 전송 데이터 기록
    USART3_DR = (uint8_t)c;
}

void TIM6_Init(void)
{
    // 1. APB1에서 TIM6 Clock Enable
    RCC_APB1ENR |= (1U << 4);

    // 2. TIM6 정지
    TIM6_CR1 = 0U;

    // 3. 인터럽트 비활성화
    TIM6_DIER = 0U;

    // 4. Prescaler 설정
    // 16MHz / (15999 + 1) = 1000Hz
    TIM6_PSC = 15999U;

    // 5. Auto-Reload 설정
    // 0~499 = 총 500카운트 = 500ms
    TIM6_ARR = 499U;

    // 6. Counter 초기화
    TIM6_CNT = 0U;

    // 7. UG(Update Generation) 발생
    // PSC 설정을 실제 분주기에 반영
    TIM6_EGR = (1U << 0);

    // 8. 초기 UIF(Update Interrupt Flag) 제거
    // UG로 인해 설정된 UIF를 초기화
    TIM6_SR = 0U;

    // 9. TIM6 Update Interrupt Enable
    // DIER.UIE(bit0) = 1
    TIM6_DIER |= (1U << 0);

    // 10. NVIC에서 TIM6 인터럽트 허용
    // TIM6_DAC_IRQn = 54
    // ISER1 bit22 = 54 - 32
    NVIC_ISER1 = (1U << 22);

    // 11. TIM6 Counter 시작
    // CR1.CEN(bit0) = 1
    TIM6_CR1 |= (1U << 0);
}

void TIM6_UserHandler(void)
{
    // UIF(bit0) 확인: Update Event 발생 여부
    if (TIM6_SR & (1U << 0))
    {
        // 인터럽트 플래그 제거
        TIM6_SR = 0U;

        // 500ms마다 '@' 출력
        USART3_SendChar('@');
    }
}

int main(void)
{
    Clock_Init();
    USART3_Init();
    TIM6_Init();

    // 이전 카운터 값 저장
    uint32_t previous_cnt = TIM6_CNT;

    while (1)
    {
        // 현재 TIM6 카운터 값 읽기
        uint32_t current_cnt = TIM6_CNT;

        // CNT가 499 → 0으로 돌아왔는지 감지
        if (current_cnt < previous_cnt)
        {
            // Polling: 500ms마다 '!' 출력
            USART3_SendChar('!');
        }

        // 다음 비교를 위해 현재 값 저장
        previous_cnt = current_cnt;
    }
}
