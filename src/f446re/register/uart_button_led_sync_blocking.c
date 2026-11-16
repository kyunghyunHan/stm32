// 지연 함수로 버튼 디바운싱을 처리하고 UART로 LED를 연동하는 실습

#include <stdint.h>

#define RCC_BASE      0x40023800UL
#define GPIOA_BASE    0x40020000UL
#define GPIOC_BASE    0x40020800UL
#define USART1_BASE   0x40011000UL
#define SYSTICK_BASE  0xE000E010UL

#define REG32(addr) (*(volatile uint32_t *)(addr))

/* RCC */
#define RCC_AHB1ENR REG32(RCC_BASE + 0x30UL)
#define RCC_APB2ENR REG32(RCC_BASE + 0x44UL)

/* GPIOA */
#define GPIOA_MODER REG32(GPIOA_BASE + 0x00UL)
#define GPIOA_AFRH  REG32(GPIOA_BASE + 0x24UL)
#define GPIOA_BSRR  REG32(GPIOA_BASE + 0x18UL)

/* GPIOC */
#define GPIOC_MODER REG32(GPIOC_BASE + 0x00UL)
#define GPIOC_PUPDR REG32(GPIOC_BASE + 0x0CUL)
#define GPIOC_IDR   REG32(GPIOC_BASE + 0x10UL)

/* USART1 */
#define USART1_SR   REG32(USART1_BASE + 0x00UL)
#define USART1_DR   REG32(USART1_BASE + 0x04UL)
#define USART1_BRR  REG32(USART1_BASE + 0x08UL)
#define USART1_CR1  REG32(USART1_BASE + 0x0CUL)

/* SysTick */
#define STK_CTRL REG32(SYSTICK_BASE + 0x00UL)
#define STK_LOAD REG32(SYSTICK_BASE + 0x04UL)
#define STK_VAL  REG32(SYSTICK_BASE + 0x08UL)

/* B1: pressed = LOW */
#define BUTTON_PRESSED_LEVEL 0U

void gpio_init(void)
{
    /* GPIOA, GPIOC clock enable */
    RCC_AHB1ENR |= (1U << 0);
    RCC_AHB1ENR |= (1U << 2);

    /* PA5: Output (01) */
    GPIOA_MODER &= ~(3U << 10);
    GPIOA_MODER |=  (1U << 10);

    /* LED OFF */
    GPIOA_BSRR = (1U << 21);

    /* PC13: Input (00) */
    GPIOC_MODER &= ~(3U << 26);

    /* PC13: Pull-up (01) */
    GPIOC_PUPDR &= ~(3U << 26);
    GPIOC_PUPDR |=  (1U << 26);
}

void uart1_init(void)
{
    /* GPIOA clock */
    RCC_AHB1ENR |= (1U << 0);

    /* USART1 clock (APB2 bit 4) */
    RCC_APB2ENR |= (1U << 4);

    /* PA9, PA10 Alternate Function (10) */
    GPIOA_MODER &= ~((3U << 18) | (3U << 20));
    GPIOA_MODER |=  ((2U << 18) | (2U << 20));

    /* AF7: USART1 */
    GPIOA_AFRH &= ~((0xFU << 4) | (0xFU << 8));
    GPIOA_AFRH |=  ((7U << 4) | (7U << 8));

    /* USART disabled during configuration */
    USART1_CR1 = 0U;

    /* 16MHz / 9600bps, OVER8=0 */
    USART1_BRR = 0x0683U;

    /* UE + TE + RE */
    USART1_CR1 = (1U << 13) |
                 (1U << 3)  |
                 (1U << 2);
}

void uart1_send(char data)
{
    /* Wait until TX data register empty */
    while ((USART1_SR & (1U << 7)) == 0U)
    {
    }

    USART1_DR = (uint8_t)data;
}

int uart1_receive(void)
{
    uint32_t status = USART1_SR;

    /* Check overrun, framing, noise, parity errors */
    if (status & ((1U << 3) | (1U << 2) |
                  (1U << 1) | (1U << 0)))
    {
        /* SR -> DR read clears error flags */
        (void)USART1_DR;
        return -1;
    }

    /* RXNE: received data exists */
    if (status & (1U << 5))
    {
        return (int)(USART1_DR & 0xFFU);
    }

    return -1;
}

void led_on(void)
{
    /* PA5 SET */
    GPIOA_BSRR = (1U << 5);
}

void led_off(void)
{
    /* PA5 RESET */
    GPIOA_BSRR = (1U << 21);
}

uint32_t button_pressed(void)
{
    uint32_t level = (GPIOC_IDR >> 13) & 1U;

    return level == BUTTON_PRESSED_LEVEL;
}

void delay_ms(uint32_t ms)
{
    STK_CTRL = 0U;
    STK_LOAD = 16000U - 1U;
    STK_VAL = 0U;

    /* Core clock, enable, no interrupt */
    STK_CTRL = (1U << 2) | (1U << 0);

    while (ms--)
    {
        while ((STK_CTRL & (1U << 16)) == 0U)
        {
        }
    }

    STK_CTRL = 0U;
}

int main(void)
{
    gpio_init();
    uart1_init();

    uint32_t previous = button_pressed();

    /* Send initial button state */
    uart1_send(previous ? '1' : '0');

    while (1)
    {
        /* STM32 button -> ESP32 LED */
        uint32_t current = button_pressed();

        if (current != previous)
        {
            /* Debounce */
            delay_ms(20);

            current = button_pressed();

            if (current != previous)
            {
                uart1_send(current ? '1' : '0');
                previous = current;
            }
        }

        /* ESP32 button -> STM32 LED */
        int received = uart1_receive();

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
