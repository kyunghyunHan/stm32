
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

void UART2_Init(void)
{
    // 1. USART2 및 관련 GPIO 클럭 활성화
    //    USART2는 APB1 버스에 연결되어 있음
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;   // USART2 클럭 활성화

    // 2. GPIOD 클럭 활성화 (PD5, PD6 사용)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;    // GPIOD 클럭 활성화

    // 3. PD5 (TX), PD6 (RX)를 Alternate Function (AF7) 모드로 설정
    GPIOD->MODER |=  ((2U << (5 * 2)) | (2U << (6 * 2))); // AF 모드(10b) 설정

    // 4. PD5, PD6에 대한 AF 설정 (AF7)
    // PD5 → AFR[0]의 비트 [23:20], PD6 → AFR[0]의 비트 [27:24]
    GPIOD->AFR[0] |=  ((0x7U << (5 * 4)) | (0x7U << (6 * 4)));

    // 5. USART2 설정: 비동기(8N1, 115200bps), TX/RX 활성
    USART2->CR1 = 0;    // 초기화 (비활성화)
    USART2->CR2 = 0;
    USART2->CR3 = 0;
    // 예: APB1 클럭이 54MHz일 경우, BRR = 54MHz/115200 ≈ 468.75, 469로 설정
    // 예: APB1 클럭이 54MHz일 경우, BRR = 54MHz/9600 ≈ 5625, 469로 설정
//    USART2->BRR = 469;
    USART2->BRR = 5625;
    // TE=1, RE=1, UE=1 → TX, RX 활성 및 USART2 활성화
    USART2->CR1 |= (USART_CR1_TE | USART_CR1_RE | USART_CR1_UE);
}
void UART2_SendChar(char c)
{
    // TXE (송신 데이터 레지스터 비어있음) 플래그 대기
    while (!(USART2->ISR & USART_ISR_TXE));
    USART2->TDR = (uint8_t)c;
}
char UART2_ReceiveChar(void)
{
    // RXNE (수신 데이터 레지스터에 데이터 있음) 플래그 대기
    while (!(USART2->ISR & USART_ISR_RXNE));
    return (char)(USART2->RDR & 0xFF);
}
/* ─────────────────────────────────────────────────────────────────────────────── */
uint8_t received_char = 0;
int main(void)
{
/* 0) 시스템 클록·캐시 초기화 (216 MHz) */
    Initialize_MCU();
    UART2_Init();
    while (1)
    {
        received_char = UART2_ReceiveChar();

        /* 4) 수신된 문자 에코 (그대로 다시 전송) */
        UART2_SendChar(received_char+1);

    }
}
