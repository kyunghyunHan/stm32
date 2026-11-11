#include <stdint.h>

/*
 * STM32F429ZI USART3 UART Echo Back
 *
 * HSI / APB1 = 16MHz
 * Baudrate = 9600, 8N1
 *
 * PD8 = USART3_TX (AF7)
 * PD9 = USART3_RX (AF7)
 *
 * PC에서 'h' 입력 → STM32 수신 → 'h' 재전송
 */
/*
레지스터	주소	역할
USART3_SR	0x40004800	송수신 상태
USART3_DR	0x40004804	송수신 데이터
USART3_BRR	0x40004808	Baud Rate
USART3_CR1	0x4000480C	USART 제어
USART3_CR2	0x40004810	Stop Bit 등
USART3_CR3	0x40004814	추가 기능 제어
 */

// Base Address
#define RCC_BASE     0x40023800UL
#define GPIOD_BASE   0x40020C00UL
#define USART3_BASE  0x40004800UL

// RCC Register
#define RCC_AHB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x30UL))
#define RCC_APB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x40UL))

// GPIOD Register
#define GPIOD_MODER (*(volatile uint32_t *)(GPIOD_BASE + 0x00UL))
#define GPIOD_AFRH  (*(volatile uint32_t *)(GPIOD_BASE + 0x24UL))

// USART3 Register
#define USART3_SR  (*(volatile uint32_t *)(USART3_BASE + 0x00UL))
#define USART3_DR  (*(volatile uint32_t *)(USART3_BASE + 0x04UL))
#define USART3_BRR (*(volatile uint32_t *)(USART3_BASE + 0x08UL))
#define USART3_CR1 (*(volatile uint32_t *)(USART3_BASE + 0x0CUL))
#define USART3_CR2 (*(volatile uint32_t *)(USART3_BASE + 0x10UL))
#define USART3_CR3 (*(volatile uint32_t *)(USART3_BASE + 0x14UL))


// USART3 초기화
void uart3_init(void)
{
    // 1. GPIOD 클럭 활성화 (bit 3)
    RCC_AHB1ENR |= (1U << 3);

    // 2. USART3 클럭 활성화 (bit 18)
    RCC_APB1ENR |= (1U << 18);

    // 3. PD8, PD9 Alternate Function 설정 (10)
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
    // UE(bit 13) = 1
    // TE(bit 3)  = 1
    // RE(bit 2)  = 1
    USART3_CR1 = (1U << 13) |
                 (1U << 3)  |
                 (1U << 2);
}


// 문자 1개 송신
void uart3_send_char(char c)
{
    // TXE(bit 7) = 1이 될 때까지 대기
    while ((USART3_SR & (1U << 7)) == 0U)
        ;

    // 송신 데이터 레지스터에 문자 기록
    USART3_DR = (uint8_t)c;
}


// 문자 1개 수신
char uart3_receive_char(void)
{
    // RXNE(bit 5) = 1이 될 때까지 대기
    while ((USART3_SR & (1U << 5)) == 0U)
        ;

    // 수신 데이터 레지스터 읽기
    return (char)(USART3_DR & 0xFFU);
}


// Main
int main(void)
{
    // USART3 초기화
    uart3_init();

    while (1)
    {
        // 1. PC에서 문자 1개 수신
        char received = uart3_receive_char();

        // 2. 수신한 문자를 그대로 PC로 송신
        uart3_send_char(received);
    }
}
