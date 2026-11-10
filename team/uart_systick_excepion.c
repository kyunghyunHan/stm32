
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

// Exception Handler에서 증가
// 프로그램 시작 후 경과한 시간을 ms 단위로 기록
volatile uint32_t systick_ms = 0;

void Clock_Init(void)
{
    // HSI Enable
    // 내부 고속 오실레이터(HSI) 활성화
    RCC_CR |= (1U << 0);

    // HSI 클럭이 준비될 때까지 대기
    while ((RCC_CR & (1U << 1)) == 0U)
    {
    }

    // SYSCLK = HSI
    RCC_CFGR &= ~(3U << 0);

    while ((RCC_CFGR & (3U << 2)) != 0U)
    {
    }

    // AHB = /1
    // SYSCLK
    RCC_CFGR &= ~(0xFU << 4);

    // APB1과 APB2 버스의 클럭 분주비를 1로 설정
    // CPU에서 사용하는 클럭을 주변장치에 몇 분의 1 속도로 전달할지 설정

    /*
     * 설정 비트   분주비   HCLK 16MHz 기준
     * 0xx         /1       16MHz
     * 100         /2       8MHz
     * 101         /4       4MHz
     * 110         /8       2MHz
     * 111         /16      1MHz
     */

    // APB1 = /1
    RCC_CFGR &= ~(7U << 10);

    // APB2 = /1
    RCC_CFGR &= ~(7U << 13);
}

void USART3_Init(void)
{
    // GPIOD Clock Enable
    RCC_AHB1ENR |= (1U << 3);

    // USART3 Clock Enable
    RCC_APB1ENR |= (1U << 18);

    // PD8 Alternate Function (주변장치 활성화)
    GPIOD_MODER &= ~(3U << 16);
    GPIOD_MODER |=  (2U << 16);

    // Push-Pull
    GPIOD_OTYPER &= ~(1U << 8);

    // AF7: GPIOD 포트의 상위 핀(PD8~PD15) 부가 기능 설정
    GPIOD_AFRH &= ~(0xFU << 0);
    GPIOD_AFRH |=  (7U << 0);

    // USART3_CR1: USART 비활성화, TX/RX 비활성화, 8비트 데이터,
    // 패리티 없음, USART 인터럽트 비활성화
    USART3_CR1 = 0U;

    // USART3_CR2: Stop Bit 1개, 동기식 클럭 비활성화,
    // LIN 및 특수 통신 모드 비활성화
    USART3_CR2 = 0U;

    // USART3_CR3: RTS/CTS 흐름 제어 비활성화, DMA 비활성화,
    // Half-Duplex, Smartcard, IrDA 등 특수 모드 비활성화
    USART3_CR3 = 0U;

    // PCLK1 = 16MHz, Baud = 115200
    USART3_BRR = 0x008BU;

    // USART Enable + TX Enable
    USART3_CR1 = (1U << 13) | (1U << 3);
}

void USART3_SendChar(char c)
{
    while ((USART3_SR & (1U << 7)) == 0U)
    {
    }

    USART3_DR = (uint8_t)c;
}

void SysTick_Init(void)
{
    // SysTick 제어 레지스터 초기화 (타이머 및 인터럽트 비활성화)
    SYST_CSR = 0U;

    // Reload 설정: 16MHz / 1000 = 16,000클럭 → 1ms
    // LOAD는 실제 카운트 횟수보다 1 작게 설정
    SYST_RVR = 16000U - 1U;

    // 현재 카운터 값 초기화 (카운터 및 COUNTFLAG 초기화)
    SYST_CVR = 0U;

    // CLKSOURCE(Bit 2) = 1 : CPU 클럭(HCLK) 사용
    // TICKINT  (Bit 1) = 1 : SysTick Exception 활성화
    // ENABLE   (Bit 0) = 1 : SysTick 카운터 시작
    SYST_CSR = (1U << 2) |
               (1U << 1) |
               (1U << 0);
}

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
