#include <stdint.h>

/*
 * STM32F429ZI USART3
 * HSI / APB1 = 16MHz
 * Baudrate = 9600, 8N1
 * PD8 = TX (AF7)
 * PD9 = RX (AF7)
 *
 * ST-LINK Virtual COM Port
 */

#define RCC_BASE     0x40023800UL
#define GPIOD_BASE   0x40020C00UL
#define USART3_BASE  0x40004800UL

// RCC
#define RCC_AHB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x30UL))
#define RCC_APB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x40UL))

// GPIOD
#define GPIOD_MODER (*(volatile uint32_t *)(GPIOD_BASE + 0x00UL))
#define GPIOD_AFRH  (*(volatile uint32_t *)(GPIOD_BASE + 0x24UL))

// USART3
#define USART3_SR  (*(volatile uint32_t *)(USART3_BASE + 0x00UL))
#define USART3_DR  (*(volatile uint32_t *)(USART3_BASE + 0x04UL))
#define USART3_BRR (*(volatile uint32_t *)(USART3_BASE + 0x08UL))
#define USART3_CR1 (*(volatile uint32_t *)(USART3_BASE + 0x0CUL))
#define USART3_CR2 (*(volatile uint32_t *)(USART3_BASE + 0x10UL))
#define USART3_CR3 (*(volatile uint32_t *)(USART3_BASE + 0x14UL))

void uart3_init(void)
{
    // 1. GPIOD 클럭 활성화 (bit 3)
    RCC_AHB1ENR |= (1U << 3);

    // 2. USART3 클럭 활성화 (bit 18)
    RCC_APB1ENR |= (1U << 18);

    // 3. PD8, PD9 Alternate Function (10)
    GPIOD_MODER &= ~((3U << 16) | (3U << 18));
    GPIOD_MODER |=  ((2U << 16) | (2U << 18));

    // 4. PD8, PD9 = AF7 (USART3)
    GPIOD_AFRH &= ~((0xFU << 0) | (0xFU << 4));
    GPIOD_AFRH |=  ((7U << 0) | (7U << 4));

    // 5. USART 비활성화 및 초기화
    USART3_CR1 = 0U;
    USART3_CR2 = 0U;
    USART3_CR3 = 0U;

    // 6. Baudrate = 9600
    // APB1 = 16MHz, OVER8 = 0
    USART3_BRR = 1667U;

    // 7. USART 활성화
    // UE(bit13), TE(bit3), RE(bit2)
    USART3_CR1 = (1U << 13) | (1U << 3) | (1U << 2);
}

// 문자 1개 송신
void uart3_send_char(char c)
{
    // TXE(bit7) = 1이면 송신 가능
    while ((USART3_SR & (1U << 7)) == 0U)
        ;

    USART3_DR = (uint8_t)c;
}

// 문자열 송신
void uart3_send_string(const char *str)
{
    while (*str != '\0')
    {
        uart3_send_char(*str);
        str++;
    }
}

// 문자 1개 수신
char uart3_receive_char(void)
{
    // RXNE(bit5) = 1이면 수신 완료
    while ((USART3_SR & (1U << 5)) == 0U)
        ;

    return (char)(USART3_DR & 0xFFU);
}

int main(void)
{
    uart3_init();

    // 부팅 시 Hello World 출력
    uart3_send_string("Hello World!\r\n");

    while (1)
    {
        // PC에서 문자 수신
        char received = uart3_receive_char();

        // 수신한 문자를 다시 PC로 전송
        uart3_send_char(received);
    }
}