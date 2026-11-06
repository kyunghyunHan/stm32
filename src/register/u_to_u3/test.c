
#include <stdint.h>

/*
 * STM32F446RE - 3 MCU UART HUB
 *
 * F429ZI <-> F446RE <-> ESP32
 *
 * Clock: HSI / HCLK / PCLK1 / PCLK2 = 16MHz
 * UART : 9600bps, 8N1
 *
 * F446RE:
 *   PC13 : B1 USER Button (Active LOW)
 *   PA5  : LD2 Green LED (Active HIGH)
 *
 * USART1 <-> STM32F429ZI
 *   PA9  / D8 : TX -> F429ZI D0 (PG9 RX)
 *   PA10 / D2 : RX <- F429ZI D1 (PG14 TX)
 *
 * UART4 <-> ESP32
 *   PA0 / A0 : TX -> ESP32 GPIO25 RX
 *   PA1 / A1 : RX <- ESP32 GPIO26 TX
 *
 * Protocol:
 *   TX '1' = LED ON / Button Pressed
 *   TX '0' = LED OFF / Button Released
 *
 * No Interrupt / No DMA
 */

/* =========================================
 * 1. BASE ADDRESS
 * ========================================= */

#define RCC_BASE       0x40023800UL
#define GPIOA_BASE     0x40020000UL
#define GPIOC_BASE     0x40020800UL
#define USART1_BASE    0x40011000UL
#define UART4_BASE     0x40004C00UL
#define SYSTICK_BASE   0xE000E010UL

#define REG32(addr) (*(volatile uint32_t *)(addr))

/* RCC */
#define RCC_AHB1ENR REG32(RCC_BASE + 0x30UL)
#define RCC_APB1ENR REG32(RCC_BASE + 0x40UL)
#define RCC_APB2ENR REG32(RCC_BASE + 0x44UL)

/* GPIOA */
#define GPIOA_MODER REG32(GPIOA_BASE + 0x00UL)
#define GPIOA_AFRL  REG32(GPIOA_BASE + 0x20UL)
#define GPIOA_AFRH  REG32(GPIOA_BASE + 0x24UL)
#define GPIOA_BSRR  REG32(GPIOA_BASE + 0x18UL)

/* GPIOC */
#define GPIOC_MODER REG32(GPIOC_BASE + 0x00UL)
#define GPIOC_PUPDR REG32(GPIOC_BASE + 0x0CUL)
#define GPIOC_IDR   REG32(GPIOC_BASE + 0x10UL)

/* USART1 */
#define USART1_SR  REG32(USART1_BASE + 0x00UL)
#define USART1_DR  REG32(USART1_BASE + 0x04UL)
#define USART1_BRR REG32(USART1_BASE + 0x08UL)
#define USART1_CR1 REG32(USART1_BASE + 0x0CUL)
#define USART1_CR2 REG32(USART1_BASE + 0x10UL)
#define USART1_CR3 REG32(USART1_BASE + 0x14UL)

/* UART4 */
#define UART4_SR   REG32(UART4_BASE + 0x00UL)
#define UART4_DR   REG32(UART4_BASE + 0x04UL)
#define UART4_BRR  REG32(UART4_BASE + 0x08UL)
#define UART4_CR1  REG32(UART4_BASE + 0x0CUL)
#define UART4_CR2  REG32(UART4_BASE + 0x10UL)
#define UART4_CR3  REG32(UART4_BASE + 0x14UL)

/* SysTick */
#define STK_CTRL REG32(SYSTICK_BASE + 0x00UL)
#define STK_LOAD REG32(SYSTICK_BASE + 0x04UL)
#define STK_VAL  REG32(SYSTICK_BASE + 0x08UL)

/* Settings */
#define DEBOUNCE_MS 20U
#define RESEND_MS   200U

/* =========================================
 * 2. GPIO INIT
 * ========================================= */

void gpio_init(void)
{
    /* GPIOA + GPIOC Clock Enable */
    RCC_AHB1ENR |= (1U << 0) | (1U << 2);

    /* PA5 Output */
    GPIOA_MODER &= ~(3U << 10);
    GPIOA_MODER |=  (1U << 10);

    /* LED OFF */
    GPIOA_BSRR = (1U << 21);

    /* PC13 Input */
    GPIOC_MODER &= ~(3U << 26);

    /* PC13 Pull-up */
    GPIOC_PUPDR &= ~(3U << 26);
    GPIOC_PUPDR |=  (1U << 26);
}

void led_on(void)
{
    GPIOA_BSRR = (1U << 5);
}

void led_off(void)
{
    GPIOA_BSRR = (1U << 21);
}

uint32_t button_pressed(void)
{
    return ((GPIOC_IDR & (1U << 13)) == 0U);
}

/* =========================================
 * 3. USART1 - STM32F429ZI
 * ========================================= */

void uart1_init(void)
{
    /* GPIOA Clock */
    RCC_AHB1ENR |= (1U << 0);

    /* USART1 Clock: APB2 Bit 4 */
    RCC_APB2ENR |= (1U << 4);

    /* PA9, PA10 Alternate Function */
    GPIOA_MODER &= ~((3U << 18) | (3U << 20));
    GPIOA_MODER |=  ((2U << 18) | (2U << 20));

    /* PA9, PA10 = AF7 */
    GPIOA_AFRH &= ~((0xFU << 4) | (0xFU << 8));
    GPIOA_AFRH |=  ((7U << 4) | (7U << 8));

    /* Disable USART */
    USART1_CR1 = 0U;
    USART1_CR2 = 0U;
    USART1_CR3 = 0U;

    /* PCLK2 = 16MHz, 9600bps */
    USART1_BRR = 0x0683U;

    /* UE + TE + RE */
    USART1_CR1 =
        (1U << 13) |
        (1U << 3)  |
        (1U << 2);
}

void uart1_send(char data)
{
    while ((USART1_SR & (1U << 7)) == 0U)
    {
    }

    USART1_DR = (uint8_t)data;
}

int uart1_receive(void)
{
    uint32_t status = USART1_SR;

    /* ORE / NE / FE / PE */
    if (status & 0x0FU)
    {
        (void)USART1_DR;
        return -1;
    }

    /* RXNE */
    if (status & (1U << 5))
    {
        return (int)(USART1_DR & 0xFFU);
    }

    return -1;
}

/* =========================================
 * 4. UART4 - ESP32
 * ========================================= */

void uart4_init(void)
{
    /* GPIOA Clock */
    RCC_AHB1ENR |= (1U << 0);

    /* UART4 Clock: APB1 Bit 19 */
    RCC_APB1ENR |= (1U << 19);

    /*
     * PA0 = A0 = UART4 TX
     * PA1 = A1 = UART4 RX
     *
     * MODER = 10
     */

    GPIOA_MODER &= ~((3U << 0) | (3U << 2));
    GPIOA_MODER |=  ((2U << 0) | (2U << 2));

    /* PA0, PA1 = AF8 */
    GPIOA_AFRL &= ~((0xFU << 0) | (0xFU << 4));
    GPIOA_AFRL |=  ((8U << 0) | (8U << 4));

    /* Disable UART */
    UART4_CR1 = 0U;
    UART4_CR2 = 0U;
    UART4_CR3 = 0U;

    /* PCLK1 = 16MHz, 9600bps */
    UART4_BRR = 0x0683U;

    /* UE + TE + RE */
    UART4_CR1 =
        (1U << 13) |
        (1U << 3)  |
        (1U << 2);
}

void uart4_send(char data)
{
    while ((UART4_SR & (1U << 7)) == 0U)
    {
    }

    UART4_DR = (uint8_t)data;
}

int uart4_receive(void)
{
    uint32_t status = UART4_SR;

    /* ORE / NE / FE / PE */
    if (status & 0x0FU)
    {
        (void)UART4_DR;
        return -1;
    }

    /* RXNE */
    if (status & (1U << 5))
    {
        return (int)(UART4_DR & 0xFFU);
    }

    return -1;
}

/* =========================================
 * 5. SYSTICK - 1ms
 * ========================================= */

void systick_init(void)
{
    STK_CTRL = 0U;
    STK_LOAD = 16000U - 1U;
    STK_VAL = 0U;

    /* Core clock, no interrupt */
    STK_CTRL = (1U << 2) | (1U << 0);
}

/* =========================================
 * 6. MAIN
 * ========================================= */

int main(void)
{
    gpio_init();
    uart1_init();
    uart4_init();
    systick_init();

    /*
     * Individual button states
     *
     * re_state  : F446RE B1
     * zi_state  : F429ZI B1
     * esp_state : ESP32 BOOT
     */

    uint32_t re_state = button_pressed();
    uint32_t zi_state = 0U;
    uint32_t esp_state = 0U;

    uint32_t raw_button = re_state;
    uint32_t debounce_time = 0U;

    uint32_t now = 0U;
    uint32_t last_send = 0U;

    /* 2 = invalid initial value */
    uint32_t last_zi_output = 2U;
    uint32_t last_esp_output = 2U;

    while (1)
    {
        /* -----------------------------
         * 1ms Time Update
         * ----------------------------- */

        if (STK_CTRL & (1U << 16))
        {
            now++;
        }

        /* -----------------------------
         * F446RE Button Debounce
         * ----------------------------- */

        uint32_t current = button_pressed();

        if (current != raw_button)
        {
            raw_button = current;
            debounce_time = now;
        }

        if ((now - debounce_time) >= DEBOUNCE_MS)
        {
            re_state = raw_button;
        }

        /* -----------------------------
         * F429ZI Receive
         * ----------------------------- */

        int zi_rx = uart1_receive();

        if (zi_rx == '1')
        {
            zi_state = 1U;
        }
        else if (zi_rx == '0')
        {
            zi_state = 0U;
        }

        /* -----------------------------
         * ESP32 Receive
         * ----------------------------- */

        int esp_rx = uart4_receive();

        if (esp_rx == '1')
        {
            esp_state = 1U;
        }
        else if (esp_rx == '0')
        {
            esp_state = 0U;
        }

        /* -----------------------------
         * F446RE LED:
         * F429ZI OR ESP32
         * ----------------------------- */

        if (zi_state || esp_state)
        {
            led_on();
        }
        else
        {
            led_off();
        }

        /* -----------------------------
         * F429ZI LED:
         * F446RE OR ESP32
         * ----------------------------- */

        uint32_t zi_output =
            re_state || esp_state;

        /* -----------------------------
         * ESP32 LED:
         * F446RE OR F429ZI
         * ----------------------------- */

        uint32_t esp_output =
            re_state || zi_state;

        /* -----------------------------
         * Send on change or every 200ms
         * ----------------------------- */

        uint32_t refresh =
            ((now - last_send) >= RESEND_MS);

        if ((zi_output != last_zi_output) || refresh)
        {
            uart1_send(zi_output ? '1' : '0');
            last_zi_output = zi_output;
        }

        if ((esp_output != last_esp_output) || refresh)
        {
            uart4_send(esp_output ? '1' : '0');
            last_esp_output = esp_output;
        }

        if (refresh)
        {
            last_send = now;
        }
    }
}
