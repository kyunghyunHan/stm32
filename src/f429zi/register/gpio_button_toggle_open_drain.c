#include <stdint.h>

#define RCC_BASE    0x40023800UL
#define GPIOF_BASE  0x40021400UL

#define RCC_AHB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x30UL))

#define GPIOF_MODER  (*(volatile uint32_t *)(GPIOF_BASE + 0x00UL))
#define GPIOF_OTYPER (*(volatile uint32_t *)(GPIOF_BASE + 0x04UL))
#define GPIOF_PUPDR  (*(volatile uint32_t *)(GPIOF_BASE + 0x0CUL))
#define GPIOF_IDR    (*(volatile uint32_t *)(GPIOF_BASE + 0x10UL))
#define GPIOF_ODR    (*(volatile uint32_t *)(GPIOF_BASE + 0x14UL))

int main(void)
{
    // 1. GPIOF 클럭 활성화
    RCC_AHB1ENR |= (1U << 5);

    // 2. PF14(D4) 입력 모드 00
    GPIOF_MODER &= ~(3U << 28);

    // 3. PF14 내부 Pull-up 설정 01
    GPIOF_PUPDR &= ~(3U << 28);
    GPIOF_PUPDR |=  (1U << 28);

    // 4. PF15(D2) 초기값 HIGH 명령 (LED OFF)
    GPIOF_ODR |= (1U << 15);

    // 5. PF15 Open-Drain 출력 설정
    GPIOF_OTYPER |= (1U << 15);

    // 6. PF15 출력 모드 01
    GPIOF_MODER &= ~(3U << 30);
    GPIOF_MODER |=  (1U << 30);

    // 7. 이전 버튼 상태 저장
    uint32_t previous_button = (GPIOF_IDR >> 14) & 1U;

    while (1)
    {
        // PF14 외부 버튼 읽기
        uint32_t button = (GPIOF_IDR >> 14) & 1U;

        // HIGH -> LOW : 버튼 눌림 감지
        if (previous_button == 1U && button == 0U)
        {
            GPIOF_ODR ^= (1U << 15);
        }

        previous_button = button;
    }
}