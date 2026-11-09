
#include <stdint.h>

/*
 * STM32 NUCLEO-F446RE
 * UART Command LED / Button Control
 *
 * USART2 (ST-LINK VCP)
 *   PA2 = TX (AF7)
 *   PA3 = RX (AF7)
 *
 * LED1 = PB0 (External)
 * LED2 = PA5 (Onboard LD2)
 * LED3 = PB1 (External)
 *
 * KEY1 = PC13 (Onboard B1)
 *
 * Clock: HSI / APB1 = 16MHz
 * Baudrate: 9600, 8N1
 *
 * Bare-metal Register Programming
 */

// =====================================
// 1. Base Addresses
// =====================================

#define RCC_BASE     0x40023800UL
#define GPIOA_BASE   0x40020000UL
#define GPIOB_BASE   0x40020400UL
#define GPIOC_BASE   0x40020800UL
#define USART2_BASE  0x40004400UL

// =====================================
// 2. RCC Registers
// =====================================

#define RCC_AHB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x30UL))
#define RCC_APB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x40UL))

// =====================================
// 3. GPIO Registers
// =====================================

// GPIOA
#define GPIOA_MODER (*(volatile uint32_t *)(GPIOA_BASE + 0x00UL))
#define GPIOA_BSRR  (*(volatile uint32_t *)(GPIOA_BASE + 0x18UL))
#define GPIOA_AFRL  (*(volatile uint32_t *)(GPIOA_BASE + 0x20UL))

// GPIOB
#define GPIOB_MODER (*(volatile uint32_t *)(GPIOB_BASE + 0x00UL))
#define GPIOB_BSRR  (*(volatile uint32_t *)(GPIOB_BASE + 0x18UL))

// GPIOC
#define GPIOC_MODER (*(volatile uint32_t *)(GPIOC_BASE + 0x00UL))
#define GPIOC_IDR   (*(volatile uint32_t *)(GPIOC_BASE + 0x10UL))

// =====================================
// 4. USART2 Registers
// =====================================

#define USART2_SR  (*(volatile uint32_t *)(USART2_BASE + 0x00UL))
#define USART2_DR  (*(volatile uint32_t *)(USART2_BASE + 0x04UL))
#define USART2_BRR (*(volatile uint32_t *)(USART2_BASE + 0x08UL))
#define USART2_CR1 (*(volatile uint32_t *)(USART2_BASE + 0x0CUL))
#define USART2_CR2 (*(volatile uint32_t *)(USART2_BASE + 0x10UL))
#define USART2_CR3 (*(volatile uint32_t *)(USART2_BASE + 0x14UL))

// =====================================
// 5. LED Pin Definitions
// =====================================

#define LED1 (1U << 0)  // PB0
#define LED2 (1U << 5)  // PA5
#define LED3 (1U << 1)  // PB1

// =====================================
// 6. GPIO Initialization
// =====================================

void gpio_init(void)
{
    // GPIOA, GPIOB, GPIOC clock enable
    RCC_AHB1ENR |= (1U << 0) |
                   (1U << 1) |
                   (1U << 2);

    // PA5 = Output (LD2)
    GPIOA_MODER &= ~(3U << 10);
    GPIOA_MODER |=  (1U << 10);

    // PB0, PB1 = Output
    GPIOB_MODER &= ~((3U << 0) | (3U << 2));
    GPIOB_MODER |=  ((1U << 0) | (1U << 2));

    // Initial LED OFF
    GPIOA_BSRR = (LED2 << 16);
    GPIOB_BSRR = ((LED1 | LED3) << 16);

    // PC13 = Input
    GPIOC_MODER &= ~(3U << 26);
}

// =====================================
// 7. UART2 Initialization
// =====================================

void uart2_init(void)
{
    // GPIOA clock enable
    RCC_AHB1ENR |= (1U << 0);

    // USART2 clock enable
    RCC_APB1ENR |= (1U << 17);

    // PA2, PA3 Alternate Function
    GPIOA_MODER &= ~((3U << 4) | (3U << 6));
    GPIOA_MODER |=  ((2U << 4) | (2U << 6));

    // AF7 = USART2
    GPIOA_AFRL &= ~((0xFU << 8) | (0xFU << 12));
    GPIOA_AFRL |=  ((7U << 8) | (7U << 12));

    // USART2 disable and configure
    USART2_CR1 = 0U;
    USART2_CR2 = 0U;
    USART2_CR3 = 0U;

    // PCLK1 = 16MHz, 9600bps
    USART2_BRR = 1667U;

    // USART Enable + TX + RX
    USART2_CR1 = (1U << 13) |
                 (1U << 3)  |
                 (1U << 2);
}

// =====================================
// 8. UART Send Character
// =====================================

void uart2_send_char(char c)
{
    while ((USART2_SR & (1U << 7)) == 0U)
        ;

    USART2_DR = (uint8_t)c;
}

// =====================================
// 9. UART Send String
// =====================================

void uart2_send_string(const char *str)
{
    while (*str)
    {
        uart2_send_char(*str++);
    }
}

// =====================================
// 10. UART Receive Character
// =====================================

char uart2_receive_char(void)
{
    while ((USART2_SR & (1U << 5)) == 0U)
        ;

    return (char)(USART2_DR & 0xFFU);
}

// =====================================
// 11. String Compare
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
// 12. LED Control
// =====================================

void led_control(uint8_t led, uint8_t on)
{
    if (led == 1)
    {
        GPIOB_BSRR = on ? LED1 : (LED1 << 16);
    }
    else if (led == 2)
    {
        GPIOA_BSRR = on ? LED2 : (LED2 << 16);
    }
    else if (led == 3)
    {
        GPIOB_BSRR = on ? LED3 : (LED3 << 16);
    }
}

void all_led_control(uint8_t on)
{
    GPIOA_BSRR = on ? LED2 : (LED2 << 16);

    GPIOB_BSRR = on
               ? (LED1 | LED3)
               : ((LED1 | LED3) << 16);
}

// =====================================
// 13. Command Processing
// =====================================

void process_command(const char *cmd)
{
    if (str_equal(cmd, "led1 on"))
    {
        led_control(1, 1);
        uart2_send_string("LED1 ON\r\n");
    }
    else if (str_equal(cmd, "led2 on"))
    {
        led_control(2, 1);
        uart2_send_string("LD2 ON\r\n");
    }
    else if (str_equal(cmd, "led3 on"))
    {
        led_control(3, 1);
        uart2_send_string("LED3 ON\r\n");
    }
    else if (str_equal(cmd, "led1 off"))
    {
        led_control(1, 0);
        uart2_send_string("LED1 OFF\r\n");
    }
    else if (str_equal(cmd, "led2 off"))
    {
        led_control(2, 0);
        uart2_send_string("LD2 OFF\r\n");
    }
    else if (str_equal(cmd, "led3 off"))
    {
        led_control(3, 0);
        uart2_send_string("LED3 OFF\r\n");
    }
    else if (str_equal(cmd, "123 on"))
    {
        all_led_control(1);
        uart2_send_string("ALL LED ON\r\n");
    }
    else if (str_equal(cmd, "123 off") ||
             str_equal(cmd, "off"))
    {
        all_led_control(0);
        uart2_send_string("ALL LED OFF\r\n");
    }
    else if (str_equal(cmd, "key1"))
    {
        // B1 PC13 : Pressed = HIGH
        uint32_t pressed =
            (GPIOC_IDR & (1U << 13)) != 0U;

        if (pressed)
        {
            uart2_send_string("KEY1 = 1 (PRESSED)\r\n");
        }
        else
        {
            uart2_send_string("KEY1 = 0 (RELEASED)\r\n");
        }
    }
    else
    {
        uart2_send_string("UNKNOWN COMMAND\r\n");
    }
}

// =====================================
// 14. Main
// =====================================

int main(void)
{
    gpio_init();
    uart2_init();

    char buffer[32];
    uint32_t index = 0;

    uart2_send_string("\r\nSTM32F446RE READY\r\n");
    uart2_send_string("> ");

    while (1)
    {
        char c = uart2_receive_char();

        // Enter: CR or LF
        if (c == '\r' || c == '\n')
        {
            if (index > 0)
            {
                buffer[index] = '\0';

                uart2_send_string("\r\n");
                process_command(buffer);

                index = 0;
                uart2_send_string("> ");
            }
        }
        // Backspace
        else if (c == '\b' || (uint8_t)c == 127U)
        {
            if (index > 0)
            {
                index--;
                uart2_send_string("\b \b");
            }
        }
        // Printable ASCII
        else if (c >= 32 && c <= 126)
        {
            if (index < sizeof(buffer) - 1U)
            {
                buffer[index++] = c;

                // Echo character to CoolTerm
                uart2_send_char(c);
            }
        }
    }
}
