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
    GPIOA_MODER &= ~(0x3U << 10);
    GPIOA_MODER |= (0x1U << 10);

    volatile uint32_t button = 0;

    while (1)
    { // PC 13에 연결되어있고 IDR은 핀 하나당 1비트
        button = (GPIOC_IDR >> 13) & 1U;
        //버튼을 눌럿을떄
        //PC13이 GND에 연결되어 LOW(0)
        if (button == 0U)
        {
            GPIOA_ODR |= (1U << 5);
        }
        else
        {
            GPIOA_ODR &= ~(1U << 5);
        }
    }
}