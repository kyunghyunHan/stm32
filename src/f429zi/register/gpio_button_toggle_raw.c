// 버튼 입력을 직접 읽어 LED를 토글하는 기초 실습
// #include "stm32f446xx.h"

#include <stdint.h>

#define RCC_BASE 0x40023800UL
// 연결버스 레지스터 시작주소
#define RCC_AHB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x30U))
// GPIOC BASE
#define GPIOC_BASE 0x40020800UL
#define GPIOA_BASE 0x40020000UL

#define GPIOC_MODER (*(volatile uint32_t *)(GPIOC_BASE + 0x00UL))
#define GPIOA_MODER (*(volatile uint32_t *)(GPIOA_BASE + 0x00UL))

#define GPIOC_IDR (*(volatile uint32_t *)(GPIOC_BASE + 0x10UL))

#define GPIOA_ODR (*(volatile uint32_t *)(GPIOA_BASE + 0x14UL))
int main(void)
{

    // GPIOC 활성화

    RCC_AHB1ENR |= (1U << 2);
    // GPIOA활성화
    RCC_AHB1ENR |= (1U << 0);
    // 수정: 26번과 27번 비트를 0으로 만듦
    // MODER은 핀하나에 2비트
    // iNPUT 00 13PC
    GPIOC_MODER &= ~(0x3U << 26);
    // OUTPUT PA5
    // 0으로 먼저 초기화 2비트
    // GPIOA_MODER &= ~(0x3U << 10);
    // GPIOA_MODER |= (0x1U << 10);
    // PA10 출력 모드 설정
    GPIOA_MODER &= ~(3U << 20);
    GPIOA_MODER |= (1U << 20);

    volatile uint32_t button = 0;

    // while (1)
    // {
    //     /*
    //      * B1 버튼: Active-Low (Pull-up 회로)
    //      *
    //      * [버튼을 누르지 않았을 때]
    //      * 1. 버튼 스위치가 열려 있다.
    //      * 2. Pull-up 저항을 통해 PC13이 3.3V 쪽으로 연결된다.
    //      * 3. PC13 입력 전압이 약 3.3V가 된다.
    //      * 4. 입력 슈미트 트리거가 HIGH로 판별한다.
    //      * 5. GPIOC_IDR bit 13 = 1
    //      *
    //      * [버튼을 눌렀을 때]
    //      * 1. 버튼 스위치가 닫힌다.
    //      * 2. PC13이 GND에 연결된다.
    //      * 3. PC13 입력 전압이 약 0V로 내려간다.
    //      * 4. 입력 슈미트 트리거가 LOW로 판별한다.
    //      * 5. GPIOC_IDR bit 13 = 0
    //      */

    //     // IDR의 13번 비트만 추출
    //     //PULL up방식으로 연결대어있음
    //     //0이면 High
    //     //1이면 LOW
    //     button = (GPIOC_IDR >> 13) & 1U;

    //     if (button == 0U)
    //     {
    //         /*
    //          * 버튼 눌림
    //          *
    //          * PC13 LOW → IDR bit 13 = 0
    //          *
    //          * ODR bit 5를 1로 설정
    //          * → PA5 Push-Pull 상단 PMOS ON
    //          * → PA5 HIGH
    //          * → LED ON
    //          */

    //         GPIOA_ODR |= (1U << 5);
    //     }
    //     else
    //     {
    //         /*
    //          * 버튼 안 눌림
    //          *
    //          * PC13 HIGH → IDR bit 13 = 1
    //          *
    //          * ODR bit 5를 0으로 설정
    //          * → PA5 Push-Pull 하단 NMOS ON
    //          * → PA5 LOW
    //          * → LED OFF
    //          */
    //         GPIOA_ODR &= ~(1U << 5);
    //     }
    // }

    uint32_t previous_button = 0U;

    while (1)
    {
        uint32_t button = (GPIOC_IDR >> 13) & 1U;

        // 이전 HIGH → 현재 LOW인 순간만 감지
        if (previous_button == 1U && button == 0U)
        {
            GPIOA_ODR ^= (1U << 10);
        }

        // 현재 버튼 상태를 다음 반복을 위해 저장
        previous_button = button;
    }
}

