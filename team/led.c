
#include <stdint.h>

#define RCC_BASE 0x40023800UL
#define RCC_AHB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x30UL))
#define GPIOB_BASE 0x40020400UL
#define GPIOB_MODER (*(volatile uint32_t *)(GPIOB_BASE + 0x00UL))
#define GPIOB_ODR (*(volatile uint32_t *)(GPIOB_BASE + 0x14UL))

// Time

#define SYSTICK_BASE 0xE000E010UL
#define SYSTICK_CTRL (*(volatile uint32_t *)(SYSTICK_BASE + 0x00UL))

/*
 * SysTick Reload Value Register
 *
 * LOAD Offset = 0x04
 * 실제 주소    = 0xE000E014
 *
 * 카운터가 다시 시작할 값을 설정
 *
 * SysTick은 Down Counter.
 *
 * Clock = 16MHz
 *
 * 1초 = 16,000,000 Clock
 * 1ms = 16,000 Clock
 *
 * LOAD = 16000 - 1
 *      = 15999
 *
 * 24비트 레지스터이므로
 * 설정값은 0xFFFFFF 이하이어야 함.
 */

#define SYSTICK_LOAD (*(volatile uint32_t *)(SYSTICK_BASE + 0x04UL))

/*
 * SysTick Current Value Register
 *
 * VAL Offset = 0x08
 * 실제 주소   = 0xE000E018
 *
 * 현재 카운터 값을 확인하는 레지스터
 *
 * VAL에 값을 기록하면
 * 현재 카운터가 0으로 초기화되고
 * COUNTFLAG도 클리어됨.
 *
 * 동작 예:
 *
 * LOAD = 15999
 * VAL  = 0 (초기화)
 *
 * SysTick 활성화
 *
 * 15999
 * 15998
 * 15997
 * ...
 * 2
 * 1
 * 0
 *
 * 0에 도달하면 COUNTFLAG 설정.
 *
 * 이후 다시 LOAD 값으로 재시작.
 */

#define SYSTICK_VAL (*(volatile uint32_t *)(SYSTICK_BASE + 0x08UL))

/*=========================================================
 * 4. CPU Clock
 *=========================================================*/

/*
 * HSI : High Speed Internal Oscillator
 *
 * STM32F429 내부 고속 RC 오실레이터
 * HSI 주파수 = 약 16MHz
 *
 * 여기서는 시스템 클럭이
 * HSI 16MHz인 상태를 가정.
 *
 * 주의:
 * PLL이 활성화되어 시스템 클럭이
 * 변경되었다면 이 값도 변경해야 함.
 */

#define CPU_CLOCK_HZ 16000000UL

/*=========================================================
 * 5. delay_ms()
 *=========================================================*/

/*
 * delay_ms(uint32_t ms)
 *
 * SysTick을 사용하여
 * 밀리초 단위 시간 지연을 수행.
 *
 * 예:
 *
 * delay_ms(1000); = 약 1초
 * delay_ms(3000); = 약 3초
 *
 * Polling 방식:
 *
 * CPU가 COUNTFLAG를 반복 확인.
 * 지연 시간 동안 CPU는 다른 작업을
 * 수행하지 못함.
 *
 * 단, 이 방식은 SysTick을 독점하므로
 * RTOS 또는 다른 SysTick 기반 코드와
 * 함께 사용하는 데 적합하지 않음.
 */

void delay_ms(uint32_t ms)
{
    /*
     * Reload 설정
     * CPU Clock = 16MHz
     * 16,000,000 / 1000 = 16,000
     * LOAD = 16,000 - 1 = 15,999
     * SysTick이 1ms마다 0에 도달.
     */

    SYSTICK_LOAD = (CPU_CLOCK_HZ / 1000U) - 1U;

    /*
     Current Value 초기화
     */

    SYSTICK_VAL = 0U;

    /*
     * SysTick 활성화
     *
     * CTRL Bit 2 = 1
     * → Processor Clock 사용
     *
     * CTRL Bit 1 = 0
     * → Interrupt Disable
     *
     * CTRL Bit 0 = 1
     * → SysTick Enable
     *
     * 0000 0101 = 0x05
     */

    SYSTICK_CTRL = (1U << 2) | (1U << 0);

    /*
     * COUNTFLAG Polling
     *
     * 1ms마다 COUNTFLAG가 설정됨.
     *
     * ms = 1000
     *
     * 1ms × 1000회 = 약 1초
     */

    for (uint32_t i = 0; i < ms; i++)
    {
        /*
         * CTRL Bit 16 확인
         *
         * 0 = 아직 카운트 완료 안 됨
         * 1 = 카운트 완료
         *
         * 카운트 완료까지 대기.
         */

        while ((SYSTICK_CTRL & (1U << 16)) == 0U)
        {
        }
    }

    /*
     * Current Value 초기화
     *
     * ENABLE = 0
     */

    SYSTICK_CTRL = 0U;
}

int main(void)
{
    // GPIOB Clock 활성
    RCC_AHB1ENR |= (1U << 1);

    /*
     PB7 Output Mode
     Output(01) 설정
     */
    GPIOB_MODER &= ~(3U << 14);
    GPIOB_MODER |= (1U << 14);

    // GPIOB_ODR Bit 7 = 0
    GPIOB_ODR &= ~(1U << 7);

    while (1)
    {
        // XOR 연산을 사용.
        // 계속 ON/OFF
        GPIOB_ODR ^= (1U << 7);
        // 1000ms = 약 1초
        delay_ms(1000);
    }
}
