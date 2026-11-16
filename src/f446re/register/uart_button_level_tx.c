// 버튼 입력값이 바뀌면 UART로 0 또는 1을 전송하는 실습
#include <stdint.h>

// 3. USART2 레지스터 주소 정의

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