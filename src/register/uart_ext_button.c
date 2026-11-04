#include <stdint.h>

#define RCC_BASE    0x40023800UL
#define GPIOA_BASE  0x40020000UL
#define USART2_BASE 0x40004400UL

#define RCC_AHB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x30UL))
#define RCC_APB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x40UL))

#define GPIOA_MODER (*(volatile uint32_t *)(GPIOA_BASE + 0x00UL))
#define GPIOA_PUPDR (*(volatile uint32_t *)(GPIOA_BASE + 0x0CUL))
#define GPIOA_AFRL  (*(volatile uint32_t *)(GPIOA_BASE + 0x20UL))
#define GPIOA_IDR   (*(volatile uint32_t *)(GPIOA_BASE + 0x10UL))

#define USART2_SR   (*(volatile uint32_t *)(USART2_BASE + 0x00UL))
#define USART2_DR   (*(volatile uint32_t *)(USART2_BASE + 0x04UL))
#define USART2_BRR  (*(volatile uint32_t *)(USART2_BASE + 0x08UL))
#define USART2_CR1  (*(volatile uint32_t *)(USART2_BASE + 0x0CUL))
#define USART2_CR2  (*(volatile uint32_t *)(USART2_BASE + 0x10UL))
#define USART2_CR3  (*(volatile uint32_t *)(USART2_BASE + 0x14UL))

void UART2_Init(void)
{
    // 1. GPIOA, USART2 클럭 활성화
    RCC_AHB1ENR |= (1U << 0);
    RCC_APB1ENR |= (1U << 17);

    // 2. PA10 (D2) 입력 모드 (00)
    GPIOA_MODER &= ~(3U << 20);

    // 3. PA10 내부 Pull-down (10)
    GPIOA_PUPDR &= ~(3U << 20);
    GPIOA_PUPDR |=  (2U << 20);

    // 4. PA2 USART2_TX : AF 모드 (10)
    GPIOA_MODER &= ~(3U << 4);
    GPIOA_MODER |=  (2U << 4);

    // 5. PA2 : AF7
    GPIOA_AFRL &= ~(0xFU << 8);
    GPIOA_AFRL |=  (7U << 8);

    // 6. USART2 설정
    USART2_CR1 = 0U;
    USART2_CR2 = 0U;
    USART2_CR3 = 0U;

    // APB1 = 16MHz, 9600bps
    USART2_BRR = 1667U;

    // TE(bit3) 및 UE(bit13)
    USART2_CR1 |= (1U << 3);
    USART2_CR1 |= (1U << 13);
}

// UART 문자 하나 송신
void UART2_SendChar(char data)
{
    // TXE(bit7) = 1이 될 때까지 대기
    while ((USART2_SR & (1U << 7)) == 0U)
    {
    }

    USART2_DR = (uint8_t)data;
}

// 문자열 송신
void UART2_SendString(const char *str)
{
    while (*str != '\0')
    {
        UART2_SendChar(*str);
        str++;
    }
}

int main(void)
{
    UART2_Init();

    // 이전 버튼 상태: 0 = 안 누름
    uint32_t prev_button = 0U;

    while (1)
    {
        // PA10 버튼 현재 상태
        uint32_t button = (GPIOA_IDR >> 10) & 1U;

        // 버튼이 0 -> 1로 변경되었을 때만 전송
        if (button == 1U && prev_button == 0U)
        {
            UART2_SendString("Button Pressed!\r\n");
        }

        // 현재 상태 저장
        prev_button = button;
    }
}