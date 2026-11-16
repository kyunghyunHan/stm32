// PLL로 시스템 클록을 216MHz로 설정하고 캐시를 초기화하는 코드

void Initialize_MCU(void)
{
    /* 1. Cortex-M7 캐시 활성화 */

SCB_EnableICache(); // Instruction Cache: 명령어 접근 속도 개선
SCB_EnableDCache(); // Data Cache: 데이터 접근 속도 개선

    /* 2. Flash 접근 설정 */
    // Flash 접근 지연: 7 wait states
    // ART 가속기 및 프리페치 활성화
    FLASH->ACR = 0x00000307;

    /* 3. HSI로 임시 전환 후 PLL 설정 */

    // 내부 클록 HSI와 외부 클록 HSE 활성화
    RCC->CR |= 0x00010001;

    // HSI가 안정화될 때까지 대기: HSIRDY = 1
    while ((RCC->CR & 0x00000002) == 0)
        ;

    // PLL을 재설정하기 전에 시스템 클록을 HSI로 전환
    RCC->CFGR = 0x00000000;

    // 시스템 클록이 HSI로 전환될 때까지 대기: SWS = 00
    while ((RCC->CFGR & 0x0000000C) != 0)
        ;

    // PLL 비활성화, HSE 및 HSI 활성화 유지
    RCC->CR = 0x00010001;

    RCC->PLLCFGR = 0x09403608;

    // PLL 활성화, HSE 및 HSI 활성화 유지
    RCC->CR = 0x01010001;

    // PLL이 안정화될 때까지 대기: PLLRDY = 1
    while ((RCC->CR & 0x02000000) == 0)
        ;

    /* 4. 216MHz 동작을 위한 오버드라이브 활성화 */

    // 전원 제어 모듈(PWR)에 클록 공급
    RCC->APB1ENR |= 0x10000000;

    // 내부 전압 레귤레이터의 오버드라이브 활성화: ODEN = 1
    PWR->CR1 |= 0x00010000;

    // 오버드라이브 준비 완료까지 대기: ODRDY = 1
    while ((PWR->CSR1 & 0x00010000) == 0)
        ;

    // 오버드라이브 모드로 전환: ODSWEN = 1
    PWR->CR1 |= 0x00020000;

    // 오버드라이브 전환 완료까지 대기: ODSWRDY = 1
    while ((PWR->CSR1 & 0x00020000) == 0)
        ;

    /* 5. 시스템 클록 및 주변장치 클록 설정 */

    // SYSCLK 소스: PLL
    // AHB : SYSCLK / 1 = 216MHz
    // APB1: AHB / 4    =  54MHz
    // APB2: AHB / 4    =  54MHz
    RCC->CFGR = 0x3040B402;

    // TIMPRE = 1: 현재 버스 분주 설정에서 타이머 클록을 216MHz로 설정
    RCC->DCKCFGR1 = 0x01000000;

    // 시스템 클록이 PLL로 전환될 때까지 대기: SWS = 10
    while ((RCC->CFGR & 0x0000000C) != 0x00000008)
        ;

    // 외부 클록 HSE의 이상을 감지하는 클록 보안 시스템(CSS) 활성화
    RCC->CR |= 0x00080000;

    /* 6. I/O 보상 회로 활성화 */

    // 시스템 설정 모듈(SYSCFG)에 클록 공급
    RCC->APB2ENR |= 0x00004000;

    // 고속 I/O 동작을 위한 보상 회로 활성화
    SYSCFG->CMPCR = 0x00000001;
}