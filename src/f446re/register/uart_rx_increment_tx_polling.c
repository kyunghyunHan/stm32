#include <stdint.h>

/*
 * STM32F446RE USART2
 * HSI / APB1 = 16MHz
 * Baudrate = 9600, 8N1
 * PA2 = TX, PA3 = RX (AF7)
 */

#define RCC_BASE     0x40023800UL
#define GPIOA_BASE   0x40020000UL
#define USART2_BASE  0x40004400UL

#define RCC_AHB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x30UL))
#define RCC_APB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x40UL))

#define GPIOA_MODER (*(volatile uint32_t *)(GPIOA_BASE + 0x00UL))
#define GPIOA_AFRL  (*(volatile uint32_t *)(GPIOA_BASE + 0x20UL))

#define USART2_SR  (*(volatile uint32_t *)(USART2_BASE + 0x00UL))
#define USART2_DR  (*(volatile uint32_t *)(USART2_BASE + 0x04UL))
#define USART2_BRR (*(volatile uint32_t *)(USART2_BASE + 0x08UL))
#define USART2_CR1 (*(volatile uint32_t *)(USART2_BASE + 0x0CUL))
#define USART2_CR2 (*(volatile uint32_t *)(USART2_BASE + 0x10UL))
#define USART2_CR3 (*(volatile uint32_t *)(USART2_BASE + 0x14UL))

void uart2_init(void)
{
    // 1. GPIOA 클럭 활성화
    RCC_AHB1ENR |= (1U << 0);

    // 2. USART2 클럭 활성화
    RCC_APB1ENR |= (1U << 17);

    // 3. PA2, PA3 Alternate Function 설정
    GPIOA_MODER &= ~((3U << 4) | (3U << 6));
    GPIOA_MODER |=  ((2U << 4) | (2U << 6));

    // 4. PA2, PA3 = AF7 (USART2)
    GPIOA_AFRL &= ~((0xFU << 8) | (0xFU << 12));
    GPIOA_AFRL |=  ((7U << 8) | (7U << 12));

    // 5. USART 비활성화 및 초기화
    USART2_CR1 = 0U;
    USART2_CR2 = 0U;
    USART2_CR3 = 0U;

    // 6. Baudrate = 9600
    // APB1 = 16MHz, OVER8 = 0
    USART2_BRR = 1667U;

    // 7. USART 활성화 (UE = bit 13)
    // TX 활성화 (TE = bit 3)
    // RX 활성화 (RE = bit 2)
    USART2_CR1 = (1U << 13) | (1U << 3) | (1U << 2);
}

void uart2_send_char(char c)
{
    // TXE(bit 7) = 1이면 송신 가능
    while ((USART2_SR & (1U << 7)) == 0U)
        ;

    USART2_DR = (uint8_t)c;
}

char uart2_receiver_char(void)
{
    // RXNE(bit 5) = 1이면 수신 완료
    while ((USART2_SR & (1U << 5)) == 0U)
        ;

    return (char)(USART2_DR & 0xFFU);
}

int main(void)
{
    uart2_init();

    while (1)
    {
        char received_char = uart2_receiver_char();

        // 수신된 ASCII 값에 1을 더해서 송신
        uart2_send_char((char)((uint8_t)received_char + 1U));
    }
}