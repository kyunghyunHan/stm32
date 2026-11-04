#include <stdint.h>

#define RCC_BASE   0x40023800UL
#define GPIOA_BASE 0x40020000UL

#define RCC_AHB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x30UL))

#define GPIOA_MODER (*(volatile uint32_t *)(GPIOA_BASE + 0x00UL))
#define GPIOA_PUPDR (*(volatile uint32_t *)(GPIOA_BASE + 0x0CUL))
#define GPIOA_IDR   (*(volatile uint32_t *)(GPIOA_BASE + 0x10UL))
#define GPIOA_ODR   (*(volatile uint32_t *)(GPIOA_BASE + 0x14UL))

int main(void)
{
    // 1. GPIOA 클럭 활성화
    RCC_AHB1ENR |= (1U << 0);

    // 2. PA10 (D2) 외부 버튼 입력 모드 (00)
    GPIOA_MODER &= ~(3U << 20);

    // 3. PA10 내부 Pull-up 설정 (01)
    GPIOA_PUPDR &= ~(3U << 20);
    GPIOA_PUPDR |=  (1U << 20);

    // 4. PA8 (D7) 외부 LED 출력 모드 (01)
    GPIOA_MODER &= ~(3U << 16);
    GPIOA_MODER |=  (1U << 16);

    // 5. 외부 LED 초기 OFF
    GPIOA_ODR &= ~(1U << 8);

   while (1)
{
    // D2(PA10) 입력값을 내장 LD2(PA5)에 표시
    if ((GPIOA_IDR & (1U << 10)) == 0U)
    {
        GPIOA_ODR |= (1U << 8);
    }
    else
    {
        GPIOA_ODR &= ~(1U << 8);
    }
}
}