#include <stdint.h>

#define RCC_BASE   0x40023800UL
#define GPIOA_BASE 0x40020000UL
#define GPIOC_BASE 0x40020800UL

#define RCC_AHB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x30UL))

#define GPIOA_MODER (*(volatile uint32_t *)(GPIOA_BASE + 0x00UL))
#define GPIOA_ODR   (*(volatile uint32_t *)(GPIOA_BASE + 0x14UL))

#define GPIOC_MODER (*(volatile uint32_t *)(GPIOC_BASE + 0x00UL))
#define GPIOC_IDR   (*(volatile uint32_t *)(GPIOC_BASE + 0x10UL))

int main(void)
{
    // GPIOA, GPIOC 클럭 활성화
    RCC_AHB1ENR |= (1U << 0);
    RCC_AHB1ENR |= (1U << 2);

    // PA8 (D7) 출력 모드 : 01
    GPIOA_MODER &= ~(3U << 16);
    GPIOA_MODER |=  (1U << 16);

    // PC13 (B1) 입력 모드 : 00
    GPIOC_MODER &= ~(3U << 26);

    // 처음에는 LED OFF
    GPIOA_ODR &= ~(1U << 8);

    while (1)
    {
        // B1 버튼을 누르면 PC13 = 0
        if ((GPIOC_IDR & (1U << 13)) == 0U)
        {
            // LED ON
            GPIOA_ODR |= (1U << 8);
        }
        else
        {
            // LED OFF
            GPIOA_ODR &= ~(1U << 8);
        }
    }
}