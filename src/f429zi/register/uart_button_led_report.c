#include <stdint.h>

#define RCC_BASE    0x40023800UL
#define GPIOA_BASE  0x40020000UL
#define USART2_BASE 0x40004400UL

// RCC
#define RCC_AHB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x30UL))
#define RCC_APB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x40UL))

// GPIOA
#define GPIOA_MODER  (*(volatile uint32_t *)(GPIOA_BASE + 0x00UL))
#define GPIOA_OTYPER (*(volatile uint32_t *)(GPIOA_BASE + 0x04UL))
#define GPIOA_PUPDR  (*(volatile uint32_t *)(GPIOA_BASE + 0x0CUL))
#define GPIOA_IDR    (*(volatile uint32_t *)(GPIOA_BASE + 0x10UL))
#define GPIOA_ODR    (*(volatile uint32_t *)(GPIOA_BASE + 0x14UL))
#define GPIOA_AFRL   (*(volatile uint32_t *)(GPIOA_BASE + 0x20UL))

// USART2
#define USART2_SR  (*(volatile uint32_t *)(USART2_BASE + 0x00UL))
#define USART2_DR  (*(volatile uint32_t *)(USART2_BASE + 0x04UL))
#define USART2_BRR (*(volatile uint32_t *)(USART2_BASE + 0x08UL))
#define USART2_CR1 (*(volatile uint32_t *)(USART2_BASE + 0x0CUL))
#define USART2_CR2 (*(volatile uint32_t *)(USART2_BASE + 0x10UL))
#define USART2_CR3 (*(volatile uint32_t *)(USART2_BASE + 0x14UL))

void UART2_Init(void)
{
    // 1. GPIOA, USART2 클럭 활성화
    RCC_AHB1ENR |= (1U << 0);
    RCC_APB1ENR |= (1U << 17);

    // 2. PA10 (D2) 버튼 입력 모드 (00)
    GPIOA_MODER &= ~(3U << 20);

    // 3. PA10 내부 Pull-down (10)
    GPIOA_PUPDR &= ~(3U << 20);
    GPIOA_PUPDR |=  (2U << 20);

    // 4. PA8 (D7) 외부 LED 출력 모드 (01)
    GPIOA_ODR &= ~(1U << 8);
    GPIOA_OTYPER &= ~(1U << 8);

    GPIOA_MODER &= ~(3U << 16);
    GPIOA_MODER |=  (1U << 16);

    // 5. PA2 USART2_TX : AF 모드 (10)
    GPIOA_MODER &= ~(3U << 4);
    GPIOA_MODER |=  (2U << 4);

    // 6. PA2 : AF7
    GPIOA_AFRL &= ~(0xFU << 8);
    GPIOA_AFRL |=  (7U << 8);

    // 7. USART2 초기화
    USART2_CR1 = 0U;
    USART2_CR2 = 0U;
    USART2_CR3 = 0U;

    // 8. APB1 16MHz, Baudrate 9600bps
    USART2_BRR = 1667U;

    // 9. 송신 및 USART 활성화
    USART2_CR1 |= (1U << 3);  // TE
    USART2_CR1 |= (1U << 13); // UE
}

// UART 문자 1개 송신
void UART2_SendChar(char data)
{
    while ((USART2_SR & (1U << 7)) == 0U)
    {
    }

    USART2_DR = (uint8_t)data;
}

// UART 문자열 송신
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

    uint32_t prev_button = 0U;

    while (1)
    {
        // PA10 버튼 상태 확인
        uint32_t button = (GPIOA_IDR >> 10) & 1U;

        // 버튼 누르면 LED ON
        if (button == 1U)
        {
            GPIOA_ODR |= (1U << 8);
        }
        else
        {
            GPIOA_ODR &= ~(1U << 8);
        }

        // 버튼을 새로 누른 순간 UART 출력
        if (button == 1U && prev_button == 0U)
        {
            UART2_SendString("Button Pressed!\r\n");
        }

        prev_button = button;
    }
}