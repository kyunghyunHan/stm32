
#include <stdint.h>



#define REG32(addr) (*(volatile uint32_t *)(addr))
#define REG8(addr)  (*(volatile uint8_t *)(addr))

#define RCC_BASE     0x40023800UL
#define GPIOD_BASE   0x40020C00UL
#define USART3_BASE  0x40004800UL
#define TIM6_BASE    0x40001000UL
#define SCB_BASE     0xE000ED00UL

// RCC
#define RCC_CR       REG32(RCC_BASE + 0x00UL)
#define RCC_CFGR     REG32(RCC_BASE + 0x08UL)
#define RCC_AHB1ENR  REG32(RCC_BASE + 0x30UL)
#define RCC_APB1ENR  REG32(RCC_BASE + 0x40UL)

// GPIOD
#define GPIOD_MODER  REG32(GPIOD_BASE + 0x00UL)
#define GPIOD_AFRH   REG32(GPIOD_BASE + 0x24UL)

// USART3
#define USART3_SR    REG32(USART3_BASE + 0x00UL)
#define USART3_DR    REG32(USART3_BASE + 0x04UL)
#define USART3_BRR   REG32(USART3_BASE + 0x08UL)
#define USART3_CR1   REG32(USART3_BASE + 0x0CUL)

// TIM6
#define TIM6_CR1     REG32(TIM6_BASE + 0x00UL)
#define TIM6_DIER    REG32(TIM6_BASE + 0x0CUL)
#define TIM6_SR      REG32(TIM6_BASE + 0x10UL)
#define TIM6_EGR     REG32(TIM6_BASE + 0x14UL)
#define TIM6_CNT     REG32(TIM6_BASE + 0x24UL)
#define TIM6_PSC     REG32(TIM6_BASE + 0x28UL)
#define TIM6_ARR     REG32(TIM6_BASE + 0x2CUL)

// NVIC
#define NVIC_ISER1   REG32(0xE000E104UL)
#define NVIC_ICER1   REG32(0xE000E184UL)
#define NVIC_ISPR1   REG32(0xE000E204UL)
#define NVIC_ICPR1   REG32(0xE000E284UL)
#define NVIC_IABR1   REG32(0xE000E304UL)
#define NVIC_IPR(n)  REG8(0xE000E400UL + (n))

// SCB
#define SCB_AIRCR    REG32(SCB_BASE + 0x0CUL)

#define TIM6_IRQ_BIT (1UL << 22)  // IRQ54
#define TIM7_IRQ_BIT (1UL << 23)  // IRQ55

volatile uint32_t tim6_done = 0;
volatile uint32_t tim7_count = 0;

/* ================= INIT ================= */

static void clock_init(void)
{
    // HSI 16MHz 사용
    RCC_CR |= (1U << 0);
    while (!(RCC_CR & (1U << 1)));

    RCC_CFGR &= ~(3U << 0);
    while ((RCC_CFGR & (3U << 2)) != 0U);

    // AHB /1, APB1 /1
    RCC_CFGR &= ~(0xFU << 4);
    RCC_CFGR &= ~(7U << 10);
}

static void uart3_init(void)
{
    // GPIOD, USART3 Clock Enable
    RCC_AHB1ENR |= (1U << 3);
    RCC_APB1ENR |= (1U << 18);

    // PD8 AF7
    GPIOD_MODER &= ~(3U << 16);
    GPIOD_MODER |=  (2U << 16);

    GPIOD_AFRH &= ~(0xFU << 0);
    GPIOD_AFRH |=  (7U << 0);

    USART3_CR1 = 0;
    USART3_BRR = 0x008BU;

    // TE=1, UE=1
    USART3_CR1 = (1U << 3) | (1U << 13);
}

static void tim6_init(void)
{
    RCC_APB1ENR |= (1U << 4);

    TIM6_CR1 = 0;
    TIM6_PSC = 1600U - 1U;
    TIM6_ARR = 10000U - 1U;
    TIM6_CNT = 0;

    // Update Generation → UIF Clear
    TIM6_EGR = (1U << 0);
    TIM6_SR = 0;

    TIM6_DIER = (1U << 0);
}

static void nvic_init(void)
{
    // PRIGROUP=3 (4개 선점 우선순위 비트)
    uint32_t aircr = SCB_AIRCR;
    SCB_AIRCR = (0x5FAUL << 16)
              | (aircr & 0x0000F8FFUL)
              | (3UL << 8);

    // TIM6 IRQ54 Priority 10, TIM7 IRQ55 Priority 5
    NVIC_IPR(54U) = (10U << 4);
    NVIC_IPR(55U) = (5U << 4);

    // Pending Clear
    NVIC_ICPR1 = TIM6_IRQ_BIT | TIM7_IRQ_BIT;

    // TIM6 Enable, TIM7 Disable
    NVIC_ISER1 = TIM6_IRQ_BIT;
    NVIC_ICER1 = TIM7_IRQ_BIT;
}

/* ================= UART OUTPUT ================= */

// 문자열 전송
static void uart_print(const char *s)
{
    while (*s)
    {
        while (!(USART3_SR & (1U << 7)));
        USART3_DR = (uint8_t)*s++;
    }
}

// 숫자 전송
static void uart_num(uint32_t n)
{
    char buf[10];
    uint32_t i = 0;

    if (n == 0)
    {
        while (!(USART3_SR & (1U << 7)));
        USART3_DR = '0';
        return;
    }

    while (n)
    {
        buf[i++] = '0' + (n % 10U);
        n /= 10U;
    }

    while (i)
    {
        while (!(USART3_SR & (1U << 7)));
        USART3_DR = (uint8_t)buf[--i];
    }
}

/* ================= INTERRUPT HANDLERS ================= */

void TIM7_IRQHandler(void)
{
    uint32_t ipsr;
    uint32_t msp;

    tim7_count++;

    __asm volatile("mrs %0, ipsr" : "=r"(ipsr));
    __asm volatile("mrs %0, msp"  : "=r"(msp));

    uart_print("\r\n[TIM7 ISR] ENTER\r\n");

    uart_print("TIM7 Active = ");
    uart_num((NVIC_IABR1 & TIM7_IRQ_BIT) != 0U);

    uart_print("\r\nTIM6 Active = ");
    uart_num((NVIC_IABR1 & TIM6_IRQ_BIT) != 0U);

    uart_print("\r\nIPSR = ");
    uart_num(ipsr);

    uart_print("\r\nMSP = ");
    uart_num(msp);

    uart_print("\r\n[TIM7 ISR] EXIT\r\n");
}

void TIM6_DAC_IRQHandler(void)
{
    if (TIM6_SR & (1U << 0))
    {
        uint32_t ipsr;

        // UIF Clear, Timer Stop
        TIM6_SR = 0;
        TIM6_CR1 &= ~(1U << 0);

        __asm volatile("mrs %0, ipsr" : "=r"(ipsr));

        uart_print("\r\n[TIM6 ISR] ENTER\r\n");

        uart_print("TIM6 Active = ");
        uart_num((NVIC_IABR1 & TIM6_IRQ_BIT) != 0U);

        uart_print("\r\nTIM6 Priority = ");
        uart_num(NVIC_IPR(54U) >> 4);

        uart_print("\r\nIPSR = ");
        uart_num(ipsr);

        uart_print("\r\n[TIM6] Trigger TIM7 software IRQ\r\n");

        // TIM7 Pending 설정 → TIM6 선점
        NVIC_ISPR1 = TIM7_IRQ_BIT;

        uart_print("[TIM6] Resumed after TIM7\r\n");

        tim6_done = 1;

        uart_print("[TIM6 ISR] EXIT\r\n");
    }
}

/* ================= MAIN ================= */

int main(void)
{
    uint32_t primask;

    clock_init();
    uart3_init();
    tim6_init();
    nvic_init();

    __asm volatile("cpsie i" ::: "memory");

    uart_print("\r\n=== NVIC REGISTER LAB ===\r\n");

    /* TEST 1: Enable / Disable / Pending */

    uart_print("\r\n[TEST 1] Enable / Disable / Pending\r\n");

    NVIC_ICER1 = TIM7_IRQ_BIT;  // TIM7 IRQ Disable
    NVIC_ICPR1 = TIM7_IRQ_BIT;  // TIM7 Pending Clear
    NVIC_ISPR1 = TIM7_IRQ_BIT;  // TIM7 Pending Set

    uart_print("TIM7 Enable = ");
    uart_num((NVIC_ISER1 & TIM7_IRQ_BIT) != 0U);

    uart_print("\r\nTIM7 Pending = ");
    uart_num((NVIC_ISPR1 & TIM7_IRQ_BIT) != 0U);

    uart_print("\r\nEnable TIM7 IRQ\r\n");

    NVIC_ISER1 = TIM7_IRQ_BIT;  // Pending TIM7 Handler 실행

    uart_print("Returned to main\r\n");

    /* TEST 2: PRIMASK */

    uart_print("\r\n[TEST 2] PRIMASK\r\n");

    __asm volatile("cpsid i" ::: "memory");

    NVIC_ISPR1 = TIM7_IRQ_BIT;

    __asm volatile("mrs %0, primask" : "=r"(primask));

    uart_print("PRIMASK = ");
    uart_num(primask);

    uart_print("\r\nTIM7 Pending = ");
    uart_num((NVIC_ISPR1 & TIM7_IRQ_BIT) != 0U);

    uart_print("\r\nClear PRIMASK\r\n");

    __asm volatile("cpsie i" ::: "memory");

    uart_print("Returned after PRIMASK test\r\n");

    /* TEST 3: Priority */

    uart_print("\r\n[TEST 3] Priority\r\n");

    uart_print("TIM6 Priority = ");
    uart_num(NVIC_IPR(54U) >> 4);

    uart_print("\r\nTIM7 Priority = ");
    uart_num(NVIC_IPR(55U) >> 4);

    uart_print("\r\n");

    /* TEST 4: Active / Preemption / Nested */

    uart_print("\r\n[TEST 4] Start TIM6\r\n");

    TIM6_CR1 |= (1U << 0);  // TIM6 Start

    while (!tim6_done)
    {
        __asm volatile("wfi" ::: "memory");
    }

    uart_print("\r\n[MAIN] TIM6 Completed\r\n");

    uart_print("TIM7 ISR Count = ");
    uart_num(tim7_count);

    uart_print("\r\nTIM6 Active = ");
    uart_num((NVIC_IABR1 & TIM6_IRQ_BIT) != 0U);

    uart_print("\r\nTIM7 Active = ");
    uart_num((NVIC_IABR1 & TIM7_IRQ_BIT) != 0U);

    uart_print("\r\n=== NVIC LAB END ===\r\n");

    while (1)
    {
        __asm volatile("wfi" ::: "memory");
    }
}
