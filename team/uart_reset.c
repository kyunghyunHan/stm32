
#include <stdint.h>

/*
 * STM32F429ZI - Reset Cause Monitor
 *
 * HAL 사용 안 함 (Register Only)
 *
 * Clock : HSI 16MHz
 * PCLK1 : 16MHz
 *
 * USART3 : 115200bps, 8N1
 * TX : PD8 (AF7, ST-LINK VCP)
 * RX : PD9 (AF7, ST-LINK VCP)
 *
 * 기능
 * 1. 부팅 직후 RCC_CSR 저장
 * 2. Reset Flag 초기화
 * 3. UART로 Reset 원인 출력
 * 4. 'r' 입력 시 Software Reset
 * 5. 's' 입력 시 저장된 Reset 원인 다시 출력
 */

// ============================================================
// 1. Base Address
// ============================================================

#define RCC_BASE       0x40023800UL
#define GPIOD_BASE     0x40020C00UL
#define USART3_BASE    0x40004800UL
#define SCB_AIRCR_ADDR 0xE000ED0CUL

#define REG32(addr) (*(volatile uint32_t *)(addr))

// ============================================================
// 2. RCC Registers
// ============================================================

#define RCC_CR      REG32(RCC_BASE + 0x00UL)
#define RCC_CFGR    REG32(RCC_BASE + 0x08UL)
#define RCC_AHB1ENR REG32(RCC_BASE + 0x30UL)
#define RCC_APB1ENR REG32(RCC_BASE + 0x40UL)
#define RCC_CSR     REG32(RCC_BASE + 0x74UL)

// ============================================================
// 3. GPIO Registers
// ============================================================

#define GPIOD_MODER   REG32(GPIOD_BASE + 0x00UL)
#define GPIOD_OTYPER  REG32(GPIOD_BASE + 0x04UL)
#define GPIOD_OSPEEDR REG32(GPIOD_BASE + 0x08UL)
#define GPIOD_PUPDR   REG32(GPIOD_BASE + 0x0CUL)
#define GPIOD_AFRH    REG32(GPIOD_BASE + 0x24UL)

// ============================================================
// 4. USART3 Registers
// ============================================================

#define USART3_SR  REG32(USART3_BASE + 0x00UL)
#define USART3_DR  REG32(USART3_BASE + 0x04UL)
#define USART3_BRR REG32(USART3_BASE + 0x08UL)
#define USART3_CR1 REG32(USART3_BASE + 0x0CUL)
#define USART3_CR2 REG32(USART3_BASE + 0x10UL)
#define USART3_CR3 REG32(USART3_BASE + 0x14UL)

// ============================================================
// 5. Reset Flags
// ============================================================

#define LPWRRSTF  (1UL << 31)
#define WWDGRSTF  (1UL << 30)
#define IWDGRSTF  (1UL << 29)
#define SFTRSTF   (1UL << 28)
#define PORRSTF   (1UL << 27)
#define PINRSTF   (1UL << 26)
#define BORRSTF   (1UL << 25)
#define RMVF      (1UL << 24)

#define RESET_FLAGS_MASK (0xFE000000UL)

// ============================================================
// 6. Reset Cause 저장
// ============================================================

static uint32_t reset_cause;

static void Reset_Cause_Capture(void)
{
    // Reset 원인 플래그 읽기
    reset_cause = RCC_CSR & RESET_FLAGS_MASK;

    // 과거 Reset 플래그 제거
    RCC_CSR |= RMVF;
}

// ============================================================
// 7. Clock 초기화
// ============================================================

static void Clock_Init(void)
{
    // HSI 16MHz 활성화
    RCC_CR |= (1UL << 0);

    // HSIRDY 대기
    while (!(RCC_CR & (1UL << 1)))
    {
    }

    // SYSCLK을 HSI로 선택
    RCC_CFGR &= ~(3UL << 0);

    // SWS = 00 확인
    while (((RCC_CFGR >> 2) & 3UL) != 0UL)
    {
    }

    // AHB / APB1 / APB2 분주비 = 1
    RCC_CFGR &= ~((0xFUL << 4) |
                  (7UL << 10) |
                  (7UL << 13));
}

// ============================================================
// 8. USART3 초기화
// ============================================================

static void UART3_Init(void)
{
    // GPIOD 클럭 활성화
    RCC_AHB1ENR |= (1UL << 3);

    // USART3 클럭 활성화
    RCC_APB1ENR |= (1UL << 18);

    // PD8, PD9 Alternate Function 모드 (10)
    GPIOD_MODER &= ~((3UL << 16) |
                     (3UL << 18));

    GPIOD_MODER |=  ((2UL << 16) |
                     (2UL << 18));

    // Push-Pull
    GPIOD_OTYPER &= ~((1UL << 8) |
                      (1UL << 9));

    // High Speed
    GPIOD_OSPEEDR |= ((3UL << 16) |
                      (3UL << 18));

    // PD8, PD9 Pull-Up
    GPIOD_PUPDR &= ~((3UL << 16) |
                     (3UL << 18));

    GPIOD_PUPDR |= ((1UL << 16) |
                     (1UL << 18));

    // AF7 = USART3
    GPIOD_AFRH &= ~((0xFUL << 0) |
                    (0xFUL << 4));

    GPIOD_AFRH |=  ((7UL << 0) |
                    (7UL << 4));

    // USART 비활성화 후 설정
    USART3_CR1 = 0;
    USART3_CR2 = 0;
    USART3_CR3 = 0;

    // 16MHz / 115200bps, OVER8 = 0
    USART3_BRR = 0x008B;

    // UE(bit13), TE(bit3), RE(bit2)
    USART3_CR1 = (1UL << 13) |
                 (1UL << 3)  |
                 (1UL << 2);
}

// ============================================================
// 9. UART 송수신
// ============================================================

static void UART3_SendChar(char c)
{
    // TXE = 1 대기
    while (!(USART3_SR & (1UL << 7)))
    {
    }

    USART3_DR = (uint8_t)c;
}

static void UART3_SendString(const char *str)
{
    while (*str)
    {
        UART3_SendChar(*str++);
    }
}

static void UART3_SendHex(uint32_t value)
{
    const char hex[] = "0123456789ABCDEF";

    UART3_SendString("0x");

    for (int shift = 28; shift >= 0; shift -= 4)
    {
        UART3_SendChar(hex[(value >> shift) & 0xFUL]);
    }
}

static char UART3_ReadChar(void)
{
    // RXNE = 1 대기
    while (!(USART3_SR & (1UL << 5)))
    {
    }

    return (char)(USART3_DR & 0xFFUL);
}

static void UART3_WaitComplete(void)
{
    // TC = 1: 마지막 비트까지 전송 완료
    while (!(USART3_SR & (1UL << 6)))
    {
    }
}

// ============================================================
// 10. Reset 원인 출력
// ============================================================

static void Reset_Cause_Print(void)
{
    UART3_SendString("\r\n");
    UART3_SendString("===== RESET CAUSE =====\r\n");

    UART3_SendString("RCC_CSR FLAGS = ");
    UART3_SendHex(reset_cause);
    UART3_SendString("\r\n");

    if (reset_cause & LPWRRSTF)
        UART3_SendString("[1] Low Power Reset\r\n");

    if (reset_cause & WWDGRSTF)
        UART3_SendString("[1] WWDG Reset\r\n");

    if (reset_cause & IWDGRSTF)
        UART3_SendString("[1] IWDG Reset\r\n");

    if (reset_cause & SFTRSTF)
        UART3_SendString("[1] Software Reset\r\n");

    if (reset_cause & PORRSTF)
        UART3_SendString("[1] POR/PDR Reset\r\n");

    if (reset_cause & PINRSTF)
        UART3_SendString("[1] NRST Pin Reset\r\n");

    if (reset_cause & BORRSTF)
        UART3_SendString("[1] BOR or POR/PDR Reset\r\n");

    if (reset_cause == 0UL)
        UART3_SendString("No reset flag detected\r\n");

    UART3_SendString("=======================\r\n");
}

// ============================================================
// 11. Software Reset (AIRCR)
// ============================================================

static void Software_Reset(void)
{
    UART3_SendString("\r\nSoftware Reset...\r\n");

    UART3_WaitComplete();

    // AIRCR: VECTKEY + PRIGROUP 보존 + SYSRESETREQ
    uint32_t aircr = REG32(SCB_AIRCR_ADDR);

    REG32(SCB_AIRCR_ADDR) =
        (0x5FAUL << 16) |
        (aircr & (7UL << 8)) |
        (1UL << 2);

    __asm volatile ("dsb" ::: "memory");

    while (1)
    {
    }
}

// ============================================================
// 12. main
// ============================================================

int main(void)
{
    // Reset 원인은 최대한 먼저 저장
    Reset_Cause_Capture();

    Clock_Init();
    UART3_Init();

    Reset_Cause_Print();

    UART3_SendString("\r\nCommands:\r\n");
    UART3_SendString("r : Software Reset\r\n");
    UART3_SendString("s : Show Reset Cause\r\n");

    while (1)
    {
        char c = UART3_ReadChar();

        if (c == 'r' || c == 'R')
        {
            Software_Reset();
        }
        else if (c == 's' || c == 'S')
        {
            Reset_Cause_Print();
        }
    }
}
