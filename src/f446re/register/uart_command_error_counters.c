
#include <stdint.h>

/*
 * STM32 NUCLEO-F446RE
 *
 * USART2 : PA2 TX / PA3 RX (AF7)
 * LED1   : PB0  (External)
 * LED2   : PA5  (Onboard LD2)
 * LED3   : PB1  (External)
 * KEY1   : PC13 (Onboard B1)
 *
 * HSI = 16MHz, APB1 = 16MHz assumed
 * UART = 9600, 8N1
 *
 * KEY1:
 * Pressed  = HIGH
 * Released = LOW
 *
 * UART commands:
 * led1 on/off, led2 on/off, led3 on/off
 * 123 on, 123 off, off
 * key1, status, errors, clear
 */

#define RCC_BASE     0x40023800UL
#define GPIOA_BASE   0x40020000UL
#define GPIOB_BASE   0x40020400UL
#define GPIOC_BASE   0x40020800UL
#define USART2_BASE  0x40004400UL

#define REG(base, offset) \
    (*(volatile uint32_t *)((base) + (offset)))

// RCC
#define RCC_AHB1ENR REG(RCC_BASE, 0x30UL)
#define RCC_APB1ENR REG(RCC_BASE, 0x40UL)

// GPIOA
#define GPIOA_MODER REG(GPIOA_BASE, 0x00UL)
#define GPIOA_BSRR  REG(GPIOA_BASE, 0x18UL)
#define GPIOA_AFRL  REG(GPIOA_BASE, 0x20UL)

// GPIOB
#define GPIOB_MODER REG(GPIOB_BASE, 0x00UL)
#define GPIOB_BSRR  REG(GPIOB_BASE, 0x18UL)

// GPIOC
#define GPIOC_MODER REG(GPIOC_BASE, 0x00UL)
#define GPIOC_IDR   REG(GPIOC_BASE, 0x10UL)

// USART2
#define USART2_SR  REG(USART2_BASE, 0x00UL)
#define USART2_DR  REG(USART2_BASE, 0x04UL)
#define USART2_BRR REG(USART2_BASE, 0x08UL)
#define USART2_CR1 REG(USART2_BASE, 0x0CUL)
#define USART2_CR2 REG(USART2_BASE, 0x10UL)
#define USART2_CR3 REG(USART2_BASE, 0x14UL)

// SysTick
#define SYST_CSR (*(volatile uint32_t *)0xE000E010UL)
#define SYST_RVR (*(volatile uint32_t *)0xE000E014UL)
#define SYST_CVR (*(volatile uint32_t *)0xE000E018UL)

// Bit definitions
#define LED1 (1U << 0)
#define LED2 (1U << 5)
#define LED3 (1U << 1)

#define UART_PE   (1U << 0)
#define UART_FE   (1U << 1)
#define UART_NE   (1U << 2)
#define UART_ORE  (1U << 3)
#define UART_RXNE (1U << 5)
#define UART_TXE  (1U << 7)

#define UART_ERRORS \
    (UART_PE | UART_FE | UART_NE | UART_ORE)

// Status
static uint8_t led_state[3] = {0, 0, 0};
static uint32_t ore_count = 0;
static uint32_t fe_count  = 0;
static uint32_t ne_count  = 0;
static uint32_t pe_count  = 0;
static uint32_t buffer_error_count = 0;

// Button debounce
#define DEBOUNCE_MS 20U

static uint8_t key_stable = 0;
static uint8_t key_candidate = 0;
static uint32_t key_count = 0;

// =====================================
// 1. GPIO Initialization
// =====================================

void gpio_init(void)
{
    RCC_AHB1ENR |= (1U << 0) |
                   (1U << 1) |
                   (1U << 2);

    // PA5 output
    GPIOA_MODER &= ~(3U << 10);
    GPIOA_MODER |=  (1U << 10);

    // PB0, PB1 output
    GPIOB_MODER &= ~((3U << 0) | (3U << 2));
    GPIOB_MODER |=  ((1U << 0) | (1U << 2));

    // Initial LED OFF
    GPIOA_BSRR = (LED2 << 16);
    GPIOB_BSRR = ((LED1 | LED3) << 16);

    // PC13 input
    GPIOC_MODER &= ~(3U << 26);
}

// =====================================
// 2. USART2 Initialization
// =====================================

void uart2_init(void)
{
    RCC_AHB1ENR |= (1U << 0);
    RCC_APB1ENR |= (1U << 17);

    // PA2, PA3 alternate function
    GPIOA_MODER &= ~((3U << 4) | (3U << 6));
    GPIOA_MODER |=  ((2U << 4) | (2U << 6));

    // AF7
    GPIOA_AFRL &= ~((0xFU << 8) | (0xFU << 12));
    GPIOA_AFRL |=  ((7U << 8) | (7U << 12));

    USART2_CR1 = 0;
    USART2_CR2 = 0;
    USART2_CR3 = 0;

    // APB1 = 16MHz, 9600bps
    USART2_BRR = 1667U;

    // UE, TE, RE
    USART2_CR1 = (1U << 13) |
                 (1U << 3)  |
                 (1U << 2);
}

// =====================================
// 3. SysTick 1ms Initialization
// =====================================

void systick_init(void)
{
    // CPU clock = 16MHz
    // 16000 cycles = 1ms
    SYST_CSR = 0;
    SYST_RVR = 16000U - 1U;
    SYST_CVR = 0;

    // ENABLE = 1, CLKSOURCE = CPU clock
    // No SysTick interrupt
    SYST_CSR = (1U << 2) | (1U << 0);
}

// =====================================
// 4. UART Send
// =====================================

void uart2_send_char(char c)
{
    while ((USART2_SR & UART_TXE) == 0U)
    {
    }

    USART2_DR = (uint8_t)c;
}

void uart2_send_string(const char *s)
{
    while (*s)
    {
        uart2_send_char(*s++);
    }
}

void uart2_send_number(uint32_t n)
{
    char buf[11];
    uint32_t i = 0;

    if (n == 0U)
    {
        uart2_send_char('0');
        return;
    }

    while (n)
    {
        buf[i++] = (char)('0' + n % 10U);
        n /= 10U;
    }

    while (i)
    {
        uart2_send_char(buf[--i]);
    }
}

// =====================================
// 5. UART Receive (Non-blocking)
// =====================================

//  1 = valid byte
//  0 = no byte
// -1 = UART error
int uart2_receive_checked(char *out)
{
    uint32_t sr = USART2_SR;

    if ((sr & (UART_ERRORS | UART_RXNE)) == 0U)
        return 0;

    // SR -> DR sequence
    uint32_t dr = USART2_DR;

    if (sr & UART_ORE) ore_count++;
    if (sr & UART_FE)  fe_count++;
    if (sr & UART_NE)  ne_count++;
    if (sr & UART_PE)  pe_count++;

    if (sr & UART_ERRORS)
        return -1;

    if (sr & UART_RXNE)
    {
        *out = (char)(dr & 0xFFU);
        return 1;
    }

    return 0;
}

// =====================================
// 6. LED Control
// =====================================

void led_control(uint8_t led, uint8_t on)
{
    on = on ? 1U : 0U;

    if (led == 1)
    {
        GPIOB_BSRR = on ? LED1 : (LED1 << 16);
        led_state[0] = on;
    }
    else if (led == 2)
    {
        GPIOA_BSRR = on ? LED2 : (LED2 << 16);
        led_state[1] = on;
    }
    else if (led == 3)
    {
        GPIOB_BSRR = on ? LED3 : (LED3 << 16);
        led_state[2] = on;
    }
}

void all_led_control(uint8_t on)
{
    led_control(1, on);
    led_control(2, on);
    led_control(3, on);
}

// =====================================
// 7. Button
// =====================================

uint8_t key_read(void)
{
    return (GPIOC_IDR & (1U << 13)) ? 1U : 0U;
}

void key_init(void)
{
    key_stable = key_read();
    key_candidate = key_stable;
    key_count = 0;
}

// Called approximately every 1ms
void key_scan_1ms(void)
{
    uint8_t current = key_read();

    if (current != key_candidate)
    {
        key_candidate = current;
        key_count = 0;
        return;
    }

    if (current == key_stable)
    {
        key_count = 0;
        return;
    }

    if (key_count < DEBOUNCE_MS)
        key_count++;

    if (key_count >= DEBOUNCE_MS)
    {
        key_stable = current;
        key_count = 0;

        if (key_stable)
        {
            uart2_send_string(
                "\r\nKEY1 = 1 (PRESSED)\r\n> "
            );
        }
        else
        {
            uart2_send_string(
                "\r\nKEY1 = 0 (RELEASED)\r\n> "
            );
        }
    }
}

// =====================================
// 8. String Compare
// =====================================

int str_equal(const char *a, const char *b)
{
    while (*a && *b)
    {
        if (*a != *b)
            return 0;

        a++;
        b++;
    }

    return (*a == '\0' && *b == '\0');
}

// =====================================
// 9. Print Status
// =====================================

void print_status(void)
{
    uart2_send_string("\r\nDEVICE STATUS\r\n");

    uart2_send_string("LED1: ");
    uart2_send_string(led_state[0] ? "ON\r\n" : "OFF\r\n");

    uart2_send_string("LED2: ");
    uart2_send_string(led_state[1] ? "ON\r\n" : "OFF\r\n");

    uart2_send_string("LED3: ");
    uart2_send_string(led_state[2] ? "ON\r\n" : "OFF\r\n");

    uart2_send_string("KEY1: ");
    uart2_send_string(
        key_stable ? "PRESSED (1)\r\n" :
                     "RELEASED (0)\r\n"
    );
}

void print_errors(void)
{
    uart2_send_string("\r\nUART ERRORS\r\n");

    uart2_send_string("ORE: ");
    uart2_send_number(ore_count);
    uart2_send_string("\r\n");

    uart2_send_string("FE : ");
    uart2_send_number(fe_count);
    uart2_send_string("\r\n");

    uart2_send_string("NE : ");
    uart2_send_number(ne_count);
    uart2_send_string("\r\n");

    uart2_send_string("PE : ");
    uart2_send_number(pe_count);
    uart2_send_string("\r\n");

    uart2_send_string("BUFFER: ");
    uart2_send_number(buffer_error_count);
    uart2_send_string("\r\n");
}

// =====================================
// 10. Command Processing
// =====================================

void process_command(const char *cmd)
{
    if (str_equal(cmd, "led1 on"))
        led_control(1, 1);

    else if (str_equal(cmd, "led2 on"))
        led_control(2, 1);

    else if (str_equal(cmd, "led3 on"))
        led_control(3, 1);

    else if (str_equal(cmd, "led1 off"))
        led_control(1, 0);

    else if (str_equal(cmd, "led2 off"))
        led_control(2, 0);

    else if (str_equal(cmd, "led3 off"))
        led_control(3, 0);

    else if (str_equal(cmd, "123 on"))
        all_led_control(1);

    else if (str_equal(cmd, "123 off") ||
             str_equal(cmd, "off"))
        all_led_control(0);

    else if (str_equal(cmd, "key1"))
    {
        uart2_send_string("KEY1 = ");
        uart2_send_string(
            key_stable ? "1 (PRESSED)\r\n" :
                         "0 (RELEASED)\r\n"
        );
        return;
    }
    else if (str_equal(cmd, "errors"))
    {
        print_errors();
        return;
    }
    else if (str_equal(cmd, "clear"))
    {
        ore_count = 0;
        fe_count = 0;
        ne_count = 0;
        pe_count = 0;
        buffer_error_count = 0;

        uart2_send_string("ERROR COUNTERS CLEARED\r\n");
        return;
    }
    else if (str_equal(cmd, "status"))
    {
        print_status();
        return;
    }
    else
    {
        uart2_send_string("UNKNOWN COMMAND: [");
        uart2_send_string(cmd);
        uart2_send_string("]\r\n");
        return;
    }

    uart2_send_string("COMMAND OK\r\n");
    print_status();
}

// =====================================
// 11. Main
// =====================================

int main(void)
{
    gpio_init();
    uart2_init();
    systick_init();
    key_init();

    char buffer[32];
    uint32_t index = 0;
    uint8_t line_error = 0;

    uart2_send_string("\r\nSTM32F446RE READY\r\n");
    uart2_send_string("> ");

    while (1)
    {
        // 1. Button scan on SysTick 1ms flag
        if (SYST_CSR & (1U << 16))
        {
            key_scan_1ms();
        }

        // 2. UART non-blocking receive
        char c = 0;
        int result = uart2_receive_checked(&c);

        if (result == 0)
            continue;

        if (result == -1)
        {
            line_error = 1;
            continue;
        }

        // 3. Enter
        if (c == '\r' || c == '\n')
        {
            if (line_error)
            {
                uart2_send_string(
                    "\r\nRX ERROR: COMMAND DISCARDED\r\n> "
                );

                index = 0;
                line_error = 0;
                continue;
            }

            // Trim trailing spaces
            while (index > 0 &&
                   buffer[index - 1] == ' ')
            {
                index--;
            }

            if (index > 0)
            {
                buffer[index] = '\0';

                uint32_t start = 0;
                while (buffer[start] == ' ')
                    start++;

                uart2_send_string("\r\n");

                if (buffer[start] != '\0')
                    process_command(&buffer[start]);

                index = 0;
                uart2_send_string("> ");
            }
        }

        // 4. Backspace
        else if (c == '\b' || (uint8_t)c == 127U)
        {
            if (index > 0 && !line_error)
            {
                index--;
                uart2_send_string("\b \b");
            }
        }

        // 5. ASCII input
        else if (c >= 32 && c <= 126)
        {
            if (line_error)
                continue;

            if (index < sizeof(buffer) - 1U)
            {
                buffer[index++] = c;
                uart2_send_char(c);
            }
            else
            {
                buffer_error_count++;
                line_error = 1;
            }
        }
    }
}
