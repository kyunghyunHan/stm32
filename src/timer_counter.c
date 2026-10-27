/*  ‣ 목표 : TIM1 을 1 초 주기로 오버플로우시켜 PD3(LED)을 토글한다.
 *  ‣ 시스템 클록 : 216 MHz (Initialize_MCU()로 이미 설정됨)
 *  ‣ TIM1 클록   : 216 MHz  (RCC_DCKCFGR1.TIMPRE = 1 → HCLK 그대로 사용)
 *                  └ prescaler = 21 600-1  → 10 kHz 카운터
 *                    auto-reload = 10 000-1 → 1 s 주기
 */
#include "stm32f7xx.h"

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
/* ───────────────────────── 타이머 1 오버플로우 인터럽트 ───────────────────────── */
void TIM1_UP_TIM10_IRQHandler(void)
{
    if (TIM1->SR & 0x0001)                // UIF(Update Interrupt Flag) 확인
    {
        TIM1->SR &= ~0x0001;              // UIF 클리어
        GPIOC->ODR ^= 0x00000040;  // LED 토글
    }
}

/* ─────────────────────────────────────────────────────────────────────────────── */
int main(void)
{
/* 0) 시스템 클록·캐시 초기화 (216 MHz) */
    Initialize_MCU();


/* 1) GPIOD,GPIOC 클록 활성화  (RCC_AHB1ENR bit3 = 1) */
    RCC->AHB1ENR |= 0x00000008;
    RCC->AHB1ENR |= 0x00000004;    // GPIOCEN

 //2) PC6을 General-purpose output 모드로 설정 (MODER6[1:0] = 01)
    GPIOC->MODER  &= ~0x00003000;  // clear bits 13:12
    GPIOC->MODER  |=  0x00001000;  // set bit 12
     //PC6 중간 속도 설정 (OSPEEDR6[1:0] = 01)
    GPIOC->OSPEEDR &= ~0x00003000;  // clear bits 13:12
    GPIOC->OSPEEDR |=  0x00001000;  // set bit 12

/* 3) TIM1 클록 활성화 (RCC_APB2ENR bit0 = TIM1EN) */
    RCC->APB2ENR |= 0x00000001;

/* 4) 타이머 기본 설정 ---------------------------------------------------------
 *    PSC  = 21 600-1   → 216 MHz / 21 600 = 10 kHz
 *    ARR  = 10 000-1   → 10 kHz / 10 000 = 1 Hz (1 초)
 * ---------------------------------------------------------------------------*/
    TIM1->PSC = 21600 - 1;                // Prescaler
    TIM1->ARR = 10000 - 1;                // Auto-reload
    TIM1->EGR = 0x0001;                   // UG = 1 (즉시 로드)

/* 5) 인터럽트 허용 : DIER.UIE = 1, NVIC Enable */
    TIM1->DIER |= 0x0001;                 // Update interrupt enable
    NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);   // CMSIS 함수

/* 6) 타이머 카운터 시작 : CR1.CEN = 1 */
    TIM1->CR1 |= 0x0001;

/* 7) 메인 루프 – CPU는 슬립 / 다른 작업 수행 가능 */
    while (1)
    {

    }
}
