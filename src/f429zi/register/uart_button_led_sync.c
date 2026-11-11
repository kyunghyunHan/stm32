
#include <stdint.h>

// STM32F429ZI
// USART1: PA9(TX), PA10(RX)
// Button: PC13 (Active High)
// LED: PB7 (LD2 Blue)
// HSI = 16MHz, USART1 = 9600bps

#define RCC_BASE      0x40023800UL
#define GPIOA_BASE    0x40020000UL
#define GPIOB_BASE    0x40020400UL
#define GPIOC_BASE    0x40020800UL
#define USART1_BASE   0x40011000UL
#define SYSTICK_BASE  0xE000E010UL

#define REG32(addr) (*(volatile uint32_t *)(addr))

// RCC
#define RCC_AHB1ENR REG32(RCC_BASE + 0x30UL)
#define RCC_APB2ENR REG32(RCC_BASE + 0x44UL)

// GPIOA - UART
#define GPIOA_MODER REG32(GPIOA_BASE + 0x00UL)
#define GPIOA_AFRH  REG32(GPIOA_BASE + 0x24UL)

// GPIOB - LED
#define GPIOB_MODER REG32(GPIOB_BASE + 0x00UL)
#define GPIOB_ODR   REG32(GPIOB_BASE + 0x14UL)

// GPIOC - Button
#define GPIOC_MODER REG32(GPIOC_BASE + 0x00UL)
#define GPIOC_PUPDR REG32(GPIOC_BASE + 0x0CUL)
#define GPIOC_IDR   REG32(GPIOC_BASE + 0x10UL)

// USART1
#define USART1_SR  REG32(USART1_BASE + 0x00UL)
#define USART1_DR  REG32(USART1_BASE + 0x04UL)
#define USART1_BRR REG32(USART1_BASE + 0x08UL)
#define USART1_CR1 REG32(USART1_BASE + 0x0CUL)
#define USART1_CR2 REG32(USART1_BASE + 0x10UL)
#define USART1_CR3 REG32(USART1_BASE + 0x14UL)

// SysTick
#define SYSTICK_CTRL REG32(SYSTICK_BASE + 0x00UL)
#define SYSTICK_LOAD REG32(SYSTICK_BASE + 0x04UL)
#define SYSTICK_VAL  REG32(SYSTICK_BASE + 0x08UL)

#define BUTTON_PIN 13U
#define LED_PIN     7U

void uart_send(char data)
{
    while (!(USART1_SR & (1U << 7)))
    {
    }

    USART1_DR = (uint32_t)(uint8_t)data;
}

void uart_init(void)
{
    // USART1 Clock Enable
    RCC_APB2ENR |= (1U << 4);

    // PA9, PA10 Alternate Function (10)
    GPIOA_MODER &= ~((3U << 18) | (3U << 20));
    GPIOA_MODER |=  ((2U << 18) | (2U << 20));

    // PA9, PA10 = AF7
    GPIOA_AFRH &= ~((0xFU << 4) | (0xFU << 8));
    GPIOA_AFRH |=  ((7U << 4) | (7U << 8));

    // USART Disable
    USART1_CR1 = 0;
    USART1_CR2 = 0;
    USART1_CR3 = 0;

    // 16MHz / 9600bps
    USART1_BRR = 0x0683U;

    // TE, RE, UE Enable
    USART1_CR1 =
        (1U << 3) |
        (1U << 2) |
        (1U << 13);
}

void gpio_init(void)
{
    // GPIOA, GPIOB, GPIOC Clock Enable
    RCC_AHB1ENR |= (1U << 0);
    RCC_AHB1ENR |= (1U << 1);
    RCC_AHB1ENR |= (1U << 2);

    // PB7 Output (01)
    GPIOB_MODER &= ~(3U << 14);
    GPIOB_MODER |=  (1U << 14);

    // LED OFF
    GPIOB_ODR &= ~(1U << LED_PIN);

    // PC13 Input (00)
    GPIOC_MODER &= ~(3U << 26);

    // PC13 Pull-down
    GPIOC_PUPDR &= ~(3U << 26);
    GPIOC_PUPDR |=  (2U << 26);
}

void systick_init(void)
{
    SYSTICK_LOAD = 16000U - 1U;
    SYSTICK_VAL = 0;
    SYSTICK_CTRL = (1U << 2) | (1U << 0);
}

uint32_t button_pressed(void)
{
    // Active High
    return (GPIOC_IDR & (1U << BUTTON_PIN)) != 0U;
}

void led_control(char data)
{
    if (data == '1')
    {
        GPIOB_ODR |= (1U << LED_PIN);
    }
    else if (data == '0')
    {
        GPIOB_ODR &= ~(1U << LED_PIN);
    }
}

int main(void)
{
    gpio_init();
    uart_init();
    systick_init();

    uint32_t stable = button_pressed();
    uint32_t last_sample = stable;
    uint32_t count = 0;

    // 초기 버튼 상태 전송
    uart_send(stable ? '1' : '0');

    while (1)
    {
        // UART 수신 확인
        if (USART1_SR & (1U << 5))
        {
            char data = (char)(USART1_DR & 0xFFU);
            led_control(data);
        }

        // SysTick 1ms 경과 확인
        if (SYSTICK_CTRL & (1U << 16))
        {
            uint32_t current = button_pressed();

            if (current == last_sample)
            {
                if (count < 20U)
                    count++;
            }
            else
            {
                count = 0;
                last_sample = current;
            }

            // 20ms Debounce
            if ((count >= 20U) && (stable != current))
            {
                stable = current;

                // 버튼 상태 전송
                uart_send(stable ? '1' : '0');
            }
        }
    }
}
