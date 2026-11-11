#include <stdint.h>

/*
 * STM32F446RE UART 외부 LED 제어
 *
 * '1' 수신 -> 외부 LED ON
 * '0' 수신 -> 외부 LED OFF
 * 다른 문자 -> LED 상태 유지
 *
 * MCU       : STM32F446RE
 * External LED : PA8 (D7)
 * USART2 TX : PA2 (AF7)
 * USART2 RX : PA3 (AF7)
 *
 * Clock     : HSI 16MHz, APB1 16MHz
 * UART      : 9600bps, 8N1
 *
 * HAL 없이 레지스터 직접 접근
 */

#define RCC_BASE    0x40023800UL
#define GPIOA_BASE  0x40020000UL
#define USART2_BASE 0x40004400UL

// RCC
#define RCC_AHB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x30UL))
#define RCC_APB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x40UL))

// GPIOA
#define GPIOA_MODER  (*(volatile uint32_t *)(GPIOA_BASE + 0x00UL))
#define GPIOA_OTYPER (*(volatile uint32_t *)(GPIOA_BASE + 0x04UL))
#define GPIOA_ODR    (*(volatile uint32_t *)(GPIOA_BASE + 0x14UL))
#define GPIOA_AFRL   (*(volatile uint32_t *)(GPIOA_BASE + 0x20UL))

// USART2
#define USART2_SR  (*(volatile uint32_t *)(USART2_BASE + 0x00UL))
#define USART2_DR  (*(volatile uint32_t *)(USART2_BASE + 0x04UL))
#define USART2_BRR (*(volatile uint32_t *)(USART2_BASE + 0x08UL))
#define USART2_CR1 (*(volatile uint32_t *)(USART2_BASE + 0x0CUL))
#define USART2_CR2 (*(volatile uint32_t *)(USART2_BASE + 0x10UL))
#define USART2_CR3 (*(volatile uint32_t *)(USART2_BASE + 0x14UL))

#define EXT_LED_PIN 8U

void UART2_Init(void)
{
    // 1. GPIOA 클럭 활성화
    RCC_AHB1ENR |= (1U << 0);

    // 2. USART2 클럭 활성화
    RCC_APB1ENR |= (1U << 17);

    // 3. PA8 외부 LED 초기 LOW
    GPIOA_ODR &= ~(1U << EXT_LED_PIN);

    // PA8 Push-Pull
    GPIOA_OTYPER &= ~(1U << EXT_LED_PIN);

    // PA8 Output (01)
    GPIOA_MODER &= ~(3U << 16);
    GPIOA_MODER |=  (1U << 16);

    // 4. PA2 TX, PA3 RX : AF 모드 (10)
    GPIOA_MODER &= ~((3U << 4) | (3U << 6));
    GPIOA_MODER |=  ((2U << 4) | (2U << 6));

    // 5. PA2, PA3 : AF7
    GPIOA_AFRL &= ~((0xFU << 8) | (0xFU << 12));
    GPIOA_AFRL |=  ((7U << 8) | (7U << 12));

    // 6. USART2 초기화
    USART2_CR1 = 0U;
    USART2_CR2 = 0U;
    USART2_CR3 = 0U;

    // 7. 9600bps (APB1 = 16MHz)
    USART2_BRR = 1667U;

    // 8. TE, RE, UE 활성화
    USART2_CR1 |= (1U << 3);  // TX Enable
    USART2_CR1 |= (1U << 2);  // RX Enable
    USART2_CR1 |= (1U << 13); // USART Enable
}

// UART 문자 수신
char UART2_ReceiveChar(void)
{
    // RXNE(bit 5)가 1이 될 때까지 대기
    while ((USART2_SR & (1U << 5)) == 0U)
    {
    }

    return (char)(USART2_DR & 0xFFU);
}

int main(void)
{
    UART2_Init();

    while (1)
    {
        char received_char = UART2_ReceiveChar();

        if (received_char == '1')
        {
            // 외부 LED ON (PA8 HIGH)
            GPIOA_ODR |= (1U << EXT_LED_PIN);
        }
        else if (received_char == '0')
        {
            // 외부 LED OFF (PA8 LOW)
            GPIOA_ODR &= ~(1U << EXT_LED_PIN);
        }
    }
}