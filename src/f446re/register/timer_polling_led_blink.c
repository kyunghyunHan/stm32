
#include <stdint.h>

/*
 * 프로젝트: timer_led
 * MCU: STM32F446RE
 * 보드: NUCLEO-F446RE
 *
 * 목표:
 * - TIM2 하드웨어 타이머를 사용한다.
 * - 1초마다 LD2(PA5)를 ON/OFF한다.
 * - 인터럽트 없이 Polling 방식으로 동작한다.
 *
 * 조건:
 * - TIM2 입력 클럭 = 16MHz
 * - APB1 Prescaler = 1
 *
 * 전체 동작:
 * RCC 클럭 활성화
 *       ↓
 * GPIOA 출력 설정
 *       ↓
 * TIM2 주기 설정
 *       ↓
 * TIM2 카운터 시작
 *       ↓
 * TIM2 업데이트 이벤트 발생
 *       ↓
 * CPU가 UIF 확인 (Polling)
 *       ↓
 * BSRR을 통해 PA5 출력 변경
 *       ↓
 * Push-Pull 출력 드라이버 동작
 *       ↓
 * LED ON/OFF
 */

// ==========================================
// 1. 주변장치 BASE 주소
// ==========================================

#define RCC_BASE    0x40023800UL
#define GPIOA_BASE  0x40020000UL
#define TIM2_BASE   0x40000000UL

/*
 * 각 주변장치는 메모리 맵에 주소가 할당되어 있다.
 * CPU는 해당 주소에 읽기/쓰기를 수행하여
 * 주변장치 레지스터에 접근한다.
 *
 * volatile:
 * 하드웨어 레지스터 접근을 컴파일러가
 * 임의로 제거하거나 생략하지 않도록 한다.
 */

// ==========================================
// 2. RCC 레지스터
// ==========================================

#define RCC_AHB1ENR \
    (*(volatile uint32_t *)(RCC_BASE + 0x30UL))

#define RCC_APB1ENR \
    (*(volatile uint32_t *)(RCC_BASE + 0x40UL))

// ==========================================
// 3. GPIOA 레지스터
// ==========================================

#define GPIOA_MODER \
    (*(volatile uint32_t *)(GPIOA_BASE + 0x00UL))

#define GPIOA_OTYPER \
    (*(volatile uint32_t *)(GPIOA_BASE + 0x04UL))

#define GPIOA_BSRR \
    (*(volatile uint32_t *)(GPIOA_BASE + 0x18UL))

// ==========================================
// 4. TIM2 레지스터
// ==========================================

#define TIM2_CR1 \
    (*(volatile uint32_t *)(TIM2_BASE + 0x00UL))

#define TIM2_SR \
    (*(volatile uint32_t *)(TIM2_BASE + 0x10UL))

#define TIM2_EGR \
    (*(volatile uint32_t *)(TIM2_BASE + 0x14UL))

#define TIM2_CNT \
    (*(volatile uint32_t *)(TIM2_BASE + 0x24UL))

#define TIM2_PSC \
    (*(volatile uint32_t *)(TIM2_BASE + 0x28UL))

#define TIM2_ARR \
    (*(volatile uint32_t *)(TIM2_BASE + 0x2CUL))

int main(void)
{
    // ======================================
    // 5. 주변장치 클럭 활성화
    // ======================================

    /*
     * RCC는 각 주변장치에 클럭을 공급하도록 제어한다.
     *
     * GPIOA → AHB1
     * TIM2  → APB1
     *
     * 클럭이 활성화되어야 주변장치가 정상 동작한다.
     */

    // GPIOA 클럭 활성화 (GPIOAEN = 1)
    RCC_AHB1ENR |= (1U << 0);

    // TIM2 클럭 활성화 (TIM2EN = 1)
    RCC_APB1ENR |= (1U << 0);

    // ======================================
    // 6. GPIOA PA5 출력 모드 설정
    // ======================================

    /*
     * GPIOA_MODER
     *
     * PA5는 [11:10] 두 비트를 사용한다.
     *
     * 00 = Input
     * 01 = Output
     * 10 = Alternate Function
     * 11 = Analog
     *
     * 먼저 00으로 클리어한 후
     * 01로 설정한다.
     */

    GPIOA_MODER &= ~(3U << 10);
    GPIOA_MODER |=  (1U << 10);

    /*
     * GPIOA_OTYPER
     *
     * PA5의 5번 비트로 출력 타입 결정
     *
     * 0 = Push-Pull
     * 1 = Open-Drain
     *
     * Push-Pull:
     * HIGH → 상단 PMOS ON
     * LOW  → 하단 NMOS ON
     */

    GPIOA_OTYPER &= ~(1U << 5);

    // ======================================
    // 7. LED 초기 상태 OFF
    // ======================================

    /*
     * GPIOA_BSRR은 ODR 비트를 변경한다.
     *
     * [15:0]  = Set
     * [31:16] = Reset
     *
     * PA5 HIGH → bit 5에 1 기록
     * PA5 LOW  → bit 21에 1 기록
     *
     * PA5 LOW:
     * 하단 NMOS가 핀을 GND 쪽으로 구동
     * → LED OFF
     */

    GPIOA_BSRR = (1U << 21);

    // ======================================
    // 8. TIM2 Prescaler 설정
    // ======================================

    /*
     * TIM2 입력 클럭 = 16MHz
     *
     * PSC는 카운터의 증가 속도를 결정한다.
     *
     * CNT 클럭 = TIM2 클럭 / (PSC + 1)
     *
     * 16,000,000 / 16,000 = 1,000Hz
     *
     * 따라서 CNT는 1ms마다 1씩 증가한다.
     */

    TIM2_PSC = 16000U - 1U;

    // ======================================
    // 9. TIM2 Auto-Reload 설정
    // ======================================

    /*
     * ARR은 카운트 주기를 결정한다.
     *
     * ARR = 999
     *
     * CNT:
     * 0 → 1 → 2 → ... → 999 → 0
     *
     * 총 1000번의 카운트
     *
     * 1ms × 1000 = 1초
     *
     * 카운트 주기가 끝나면
     * 업데이트 이벤트가 발생한다.
     */

    TIM2_ARR = 1000U - 1U;

    // ======================================
    // 10. TIM2 설정 반영
    // ======================================

    /*
     * EGR = Event Generation Register
     * UG  = Update Generation (bit 0)
     *
     * PSC에 기록한 값은 내부 분주기에
     * 즉시 적용되지 않는다.
     *
     * UG = 1을 기록하면 업데이트 이벤트가
     * 발생하여 PSC 설정이 반영된다.
     *
     * 이때 카운터도 재초기화된다.
     */

    TIM2_EGR = (1U << 0);

    // ======================================
    // 11. 업데이트 플래그 초기화
    // ======================================

    /*
     * SR = Status Register
     * UIF = Update Interrupt Flag (bit 0)
     *
     * UG로 발생시킨 초기 업데이트 이벤트가
     * UIF를 1로 설정할 수 있다.
     *
     * 실제 1초가 경과한 이벤트와 구별하기 위해
     * 타이머 시작 전에 UIF를 클리어한다.
     *
     * UIF는 0을 기록하여 클리어한다.
     */

    TIM2_SR = ~(1U << 0);

    // ======================================
    // 12. TIM2 카운터 시작
    // ======================================

    /*
     * CR1 = Control Register 1
     * CEN = Counter Enable (bit 0)
     *
     * CEN = 1 → 타이머 시작
     * CEN = 0 → 타이머 정지
     *
     * 이제 TIM2 하드웨어가
     * CPU와 별도로 카운트를 진행한다.
     */

    TIM2_CR1 |= (1U << 0);

    // 소프트웨어 LED 상태 변수
    uint32_t led_on = 0U;

    // ======================================
    // 13. 메인 루프 (Polling)
    // ======================================

    while (1)
    {
        /*
         * CPU가 TIM2_SR의 UIF를 계속 확인한다.
         *
         * UIF = 0 → 아직 이벤트 없음
         * UIF = 1 → 업데이트 이벤트 발생
         *
         * 인터럽트가 아니라 Polling 방식이다.
         */

        if (TIM2_SR & (1U << 0))
        {
            /*
             * 처리한 이벤트의 UIF를 클리어한다.
             * 다음 업데이트 이벤트를 확인하기 위함.
             */

            TIM2_SR = ~(1U << 0);

            /*
             * led_on은 소프트웨어 변수이며
             * 실제 GPIO 레지스터는 아니다.
             *
             * 0 = LED OFF 상태
             * 1 = LED ON 상태
             */

            if (led_on)
            {
                /*
                 * PA5 LOW
                 *
                 * BSRR bit 21 기록
                 * → ODR bit 5 = 0
                 * → Output Control
                 * → Push-Pull 하단 NMOS ON
                 * → PA5 LOW
                 * → LED OFF
                 */

                GPIOA_BSRR = (1U << 21);
                led_on = 0U;
            }
            else
            {
                /*
                 * PA5 HIGH
                 *
                 * BSRR bit 5 기록
                 * → ODR bit 5 = 1
                 * → Output Control
                 * → Push-Pull 상단 PMOS ON
                 * → PA5 HIGH
                 * → LED ON
                 */

                GPIOA_BSRR = (1U << 5);
                led_on = 1U;
            }
        }
    }
}
