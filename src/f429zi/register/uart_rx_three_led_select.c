#include <stdint.h>

#define RCC_BASE    0x40023800UL
#define GPIOA_BASE  0x40020000UL
#define GPIOC_BASE  0x40020800UL
#define USART2_BASE 0x40004400UL

// RCC
#define RCC_AHB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x30UL))
#define RCC_APB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x40UL))

// GPIOA
#define GPIOA_MODER (*(volatile uint32_t *)(GPIOA_BASE + 0x00UL))
#define GPIOA_ODR   (*(volatile uint32_t *)(GPIOA_BASE + 0x14UL))
#define GPIOA_AFRL  (*(volatile uint32_t *)(GPIOA_BASE + 0x20UL))

// GPIOC
#define GPIOC_MODER (*(volatile uint32_t *)(GPIOC_BASE + 0x00UL))
#define GPIOC_ODR   (*(volatile uint32_t *)(GPIOC_BASE + 0x14UL))

// USART2
#define USART2_SR  (*(volatile uint32_t *)(USART2_BASE + 0x00UL))
#define USART2_DR  (*(volatile uint32_t *)(USART2_BASE + 0x04UL))
#define USART2_BRR (*(volatile uint32_t *)(USART2_BASE + 0x08UL))
#define USART2_CR1 (*(volatile uint32_t *)(USART2_BASE + 0x0CUL))
#define USART2_CR2 (*(volatile uint32_t *)(USART2_BASE + 0x10UL))
#define USART2_CR3 (*(volatile uint32_t *)(USART2_BASE + 0x14UL))

void UART2_Init(void)
{
    // 1. GPIOA, GPIOC 클럭 활성화
    RCC_AHB1ENR |= (1U << 0);
    RCC_AHB1ENR |= (1U << 2);

    // 2. USART2 클럭 활성화
    RCC_APB1ENR |= (1U << 17);

    // 3. LED1 PA8 Output (01)
    GPIOA_MODER &= ~(3U << 16);
    GPIOA_MODER |=  (1U << 16);

    // 4. LED2 PA9 Output (01)
    GPIOA_MODER &= ~(3U << 18);
    GPIOA_MODER |=  (1U << 18);

    // 5. LED3 PC7 Output (01)
    GPIOC_MODER &= ~(3U << 14);
    GPIOC_MODER |=  (1U << 14);

    // 모든 LED 초기 OFF
    GPIOA_ODR &= ~((1U << 8) | (1U << 9));
    GPIOC_ODR &= ~(1U << 7);

    // 6. USART2 TX PA2, RX PA3 : AF 모드 (10)
    GPIOA_MODER &= ~((3U << 4) | (3U << 6));
    GPIOA_MODER |=  ((2U << 4) | (2U << 6));

    // 7. PA2, PA3 : AF7
    GPIOA_AFRL &= ~((0xFU << 8) | (0xFU << 12));
    GPIOA_AFRL |=  ((7U << 8) | (7U << 12));

    // 8. USART2 초기화
    USART2_CR1 = 0U;
    USART2_CR2 = 0U;
    USART2_CR3 = 0U;

    // APB1 = 16MHz, 9600bps
    USART2_BRR = 1667U;

    // RX 활성화
    USART2_CR1 |= (1U << 2);

    // USART2 활성화
    USART2_CR1 |= (1U << 13);
}

// UART 문자 수신
char UART2_ReceiveChar(void)
{
    // RXNE(bit5) = 1이 될 때까지 대기
    while ((USART2_SR & (1U << 5)) == 0U)
    {
    }

    return (char)(USART2_DR & 0xFFU);
}

// 모든 LED OFF
void LED_AllOff(void)
{
    GPIOA_ODR &= ~((1U << 8) | (1U << 9));
    GPIOC_ODR &= ~(1U << 7);
}

int main(void)
{
    UART2_Init();

    while (1)
    {
        char data = UART2_ReceiveChar();

        if (data == '0')
        {
            LED_AllOff();
            GPIOA_ODR |= (1U << 8);  // LED 1 ON (PA8)
        }
        else if (data == '1')
        {
            LED_AllOff();
            GPIOA_ODR |= (1U << 9);  // LED 2 ON (PA9)
        }
        else if (data == '2')
        {
            LED_AllOff();
            GPIOC_ODR |= (1U << 7);  // LED 3 ON (PC7)
        }
        else if (data == '3')
        {
            LED_AllOff();            // 모든 LED OFF
        }
    }
}