
#include <stdint.h>

/*
 * ============================================================
 * STM32F429ZI - TIM6 Polling + Interrupt
 * ============================================================
 *
 * [실습 목표]
 * - TIM6 하나로 500ms 주기 생성
 * - Polling   : 500ms마다 '!' 출력
 * - Interrupt : 500ms마다 '@' 출력
 *
 * [Clock 설정]
 * HSI       : 16MHz
 * SYSCLK    : 16MHz
 * AHB       : /1  → HCLK  = 16MHz
 * APB1      : /1  → PCLK1 = 16MHz
 * APB2      : /1  → PCLK2 = 16MHz
 * TIM6CLK   : 16MHz
 *
 * [TIM6 설정]
 * PSC       : 15999
 * ARR       : 499
 * CNT Clock : 1000Hz (1ms마다 증가)
 * Period    : 500ms
 *
 * [USART3]
 * Baudrate  : 115200bps, 8N1
 * TX        : PD8, AF7, ST-LINK VCP
 *
 * [특징]
 * - HAL 사용 안 함
 * - TIM2 / TIM7 사용 안 함
 * - TIM6 : 16-bit Up Counter
 * - Polling과 Interrupt를 동시에 학습
 *
 * [주의]
 * TIM6_DAC_IRQHandler()가 이 파일 또는 별도의
 * 인터럽트 파일에서 TIM6_UserHandler()를 호출해야 함.
 * ============================================================
 */


/* ============================================================
 * 1. Peripheral Base Address
 * ============================================================ */

#define RCC_BASE       0x40023800UL
#define GPIOD_BASE     0x40020C00UL
#define USART3_BASE    0x40004800UL
#define TIM6_BASE      0x40001000UL

#define NVIC_ISER1     (*(volatile uint32_t *)0xE000E104UL)


/* ============================================================
 * 2. Register Definitions
 * ============================================================ */

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


/* ============================================================
 * 3. Clock Initialization
 * ============================================================
 *
 * [Clock Tree - 현재 코드의 실제 설정]
 *
 *              HSI 16MHz
 *                  |
 *                  v
 *             SYSCLK 16MHz
 *                  |
 *                  v
 *          AHB Prescaler /1
 *                  |
 *                  v
 *              HCLK 16MHz
 *                  |
 *          +-------+-------+
 *          |               |
 *          v               v
 *      APB1 /1          APB2 /1
 *          |               |
 *          v               v
 *     PCLK1 16MHz     PCLK2 16MHz
 *          |
 *     +----+----+
 *     |         |
 *     v         v
 *   USART3     TIM6
 *   16MHz    TIM6CLK 16MHz
 *
 * ※ TIMPRE = 0인 경우:
 *   APB1 /1 → TIM6CLK = PCLK1
 *   APB1 /2 이상 → TIM6CLK = PCLK1 × 2
 *
 * 현재는 APB1 /1이므로 TIM6CLK = 16MHz.
 * ============================================================ */

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


/* ============================================================
 * 4. USART3 Initialization
 * ============================================================
 *
 * GPIOD : AHB1 Bus
 * USART3: APB1 Bus
 *
 * PD8 → Alternate Function 7 (USART3_TX)
 * PCLK1 = 16MHz
 * Baudrate = 115200bps
 * ============================================================ */

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


/* ============================================================
 * 5. USART3 Character Transmission
 * ============================================================
 *
 * TXE (Transmit Data Register Empty)
 * = 1이면 데이터 레지스터에 새 데이터 기록 가능
 * ============================================================ */

void USART3_SendChar(char c)
{
    // TXE(bit7) = 1이 될 때까지 대기
    while ((USART3_SR & (1U << 7)) == 0U)
    {
    }

    // 문자 전송 데이터 기록
    USART3_DR = (uint8_t)c;
}


/* ============================================================
 * 6. TIM6 Initialization - 500ms
 * ============================================================
 *
 * [TIM6 Clock]
 *
 * HSI 16MHz
 *     |
 *     v
 * SYSCLK 16MHz
 *     |
 *     v
 * AHB /1 → HCLK 16MHz
 *     |
 *     v
 * APB1 /1 → PCLK1 16MHz
 *     |
 *     v
 * TIM6CLK 16MHz
 *     |
 *     v
 * PSC = 15999
 *     |
 *     v
 * Counter Clock = 1000Hz
 *     |
 *     v
 * CNT가 1ms마다 증가
 *     |
 *     v
 * ARR = 499
 *     |
 *     v
 * 500ms마다 Update Event
 *
 * [공식 1 - Counter Clock]
 *
 * f_CNT = TIM6CLK / (PSC + 1)
 *
 *       = 16,000,000 / (15999 + 1)
 *       = 1000Hz
 *
 * → 1초에 1000카운트
 * → 1카운트 = 1ms
 *
 * [공식 2 - Update Period]
 *
 * T = (PSC + 1) × (ARR + 1) / TIM6CLK
 *
 *   = 16000 × 500 / 16,000,000
 *   = 0.5초
 *   = 500ms
 *
 * [Up Counter]
 *
 * 시간        CNT
 * ----------------
 *   0ms         0
 *   1ms         1
 *   2ms         2
 *   ...
 * 498ms       498
 * 499ms       499
 * 500ms         0  ← Update Event
 *
 * ============================================================ */

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


/* ============================================================
 * 7. TIM6 Update Interrupt
 * ============================================================
 *
 * 500ms 경과
 *     |
 *     v
 * CNT: 499 → 0
 *     |
 *     v
 * Update Event 발생
 *     |
 *     v
 * TIM6_SR.UIF = 1
 *     |
 *     v
 * TIM6_DIER.UIE = 1
 *     |
 *     v
 * NVIC → TIM6_DAC_IRQHandler()
 *     |
 *     v
 * TIM6_UserHandler()
 *     |
 *     v
 * '@' 출력
 * ============================================================ */

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


/* ============================================================
 * 8. Main - TIM6 CNT Polling
 * ============================================================
 *
 * Polling은 TIM6_CNT 레지스터를 반복해서 읽는다.
 *
 * 이전 CNT와 현재 CNT를 비교하여
 * 카운터가 다시 0으로 돌아왔는지 판단한다.
 *
 * previous_cnt = 499
 * current_cnt  = 0
 *
 * 0 < 499 → 참 → '!' 출력
 *
 * 주의:
 * CNT를 충분히 자주 읽어야 하며,
 * 너무 오래 읽지 않으면 주기 완료를 놓칠 수 있다.
 * ============================================================ */

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
