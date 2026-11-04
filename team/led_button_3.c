// #include "stm32f446xx.h"

/*
 * STM32 GPIO 레지스터 제어 흐름
 *
 * 1. RCC_BASE 정의
 *    → RCC 주변장치 시작 주소
 *
 * 2. RCC_AHB1ENR 정의
 *    → GPIO 클럭 활성화 레지스터 주소
 *
 * 3. GPIOx_BASE 정의
 *    → GPIO 포트 시작 주소
 *
 * 4. GPIOx_MODER / IDR / ODR 정의
 *    → GPIO 모드 설정 / 입력 읽기 / 출력 제어 레지스터 주소
 *
 * 5. RCC 클럭 활성화
 *    → GPIOA, GPIOC 사용 가능
 *
 * 6. GPIO 모드 설정 (MODER)
 *    → PC13 : Input  (00)
 *    → PA5  : Output (01)
 *
 * 7. GPIO 입력 읽기 (IDR)
 *    → PC13 버튼 상태 확인
 *    → 누름 = LOW(0), 안 누름 = HIGH(1)
 *
 * 8. GPIO 출력 제어 (ODR)
 *    → PA5 HIGH(1) : LED ON
 *    → PA5 LOW(0)  : LED OFF
 *
 * 9. while(1) 반복
 *    → 버튼 상태 확인 → LED 제어 반복 (Polling)
 *
 * 핵심:
 * RCC → MODER → IDR → ODR
 */

/*
 * MCU 레지스터 기반 주변장치 제어 흐름
 *
 * 1. 주변장치 레지스터 주소 확인
 *    → 데이터시트 / Reference Manual 참고
 *
 * 2. 주변장치 클럭 활성화
 *    → 필요한 경우 클럭 공급
 *
 * 3. 핀 및 주변장치 설정
 *    → GPIO, UART, SPI, I2C, ADC 등
 *
 * 4. 동작 모드 설정
 *    → 입력 / 출력 / 통신 속도 / 인터럽트 등
 *
 * 5. 주변장치 활성화
 *    → 필요한 경우 Enable 비트 설정
 *
 * 6. 데이터 읽기 또는 쓰기
 *    → 입력값 확인 / 출력값 변경
 *
 * 7. 반복 실행 또는 인터럽트 처리
 *    → Polling / Interrupt / DMA
 *
 * 핵심:
 * 주소 확인 → 클럭 → 설정 → 활성화 → 데이터 처리
 */
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
    /*B1 USER: the user button is connected to the I/O PC13 (pin 2) of the STM32
 microcontroller */

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

/*
 * GPIO Push-Pull 출력 구조 (STM32)
 *
 *                  VDD (3.3V)
 *                      |
 *                  [ PMOS ]
 *                      |
 *                      +-------- GPIO PA5 ---- 외부 부하
 *                      |
 *                  [ NMOS ]
 *                      |
 *                     GND
 *
 * 1. ODR = 1 (HIGH 출력)
 *
 *    PMOS = ON
 *    NMOS = OFF
 *
 *    VDD → PMOS → GPIO PA5
 *
 *    GPIO 핀이 VDD 쪽으로 연결되어
 *    약 3.3V(HIGH)가 출력된다.
 *
 *
 * 2. ODR = 0 (LOW 출력)
 *
 *    PMOS = OFF
 *    NMOS = ON
 *
 *    GPIO PA5 → NMOS → GND
 *
 *    GPIO 핀이 GND 쪽으로 연결되어
 *    약 0V(LOW)가 출력된다.
 *
 *    NMOS에도 작은 ON 저항이 존재한다.
 *    단, 외부 전류 공급 경로가 없다면
 *    LOW 출력이라고 큰 전류가 흐르지는 않는다.
 *
 *
 * 3. PMOS와 NMOS가 동시에 ON이면?
 *
 *    VDD (3.3V)
 *        |
 *    PMOS ON
 *        |
 *    NMOS ON
 *        |
 *       GND
 *
 *    VDD와 GND 사이에 낮은 저항의 경로가 생겨
 *    과전류(Shoot-through)가 발생할 수 있다.
 *
 *
 * 4. STM32F446RE LD2 회로 (Active-High)
 *
 *    PA5 HIGH
 *        |
 *       LED
 *        |
 *      저항
 *        |
 *       GND
 *
 *    PA5 HIGH → LED ON
 *    PA5 LOW  → LED OFF
 *
 *
 * 핵심:
 *
 * ODR → Output Control → PMOS/NMOS → GPIO Pin
 *
 * PMOS ON → VDD 쪽으로 연결 → HIGH
 * NMOS ON → GND 쪽으로 연결 → LOW
 *
 * MOSFET의 ON/OFF는 스위치 상태이고,
 * GPIO의 HIGH/LOW는 핀의 논리 전압 상태이다.
 */
