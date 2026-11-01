#include <stdint.h>

/*
 * 실습: GPIO 입력값을 UART로 전송하기
 *
 * MCU   : STM32F446RE
 * UART  : USART2
 * Baud  : 9600 bps, 8N1
 * Clock : HSI 16MHz (APB1 = 16MHz 가정)
 *
 * GPIO
 * - PC13 : 버튼 입력 (B1 USER)
 * - PA2  : USART2_TX (AF7)
 *
 * 목표
 * 1. PC13 버튼의 입력값을 읽는다.
 * 2. UART로 '0' 또는 '1'을 전송한다.
 * 3. CoolTerm에서 버튼 상태를 확인한다.
 *
 * 주의
 * - NUCLEO-F446RE의 PC13 버튼은 일반적으로
 *   누름 = HIGH(1), 안 누름 = LOW(0)
 * - PA2는 보드의 ST-LINK Virtual COM Port에 연결된다.
 */

// 3. USART2 레지스터 주소 정의

/*
 * 함수 1: UART 초기화
 *
 * 1. GPIOA 클럭 활성화
 * 2. USART2 클럭 활성화
 * 3. PA2를 Alternate Function 모드(10)로 설정
 * 4. PA2의 Alternate Function을 AF7로 설정
 * 5. USART2_BRR 설정 (9600bps)
 * 6. USART2 송신 활성화 (TE)
 * 7. USART2 활성화 (UE)
 */
// 레지스터 주소 정의
#define RCC_BASE 0x40023800UL
#define RCC_AHB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x30UL))
#define RCC_APB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x40UL))

#define GPIOA_BASE 0x40020000UL
#define GPIOC_BASE 0x40020800UL

#define GPIOA_MODER (*(volatile uint32_t *)(GPIOA_BASE + 0x00UL))
#define GPIOC_MODER (*(volatile uint32_t *)(GPIOC_BASE + 0x00UL))

/*주변장치 연결 */
#define GPIOA_AFRL (*(volatile uint32_t *)(GPIOA_BASE + 0x20UL))

// 입력
#define GPIOC_IDR (*(volatile uint32_t *)(GPIOC_BASE + 0x10UL))

// UART2
#define USART2_BASE 0x40004400UL
#define USART2_SR (*(volatile uint32_t *)(USART2_BASE + 0x00UL))
#define USART2_DR (*(volatile uint32_t *)(USART2_BASE + 0x04UL))
#define USART2_BRR (*(volatile uint32_t *)(USART2_BASE + 0x08UL))
#define USART2_CR1 (*(volatile uint32_t *)(USART2_BASE + 0x0CUL))
void uart_init(void)
{

    // 클럭화렁화
    RCC_AHB1ENR |= (1U << 0);
    RCC_APB1ENR |= (1U << 17);

    GPIOA_MODER &= ~(3U << 4);
    GPIOA_MODER |= (2U << 4);

    GPIOA_AFRL &= ~(0xFU << 8);
    GPIOA_AFRL |= (7U << 8);

    USART2_BRR = 0x683U;
    // 송신 활성화
    USART2_CR1 |= (1U << 3);
    USART2_CR1 |= (1U << 13);
}

/*


*/

void uart_send_char(char c)
{
    while ((USART2_SR & (1U << 7)) == 0)
    {
    }
    // 추출
    //  return (char)(USART2_DR & (0xFFU));
    USART2_DR = (uint8_t)c;
}

void gpio_input_init(void)
{
    RCC_AHB1ENR |= (1U << 2);

    GPIOC_MODER &= ~(3U << 26);
}

int main(void)
{
    uart_init();
    gpio_input_init();

    uint32_t previous = (GPIOC_IDR >> 13) & 1U;

    while (1)
    {
        uint32_t button = (GPIOC_IDR >> 13) & 1U;

        if (button != previous)
        {
            uart_send_char((char)('0' + button));
            uart_send_char('\r');
            uart_send_char('\n');

            previous = button;
        }
    }
}