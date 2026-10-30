#include <stdint.h>

/*
조건 - HSI = 16MHz
     - APB1 = 16MHz
     USART2 = 9600bps, 8N1
     PD5 = TX, PD6 = RX
*/

#define RCC_BASE 0x40023800UL
#define GPIOD_BASE 0x40020C00UL
#define USART2_BASE 0x40004400UL

#define RCC_AHB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x30UL))
#define RCC_APB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x40UL))

#define GPIOD_MODER (*(volatile uint32_t *)(GPIOD_BASE + 0x00UL))
#define GPIOD_AFRL (*(volatile uint32_t *)(GPIOD_BASE + 0x20UL))

#define USART2_CR1 (*(volatile uint32_t *)(USART2_BASE + 0x00UL))
#define USART2_CR2 (*(volatile uint32_t *)(USART2_BASE + 0x04UL))
#define USART2_CR3 (*(volatile uint32_t *)(USART2_BASE + 0x08UL))
#define USART2_BRR (*(volatile uint32_t *)(USART2_BASE + 0x0CUL))
#define USART2_ISR (*(volatile uint32_t *)(USART2_BASE + 0x1CUL))
#define USART2_RDR (*(volatile uint32_t *)(USART2_BASE + 0x24UL))
#define USART2_TDR (*(volatile uint32_t *)(USART2_BASE + 0x28UL))

void uart2_init(void)
{
    // GPIOD 클럭 활성화

    RCC_AHB1ENR |= (1U << 3);

    // USART2 클럭 활성화 bit 17
    RCC_APB1ENR |= (1U << 17);

    GPIOD_MODER &= ~((3U << 10) | (3U << 12));
    GPIOD_MODER |= ((2U << 10) | (2U << 12));

    // pd5,pd6을 AF7(0111)로 설정
    GPIOD_AFRL &= ~((0xFU << 20) | (0xFU << 24));
    GPIOD_AFRL |= ((7U << 20) | (7U << 24));

    // 9. USART2 비활성화 및 설정 초기화
    USART2_CR1 = 0U;
    USART2_CR2 = 0U;
    USART2_CR3 = 0U;

    // BaudRate설정
    //  APB1 = 16MHz, 9600bps, OVER8 = 0
    //  BRR = 16000000 / 9600 ≈ 1667 (0x683)
    USART2_BRR = 1667U;

    // 11. TX(bit 3), RX(bit 2) 활성화
    USART2_CR1 |= (1U << 3) | (1U << 2);

    // 12. USART 활성화: UE(bit 0) = 1
    USART2_CR1 |= (1U << 0);
}

void uart2_send_char(char c)

{ // TXE(bit 7) = 1이면 송신 데이터 기록 가능

    while ((USART2_ISR & (1U << 7)) == 0U)
        ;
    USART2_TDR = (uint8_t)c;
}

void uart2_receiver_char(void)
{

    // RXNE(bit 5) = 1이면 수신 데이터 존재
    while ((USART2_ISR & (1U << 5)) == 0U)
        ;

    return (char)(USART2_RDR & 0xFFU);
}

int main(void)
{
    uint8_t received_char = 0u;

    uart2_init();
    while (1)
    {
        received_char = (uint8_t)UART2_ReceiveChar();
        // 수신 문자 ASCII 값에 1을 더해서 송신
        UART2_SendChar((char)(received_char + 1U));
    }
}