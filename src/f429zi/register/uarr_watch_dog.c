
#include <stdint.h>

#define RCC_BASE 0x40023800UL
#define GPIOD_BASE 0x40020C00UL
#define USART3_BASE 0x40004800UL
#define IWDG_BASE 0x40003000UL

// Cortex-M4 SysTick
#define SYST_CSR (*(volatile uint32_t *)0xE000E010UL)
#define SYST_RVR (*(volatile uint32_t *)0xE000E014UL)
#define SYST_CVR (*(volatile uint32_t *)0xE000E018UL)

#define RCC_AHB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x30UL))
#define RCC_APB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x40UL))
#define RCC_CSR (*(volatile uint32_t *)(RCC_BASE + 0x74UL))

// RCC Reset Flags
#define RCC_IWDGRSTF (1UL << 29)
#define RCC_RMVF (1UL << 24)

#define GPIOD_MODER (*(volatile uint32_t *)(GPIOD_BASE + 0x00UL))
#define GPIOD_AFRH (*(volatile uint32_t *)(GPIOD_BASE + 0x24UL))

#define USART3_SR (*(volatile uint32_t *)(USART3_BASE + 0x00UL))
#define USART3_DR (*(volatile uint32_t *)(USART3_BASE + 0x04UL))
#define USART3_BRR (*(volatile uint32_t *)(USART3_BASE + 0x08UL))
#define USART3_CR1 (*(volatile uint32_t *)(USART3_BASE + 0x0CUL))
#define USART3_CR2 (*(volatile uint32_t *)(USART3_BASE + 0x10UL))
#define USART3_CR3 (*(volatile uint32_t *)(USART3_BASE + 0x14UL))

#define IWDG_KR (*(volatile uint32_t *)(IWDG_BASE + 0x00UL))
#define IWDG_PR (*(volatile uint32_t *)(IWDG_BASE + 0x04UL))
#define IWDG_RLR (*(volatile uint32_t *)(IWDG_BASE + 0x08UL))
#define IWDG_SR (*(volatile uint32_t *)(IWDG_BASE + 0x0CUL))

void uart3_init(void)
{
    // GPIOD clock enable
    RCC_AHB1ENR |= (1UL << 3);

    // USART3 clock enable
    RCC_APB1ENR |= (1UL << 18);

    // PD8, PD9 Alternate Function
    GPIOD_MODER &= ~((3UL << 16) | (3UL << 18));
    GPIOD_MODER |= ((2UL << 16) | (2UL << 18));

    // AF7 = USART3
    GPIOD_AFRH &= ~((0xFUL << 0) | (0xFUL << 4));
    GPIOD_AFRH |= ((7UL << 0) | (7UL << 4));

    // USART reset configuration
    USART3_CR1 = 0;
    USART3_CR2 = 0;
    USART3_CR3 = 0;

    // 16MHz / 9600bps
    USART3_BRR = 1667UL;

    // UE, TE, RE enable
    USART3_CR1 = (1UL << 13) |
                 (1UL << 3) |
                 (1UL << 2);
}

void uart3_send_char(char c)
{
    // Wait TXE
    while ((USART3_SR & (1UL << 7)) == 0UL)
        ;

    USART3_DR = (uint8_t)c;
}

void uart3_send_string(const char *str)
{
    while (*str)
    {
        uart3_send_char(*str++);
    }

    // Wait for the final byte to finish
    while ((USART3_SR & (1UL << 6)) == 0UL)
        ;
}

void systick_init(void)
{
    // 16MHz / 1000 = 16000 cycles (1ms)
    SYST_CSR = 0;
    SYST_RVR = 16000UL - 1UL;
    SYST_CVR = 0;

    // CPU Clock, no interrupt, enable
    SYST_CSR = (1UL << 2) | (1UL << 0);
}

void delay_ms(uint32_t ms)
{
    while (ms--)
    {
        // Wait for COUNTFLAG (bit16)
        while ((SYST_CSR & (1UL << 16)) == 0UL)
            ;
    }
}

void iwdg_init(void)
{
    // 1. Start IWDG
    IWDG_KR = 0xCCCCUL;

    // 2. Unlock PR/RLR registers
    IWDG_KR = 0x5555UL;

    // 3. Prescaler = /64
    IWDG_PR = 4UL;

    // 4. Reload = 1499
    // Timeout = (1499+1)*64/32000
    //         = 3 seconds (nominal)
    IWDG_RLR = 1499UL;

    // 5. Wait for register updates
    // PVU(bit0), RVU(bit1)
    while (IWDG_SR & 0x03UL)
        ;

    // 6. Reload counter
    IWDG_KR = 0xAAAAUL;
}

// =============================================
// 11. Watchdog Refresh
// =============================================

void iwdg_refresh(void)
{
    // Refresh (Feed / Kick)
    IWDG_KR = 0xAAAAUL;
}

void print_reset_cause(void)
{
    uint32_t reset_flags = RCC_CSR;

    if (reset_flags & RCC_IWDGRSTF)
    {
        uart3_send_string(
            "[RESET] IWDG Watchdog Reset!\r\n");
    }
    else
    {
        uart3_send_string(
            "[RESET] No IWDG reset flag.\r\n");
    }

    // Clear reset flags
    RCC_CSR |= RCC_RMVF;
}

int main(void)
{
    uart3_init();
    systick_init();

    uart3_send_string("\r\n");
    uart3_send_string("========================\r\n");
    uart3_send_string(" STM32 Watchdog Test\r\n");
    uart3_send_string("========================\r\n");

    // Check previous reset reason
    print_reset_cause();

    uart3_send_string("[INFO] Starting IWDG...\r\n");

    // Start approximately 3-second watchdog
    iwdg_init();

    uart3_send_string("[IWDG] Started.\r\n");

    // Refresh 5 times, once per second
    for (int i = 1; i <= 5; i++)
    {
        delay_ms(1000);

        iwdg_refresh();

        uart3_send_string(
            "[IWDG] Refresh OK\r\n");
    }

    uart3_send_string("\r\n");
    uart3_send_string(
        "[WARNING] Stop refreshing!\r\n");

    uart3_send_string(
        "[WARNING] Waiting for reset...\r\n");

    // No more watchdog refresh
    while (1)
    {
        // IWDG resets the MCU automatically
    }
}
