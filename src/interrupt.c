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

/* ───── GPIO & EXTI 초기화 ───── */
void GPIO_EXTI_Init(void)
{
    /* 1) GPIOD 클록 ON (bit3) */
    RCC->AHB1ENR |= 0x00000008;
    RCC->AHB1ENR |= 0x00000004;    // GPIOCEN

    // 2) PC6을 General-purpose output 모드로 설정 (MODER6[1:0] = 01)
    GPIOC->MODER  &= ~0x00003000;  // clear bits 13:12
    GPIOC->MODER  |=  0x00001000;  // set bit 12

    // 3) PC6 중간 속도 설정 (OSPEEDR6[1:0] = 01)
    GPIOC->OSPEEDR &= ~0x00003000;  // clear bits 13:12
    GPIOC->OSPEEDR |=  0x00001000;  // set bit 12

    /* ── PD4 : 스위치 입력 / EXTI4 ───────────────────── */
    GPIOD->MODER &= ~0x00000300;   // bits9:8 = 00 (Input)

    /* 4) SYSCFG 클록 ON (EXTI 라우팅용) */
    RCC->APB2ENR |= 0x00004000;    // SYSCFGEN

    /* 35) EXTI4 소스 = Port D */
    SYSCFG->EXTICR[1] &= ~0x000F;  // Clear EXTI4 bits
    SYSCFG->EXTICR[1] |= 0x0003;   // 0x3 = Port D

    /* 6) EXTI4 Falling-edge 인터럽트 설정 */
    EXTI->IMR |= 0x00000010;       // Interrupt request unmask
    EXTI->FTSR |= 0x00000010;      // Falling edge on
    EXTI->PR = 0x00000010;         // Pending bit clear

    /* 7) NVIC: EXTI4 IRQ Enable */
    NVIC_EnableIRQ(EXTI4_IRQn);
}

/* ───── EXTI4 인터럽트 서비스 루틴 (개선 필요) ───── */
// 문제: 인터럽트 핸들러 내에서 긴 지연은 좋지 않음
void EXTI4_IRQHandler(void)
{
    if (EXTI->PR & 0x00000010)     // 펜딩 확인
    {
        EXTI->PR = 0x00000010;     // 펜딩 클리어
        // 토글 방식으로 개선
        GPIOC->ODR ^= 0x00000040;  // LED 토글
    }
}

/* ───── main ───── */
int main(void)
{
    Initialize_MCU();  // 216 MHz 클록 설정
    GPIO_EXTI_Init();  // GPIO 및 EXTI 설정

    while (1)
    {
        /* 메인 루프는 대기만; 모든 일은 인터럽트에서 처리 */
    }
}
