/*
 * STM32F767VGT6 시스템 초기화
 *
 * 외부 클록 HSE 16MHz를 PLL로 증폭하여 시스템 클록을 216MHz로 설정한다.
 * 고속 동작에 필요한 캐시, Flash 접근 지연, 오버드라이브를 설정하고,
 * 주변장치 버스 클록과 I/O 보상 회로를 초기화한다.
 *
 * 초기화 순서
 * 1. 명령어 캐시 및 데이터 캐시 활성화
 * 2. Flash 접근 지연 및 가속 기능 설정
 * 3. HSI로 임시 전환한 뒤 PLL 설정
 * 4. 216MHz 동작을 위한 오버드라이브 활성화
 * 5. 시스템 및 주변장치 클록 설정
 * 6. I/O 보상 회로 활성화
 *
 * 최종 클록 설정 — HSE가 16MHz인 경우
 * - SYSCLK : 216MHz
 * - AHB    : 216MHz
 * - APB1   :  54MHz
 * - APB2   :  54MHz
 * - PLL48CK:  48MHz
 *
 * 주의: 이 함수는 클록 관련 레지스터를 직접 설정하므로,
 *       보드의 HSE 주파수와 전원 조건에 맞게 사용해야 한다.
 */
void Initialize_MCU(void)
{
    /* 1. Cortex-M7 캐시 활성화 */
    SCB_EnableICache(); // 명령어 캐시: 명령어를 빠르게 가져오기
    SCB_EnableDCache(); // 데이터 캐시: 데이터를 빠르게 읽고 쓰기

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

    /*
     * PLL 입력 클록: HSE 16MHz
     *
     * PLLM = 8   : 16MHz / 8   =   2MHz
     * PLLN = 216 :  2MHz × 216 = 432MHz (VCO 출력)
     * PLLP = 2   : 432MHz / 2  = 216MHz (시스템 클록용)
     * PLLQ = 9   : 432MHz / 9  =  48MHz (PLL48CK)
     */
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