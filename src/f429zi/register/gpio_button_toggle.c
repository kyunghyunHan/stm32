#include <stdint.h>

#define RCC_BASE   0x40023800UL
#define GPIOC_BASE 0x40020800UL
#define GPIOF_BASE 0x40021400UL

#define RCC_AHB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x30UL))

#define GPIOC_MODER (*(volatile uint32_t *)(GPIOC_BASE + 0x00UL))
#define GPIOC_IDR   (*(volatile uint32_t *)(GPIOC_BASE + 0x10UL))

#define GPIOF_MODER (*(volatile uint32_t *)(GPIOF_BASE + 0x00UL))
#define GPIOF_ODR   (*(volatile uint32_t *)(GPIOF_BASE + 0x14UL))

int main(void)
{
    // 1. GPIOC 클럭 활성화 (버튼)
    RCC_AHB1ENR |= (1U << 2);

    // 2. GPIOF 클럭 활성화 (LED)
    RCC_AHB1ENR |= (1U << 5);

    // 3. PC13 입력 모드 (00)
    GPIOC_MODER &= ~(3U << 26);

    // 4. PF15 초기 LOW
    GPIOF_ODR &= ~(1U << 15);

    // 5. PF15 출력 모드 (01)
    GPIOF_MODER &= ~(3U << 30);
    GPIOF_MODER |=  (1U << 30);

    // 6. 버튼 이전 상태
    uint32_t previous_button = (GPIOC_IDR >> 13) & 1U;

    while (1)
    {
        uint32_t button = (GPIOC_IDR >> 13) & 1U;

        // 이전 HIGH -> 현재 LOW
        if (previous_button == 1U && button == 0U)
        {
            GPIOF_ODR ^= (1U << 15);
        }

        previous_button = button;
    }
}
