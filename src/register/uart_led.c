
#include <stdint.h>

/*
 * STM32F446RE UART 문자로 LED 제어
 *
 * '1' 수신 → LED ON
 * '0' 수신 → LED OFF
 * 다른 문자 → LED 상태 유지
 *
 * MCU       : STM32F446RE
 * LED       : PA5 (HIGH = ON, LOW = OFF)
 * USART2 TX : PA2 (AF7)
 * USART2 RX : PA3 (AF7)
 * Clock     : HSI = 16MHz, APB1 = 16MHz 가정
 * Baudrate  : 9600bps, 8N1
 *
 * HAL 없이 레지스터 직접 접근
 *
 * 참고:
 * RM0390 - STM32F446 Reference Manual
 * UM1724 - NUCLEO-64 Board User Manual
 *
 *
Moder

00	Input
01	Output
10	Alternate Function
11	Analog
 */

// 1. RCC_BASE 주소 정의
#define RCC_BASE 0x40023800UL

// 2. GPIOA_BASE 주소 정의
// GPIOA는 LED(PA5), USART2(PA2/PA3)에 사용
#define GPIOA_BASE 0x40020000UL

// 3. GPIOD_BASE 주소 정의
// 참고용: 현재 실습에서는 사용하지 않음
#define GPIOD_BASE 0x40020C00UL

// 4. USART2_BASE 주소 정의
#define USART2_BASE 0x40004400UL

// ==========================================
// 2. RCC 레지스터 정의
// ==========================================

// RCC_AHB1ENR 레지스터 정의
// Offset : 0x30
// GPIOA 클럭 활성화에 사용
#define RCC_AHB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x30))

// RCC_APB1ENR 레지스터 정의
// Offset : 0x40
// USART2 클럭 활성화에 사용
#define RCC_APB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x40))
// ==========================================
// GPIO 레지스터 정의
// ==========================================

// GPIOA_MODER 레지스터 정의
//  Offset : 0x00
//  PA2, PA3, PA5 모드 설정
#define GPIOA_MODER (*(volatile uint32_t *)(GPIOA_BASE + 0x00))
// GPIOA_ODR 레지스터 정의
// Offset : 0x14
// PA5 LED 출력 제어
#define GPIOA_ODR (*(volatile uint32_t *)(GPIOA_BASE + 0x14))
// GPIOA_AFRL 레지스터 정의
// PA2, PA3을 AF7로 설정
#define GPIOA_AFRL (*(volatile uint32_t *)(GPIOA_BASE + 0x20))
// ==========================================
// USART2 레지스터 정의
// STM32F446RE 기준 (RM0390)
// ==========================================

// USART2_SR 레지스터 정의
// Offset : 0x00
// TXE(bit 7), RXNE(bit 5) 확인
#define USART2_SR (*(volatile uint32_t *)(USART2_BASE + 0x00))
// USART2_DR 레지스터 정의
//  Offset : 0x04
//  수신 데이터 읽기 / 송신 데이터 쓰기
#define USART2_DR (*(volatile uint32_t *)(USART2_BASE + 0x04))

// USART2_BRR 레지스터 정의
//  Offset : 0x08
//  Baudrate 설정
#define USART2_BRR (*(volatile uint32_t *)(USART2_BASE + 0x08))

// USART2_CR1 레지스터 정의
//  Offset : 0x0C
//  UE(bit 13), TE(bit 3), RE(bit 2)
#define USART2_CR1 (*(volatile uint32_t *)(USART2_BASE + 0x0C))

// 14. USART2_CR2 레지스터 정의
// Offset : 0x10
// Stop bit 설정 등
#define USART2_CR2 (*(volatile uint32_t *)(USART2_BASE + 0x10))

// USART2_CR3 레지스터 정의
//  Offset : 0x14
//  DMA, 하드웨어 흐름 제어 등
#define USART2_CR3 (*(volatile uint32_t *)(USART2_BASE + 0x14))

// ==========================================
// 5. USART2 초기화 함수
// ==========================================

void UART2_Init(void)
{
    // GPIOA 클럭 활성화
    // RCC_AHB1ENR bit 0
    RCC_AHB1ENR |= (1U << 0);
    // USART2 클럭 활성화
    // RCC_APB1ENR bit 17
    RCC_APB1ENR |= (1U << 17);
    // PA5를 Output 모드(01)로 설정
    // MODER에서 PA5 = bit 11:10
    GPIOA_MODER &= ~(3U << 10);
    GPIOA_MODER |= (1U << 10);
    // LED 초기 상태 OFF
    // PA5 LOW(0) → LED OFF
    GPIOA_ODR &= ~(1U << 5);
    // 20. PA2, PA3을 Alternate Function 모드(10)로 설정
    // PA2 = MODER bit 5:4
    // PA3 = MODER bit 7:6
    GPIOA_MODER &= ~((3U << 4) | (3U << 6));
    GPIOA_MODER |= ((2U << 4) | (3U << 6));
    // PA2, PA3을 AF7로 설정
    // AFRL은 핀당 4비트
    // PA2 = bit 11:8
    // PA3 = bit 15:12
    GPIOA_AFRL &= ~((0xFU << 8) | (0xFU << 12));
    GPIOA_AFRL |= ((0x7U << 8) | (0x7U << 12));
    // USART2 비활성화 및 CR1, CR2, CR3 초기화
    USART2_CR1 = 0U;
    USART2_CR2 = 0U;
    USART2_CR3 = 0U;
    // 23. Baudrate를 9600bps로 설정
    // APB1 = 16MHz, OVER8 = 0
    // BRR = 16000000 / 9600 ≈ 1667
    USART2_BRR = 1667U;
    // USART2 TX 활성화
    // CR1 TE = bit 3
    USART2_CR1 |= (1U << 3);
    // 25. USART2 RX 활성화
    // CR1 RE = bit 2
    USART2_CR1 |= (1U << 2);
    // 26. USART2 활성화
    // CR1 UE = bit 13
    USART2_CR1 |= (1U << 13);
}

// ==========================================
// 6. UART 문자 수신 함수
// ==========================================

char UART2_ReceiveChar(void)
{
    // 27. RXNE 플래그가 1이 될 때까지 대기
    // SR bit 5
    // Polling 방식
    while (USART2_SR & (1U << 5) == 0U)
        ;
    return (char)(USART2_DR & (0xFFU));
}

// ==========================================
// 7. 메인 함수
// ==========================================

int main(void)
{
    char received_char = 0;

    USART_init();

    while (1)
    {
        // 30. UART로 문자 1개 수신
        char receiver_char = UART2_ReceiveChar();
        // 31. 수신 문자가 '1'이면

        if (receiver_char == '1')
        {
            GPIOA_ODR |= (1U << 5);
        }
        if (receiver_char == '0')
        {
            GPIOA_ODR &= ~(1U << 5);
        }
    }
}
