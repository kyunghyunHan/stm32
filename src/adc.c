#include "stm32f7xx.h"

void Initialize_MCU(void);            // 사용자가 제공한 클록 초기화 함수

#define VREF        3.3f              // +A3.3 V 기준 전압
#define ADC_RES     4095.0f           // 12-bit ADC 최대값
volatile uint16_t adc_raw = 0;        // ADC 원시값
volatile float    throttle_V = 0.0f;  // 전압 환산값

void Initialize_MCU(void) /* initialize STM32F767VGT6 MCU */
{
    // (1) 명령 캐시 및 데이터 캐시 설정
    SCB_EnableICache(); // enable L1 instruction cache
    SCB_EnableDCache(); // enable L1 data cache

    // (2) ART 가속기, 프리페치 버퍼, 웨이트 사이클 설정
    FLASH->ACR = 0x00000307; // 7 waits, enable ART accelerator and prefetch

    // (3) HSE 및 PLL 설정(시스템 클록 SYSCLK = 216MHz)
    RCC->CR |= 0x00010001; // HSE on, HSI on
    while((RCC->CR & 0x00000002) == 0); // wait until HSIRDY = 1
    RCC->CFGR = 0x00000000; // SYSCLK = HSI
    while((RCC->CFGR & 0x0000000C) != 0); // wait until SYSCLK = HSI

    RCC->CR = 0x00010001; // PLL off, HSE on, HSI on
    RCC->PLLCFGR = 0x09403608; // SYSCLK = HSE*PLLN/PLLM/PLLP = 16MHz*216/8/2 = 216MHz
                               // PLL48CK = HSE*PLLN/PLLM/PLLQ = 16MHz*216/8/9 = 48MHz
    RCC->CR = 0x01010001; // PLL on, HSE on, HSI on
    while((RCC->CR & 0x02000000) == 0); // wait until PLLRDY = 1

    // (4) 오버드라이브 설정
    RCC->APB1ENR |= 0x10000000; // 전원모듈 클록(PWREN = 1)
    PWR->CR1 |= 0x00010000; // over-drive enable(ODEN = 1)
    while((PWR->CSR1 & 0x00010000) == 0); // ODRDY = 1 ?
    PWR->CR1 |= 0x00020000; // over-drive switching enable(ODSWEN = 1)
    while((PWR->CSR1 & 0x00020000) == 0); // ODSRDY = 1 ?

    // (5) 주변장치 클록 설정(APB1CLK = APB2CLK = 54MHz)
    RCC->CFGR = 0x3040B402; // SYSCLK = PLL, AHB = 216MHz, APB1 = APB2 = 54MHz
    RCC->DCKCFGR1 = 0x01000000; // TIMxCLK = 216MHz
    while((RCC->CFGR & 0x0000000C) != 0x00000008); // wait until SYSCLK = PLL
    RCC->CR |= 0x00080000; // CSS on

    // (6) I/O 보상 설정
    RCC->APB2ENR |= 0x00004000; // 주변장치 클록(SYSCFG = 1)
    SYSCFG->CMPCR = 0x00000001; // enable compensation cell
}
int main(void)
{
/* ------------------------------------------------------------------
 * 0) 시스템 클록 및 캐시 초기화 (216 MHz) – 사용자 함수
 * -----------------------------------------------------------------*/
    Initialize_MCU();

/* ------------------------------------------------------------------
 * 1) GPIO 클럭 활성화 (GPIOA)
 *    RCC_AHB1ENR 비트 구조:
 *        [0] GPIOAEN
 * -----------------------------------------------------------------*/
    RCC->AHB1ENR |= 0x00000001;       // 0000 0001 → A ON

/* ------------------------------------------------------------------
 * 3) PA7 = 아날로그 입력 (ADC1_IN7)
 *    MODER7[15:14] = 11 (Analog)
 * //입력모드
 * -----------------------------------------------------------------*/
    GPIOA->MODER |=  0x0000C000;      // 11
/* ------------------------------------------------------------------
 * 4) ADC1 클럭 & 공통 프리스케일
 *    - APB2(PCLK2)= 54 MHz (Initialize_MCU 설정)
 *    - ADC->CCR[17:16] = 10 → PCLK2 / 6 = 9 MHz  (데이터시트 최대 36 MHz 이하)
 * -----------------------------------------------------------------*/
    RCC->APB2ENR |= 0x00000100;       // ADC1EN = 1
    ADC->CCR     |=  0x00020000;      // 0000 0000 0000 0010 0000 0000 0000 0000

/* ------------------------------------------------------------------
 * 5) ADC1 단일-변환 설정
 *    CR2 : ADON=1, CONT=0 (Single), ALIGN=0 (Right)
 *    SMPR2: 채널7 샘플타임 [23:21] = 011 → 56 cycles ≒ 6.2 µs @ 9 MHz
 *    SQR1: L=0 (총 1 변환)  /  SQR3: SQ1 = 7
 * -----------------------------------------------------------------*/
    ADC1->CR2   = 0x00000001;         // ADON
    ADC1->SMPR2 |= (0x03 << 21);      // 011
    ADC1->SQR3  |=  0x00000007;       // 채널 7

/* --------------------------- 메인 루프 -------------------------- */
    while (1)
    {
        /* -------- 쓰로틀 ADC 단일 변환 -------- */
        ADC1->CR2 |= 0x40000000;                  // SWSTART (bit 30)

        while ((ADC1->SR & 0x00000002) == 0);     // EOC 대기

        adc_raw    = ADC1->DR;                    // 데이터 읽기(EOC 클리어)
        throttle_V = (adc_raw * VREF) / ADC_RES;  // 전압 환산
    }
}
