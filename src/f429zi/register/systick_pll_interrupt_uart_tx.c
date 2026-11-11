
#include <stdint.h>

/*
 * STM32F429ZI
 *
 * SysTick Polling   : 1000ms마다 '.'
 * SysTick Exception : 500ms마다 '*'
 *
 * HSI     : 16MHz
 * PLL     : 168MHz
 * SYSCLK  : 168MHz
 * HCLK    : 168MHz
 * PCLK1   : 42MHz
 * PCLK2   : 84MHz
 *
 * SysTick Clock : HCLK / 8 = 21MHz
 * SysTick Tick  : 1ms
 *
 * USART3 : 115200bps, 8N1
 * TX     : PD8, AF7, ST-LINK VCP
 *
 * HAL 함수 사용 안 함
 * TIM2 / TIM6 사용 안 함
 *
 * 3.3V 전원 환경 기준
 */

// =====================================
// 1. Base Address
// =====================================

#define RCC_BASE       0x40023800UL
#define GPIOD_BASE     0x40020C00UL
#define USART3_BASE    0x40004800UL
#define SYSTICK_BASE   0xE000E010UL
#define PWR_BASE       0x40007000UL
#define FLASH_REG_BASE 0x40023C00UL

// RCC
#define RCC_CR         (*(volatile uint32_t *)(RCC_BASE + 0x00UL))
#define RCC_PLLCFGR    (*(volatile uint32_t *)(RCC_BASE + 0x04UL))
#define RCC_CFGR       (*(volatile uint32_t *)(RCC_BASE + 0x08UL))
#define RCC_AHB1ENR    (*(volatile uint32_t *)(RCC_BASE + 0x30UL))
#define RCC_APB1ENR    (*(volatile uint32_t *)(RCC_BASE + 0x40UL))

// PWR
#define PWR_CR         (*(volatile uint32_t *)(PWR_BASE + 0x00UL))

// FLASH
#define FLASH_ACR      (*(volatile uint32_t *)(FLASH_REG_BASE + 0x00UL))

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
#define STK_CTRL       (*(volatile uint32_t *)(SYSTICK_BASE + 0x00UL))
#define STK_LOAD       (*(volatile uint32_t *)(SYSTICK_BASE + 0x04UL))
#define STK_VAL        (*(volatile uint32_t *)(SYSTICK_BASE + 0x08UL))

// Clock
#define CPU_CLOCK_HZ   168000000UL
#define SYSTICK_HZ     (CPU_CLOCK_HZ / 8U)

// SysTick Exception에서 1ms마다 증가
volatile uint32_t systick_ms = 0U;


// =====================================
// 2. Clock Initialization - 168MHz
// =====================================

void Clock_Init(void)
{
    // 1. HSI 16MHz 활성화
    RCC_CR |= (1U << 0);

    // HSIRDY 확인
    while ((RCC_CR & (1U << 1)) == 0U)
    {
    }

    // 2. SYSCLK = HSI로 우선 전환
    RCC_CFGR &= ~(3U << 0);

    while ((RCC_CFGR & (3U << 2)) != 0U)
    {
    }

    // 3. 기존 PLL 비활성화
    RCC_CR &= ~(1U << 24);

    // PLL 완전히 정지할 때까지 대기
    while ((RCC_CR & (1U << 25)) != 0U)
    {
    }

    // 4. PWR Clock Enable
    RCC_APB1ENR |= (1U << 28);

    // Voltage Scale 1
    PWR_CR = (PWR_CR & ~(3U << 14))
           | (3U << 14);

    // 5. FLASH Wait State = 5
    // Prefetch + Instruction Cache + Data Cache
    FLASH_ACR = (FLASH_ACR & ~0xFU)
              | (1U << 8)
              | (1U << 9)
              | (1U << 10)
              | 5U;

    while ((FLASH_ACR & 0xFU) != 5U)
    {
    }

    // 6. Bus Prescaler
    // AHB  /1 -> HCLK  = 168MHz
    // APB1 /4 -> PCLK1 = 42MHz
    // APB2 /2 -> PCLK2 = 84MHz
    RCC_CFGR =
        (RCC_CFGR & ~((0xFU << 4) |
                      (7U << 10) |
                      (7U << 13)))
        | (5U << 10)
        | (4U << 13);

    // 7. PLL Configuration
    //
    // PLLM = 16
    // PLLN = 336
    // PLLP = 2
    //
    // 16MHz / 16 * 336 / 2 = 168MHz
    RCC_PLLCFGR =
          (16U  << 0)
        | (336U << 6)
        | (0U   << 16)
        | (0U   << 22)
        | (7U   << 24);

    // 8. PLL 활성화
    RCC_CR |= (1U << 24);

    while ((RCC_CR & (1U << 25)) == 0U)
    {
    }

    // 9. SYSCLK = PLL
    RCC_CFGR =
        (RCC_CFGR & ~(3U << 0)) | (2U << 0);

    // 10. SYSCLK 전환 완료 확인
    while ((RCC_CFGR & (3U << 2)) != (2U << 2))
    {
    }
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

    // PD8 Alternate Function Mode (10)
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

    // PCLK1 = 42MHz
    // Baud = 115200bps
    USART3_BRR = 0x016DU;

    // UE = 1, TE = 1
    USART3_CR1 = (1U << 13) | (1U << 3);
}


// =====================================
// 4. UART Transmit
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
// 5. SysTick Initialization
// =====================================

void SysTick_Init(void)
{
    // SysTick Disable
    STK_CTRL = 0U;

    /*
     * HCLK = 168MHz
     *
     * CLKSOURCE = 0 : HCLK / 8
     * 168MHz / 8 = 21MHz
     *
     * 21,000,000 / 1000 = 21,000 clocks
     *
     * 1ms = 21,000 clocks
     *
     * LOAD = 21,000 - 1
     */

    STK_LOAD = (SYSTICK_HZ / 1000U) - 1U;

    // Current Value Clear
    STK_VAL = 0U;

    // CLKSOURCE(Bit 2) = 0 : HCLK/8
    // TICKINT  (Bit 1) = 1 : Exception Enable
    // ENABLE   (Bit 0) = 1 : Counter Enable
    STK_CTRL = (1U << 1) |
               (1U << 0);
}


// =====================================
// 6. SysTick Exception User Handler
// =====================================

// stm32f4xx_it.c의 SysTick_Handler에서 호출
void SysTick_UserHandler(void)
{
    static uint32_t exception_count = 0U;

    // 1ms 경과
    systick_ms++;

    exception_count++;

    // 500ms마다 '*' 출력
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
