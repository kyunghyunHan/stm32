
#include <stdint.h>

/*
 * STM32F429ZI <-> STM32F446RE UART
 *
 * MCU      : STM32F429ZI
 * Clock    : HSI 16MHz
 * HCLK     : 16MHz
 * PCLK2    : 16MHz
 * USART6   : 9600bps, 8N1
 *
 * PC13     : USER B1 Button
 * PB7      : LD2 Blue LED
 *
 * PG14     : USART6 TX (D1) -> F446RE D2 RX
 * PG9      : USART6 RX (D0) <- F446RE D8 TX
 *
 * Protocol:
 * TX '1' : Button pressed
 * TX '0' : Button released
 *
 * RX '1' : LED ON
 * RX '0' : LED OFF
 */

#define RCC_BASE       0x40023800UL
#define GPIOB_BASE     0x40020400UL
#define GPIOC_BASE     0x40020800UL
#define GPIOG_BASE     0x40021800UL
#define USART6_BASE    0x40011400UL
#define SYSTICK_BASE   0xE000E010UL

#define REG32(addr) (*(volatile uint32_t *)(addr))

/* RCC */
#define RCC_AHB1ENR REG32(RCC_BASE + 0x30UL)
#define RCC_APB2ENR REG32(RCC_BASE + 0x44UL)

/* GPIOB - LD2 Blue LED */
#define GPIOB_MODER REG32(GPIOB_BASE + 0x00UL)
#define GPIOB_BSRR  REG32(GPIOB_BASE + 0x18UL)

/* GPIOC - B1 Button */
#define GPIOC_MODER REG32(GPIOC_BASE + 0x00UL)
#define GPIOC_PUPDR REG32(GPIOC_BASE + 0x0CUL)
#define GPIOC_IDR   REG32(GPIOC_BASE + 0x10UL)

/* GPIOG - USART6 */
#define GPIOG_MODER REG32(GPIOG_BASE + 0x00UL)
#define GPIOG_AFRH  REG32(GPIOG_BASE + 0x24UL)

/* USART6 */
#define USART6_SR   REG32(USART6_BASE + 0x00UL)
#define USART6_DR   REG32(USART6_BASE + 0x04UL)
#define USART6_BRR  REG32(USART6_BASE + 0x08UL)
#define USART6_CR1  REG32(USART6_BASE + 0x0CUL)
#define USART6_CR2  REG32(USART6_BASE + 0x10UL)
#define USART6_CR3  REG32(USART6_BASE + 0x14UL)

/* SysTick */
#define STK_CTRL REG32(SYSTICK_BASE + 0x00UL)
#define STK_LOAD REG32(SYSTICK_BASE + 0x04UL)
#define STK_VAL  REG32(SYSTICK_BASE + 0x08UL)

/* F429ZI B1: pressed = HIGH */
#define BUTTON_PRESSED_LEVEL 1U


/* ---------------------------------
 * 1. GPIO Init
 * --------------------------------- */
void gpio_init(void)
{
    /* GPIOB, GPIOC clocks */
    RCC_AHB1ENR |= (1U << 1);
    RCC_AHB1ENR |= (1U << 2);

    /* PB7: Output (01) */
    GPIOB_MODER &= ~(3U << 14);
    GPIOB_MODER |=  (1U << 14);

    /* LED OFF */
    GPIOB_BSRR = (1U << 23);

    /* PC13: Input (00) */
    GPIOC_MODER &= ~(3U << 26);

    /* PC13: Pull-down (10) */
    GPIOC_PUPDR &= ~(3U << 26);
    GPIOC_PUPDR |=  (2U << 26);
}


/* ---------------------------------
 * 2. USART6 Init
 * --------------------------------- */
void uart6_init(void)
{
    /* GPIOG clock: AHB1 bit 6 */
    RCC_AHB1ENR |= (1U << 6);

    /* USART6 clock: APB2 bit 5 */
    RCC_APB2ENR |= (1U << 5);

    /*
     * PG9  = D0 = RX
     * PG14 = D1 = TX
     *
     * MODER = 10 (Alternate Function)
     */

    /* PG9: MODER[19:18] */
    /* PG14: MODER[29:28] */
    GPIOG_MODER &= ~((3U << 18) | (3U << 28));
    GPIOG_MODER |=  ((2U << 18) | (2U << 28));

    /*
     * USART6 = AF8
     *
     * PG9  -> AFRH[7:4]
     * PG14 -> AFRH[27:24]
     */

    GPIOG_AFRH &= ~((0xFU << 4) | (0xFU << 24));
    GPIOG_AFRH |=  ((8U << 4) | (8U << 24));

    /* Disable during configuration */
    USART6_CR1 = 0U;
    USART6_CR2 = 0U;
    USART6_CR3 = 0U;

    /* PCLK2 = 16MHz, Baud = 9600 */
    USART6_BRR = 0x0683U;

    /*
     * UE = bit 13
     * TE = bit 3
     * RE = bit 2
     */
    USART6_CR1 = (1U << 13) |
                 (1U << 3)  |
                 (1U << 2);
}


/* ---------------------------------
 * 3. UART TX
 * --------------------------------- */
void uart6_send(char data)
{
    /* Wait for TXE = 1 */
    while ((USART6_SR & (1U << 7)) == 0U)
    {
    }

    USART6_DR = (uint8_t)data;
}


/* ---------------------------------
 * 4. UART RX (Non-blocking)
 * --------------------------------- */
int uart6_receive(void)
{
    uint32_t status = USART6_SR;

    /* ORE, NE, FE, PE */
    if (status & ((1U << 3) | (1U << 2) |
                  (1U << 1) | (1U << 0)))
    {
        /* Clear error by SR -> DR read */
        (void)USART6_DR;
        return -1;
    }

    /* RXNE */
    if (status & (1U << 5))
    {
        return (int)(USART6_DR & 0xFFU);
    }

    return -1;
}


/* ---------------------------------
 * 5. LED Control
 * --------------------------------- */
void led_on(void)
{
    /* PB7 SET */
    GPIOB_BSRR = (1U << 7);
}

void led_off(void)
{
    /* PB7 RESET */
    GPIOB_BSRR = (1U << 23);
}


/* ---------------------------------
 * 6. Button Read
 * --------------------------------- */
uint32_t button_pressed(void)
{
    uint32_t level = (GPIOC_IDR >> 13) & 1U;

    return (level == BUTTON_PRESSED_LEVEL);
}


/* ---------------------------------
 * 7. SysTick Delay
 * --------------------------------- */
void delay_ms(uint32_t ms)
{
    STK_CTRL = 0U;
    STK_LOAD = 16000U - 1U;
    STK_VAL = 0U;

    /* Core clock, no interrupt */
    STK_CTRL = (1U << 2) | (1U << 0);

    while (ms--)
    {
        while ((STK_CTRL & (1U << 16)) == 0U)
        {
        }
    }

    STK_CTRL = 0U;
}


/* ---------------------------------
 * 8. Main
 * --------------------------------- */
int main(void)
{
    gpio_init();
    uart6_init();

    uint32_t previous = button_pressed();

    /* Initial button state */
    uart6_send(previous ? '1' : '0');

    while (1)
    {
        /*
         * F429ZI Button
         *       ↓
         * F446RE Green LED
         */

        uint32_t current = button_pressed();

        if (current != previous)
        {
            delay_ms(20);

            current = button_pressed();

            if (current != previous)
            {
                uart6_send(current ? '1' : '0');
                previous = current;
            }
        }

        /*
         * F446RE Button
         *       ↓
         * F429ZI Blue LED
         */

        int received = uart6_receive();

        if (received == '1')
        {
            led_on();
        }
        else if (received == '0')
        {
            led_off();
        }
    }
}
