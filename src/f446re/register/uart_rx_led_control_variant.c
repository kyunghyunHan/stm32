
#include <stdint.h>

/*
 * STM32F446RE UART 문자로 LED 제어
 *
 * '1' 수신 → LED ON
 * '0' 수신 → LED OFF
 * 다른 문자 → LED 상태 유지
 *
 * MCU       : STM32F446RE
 * LED       : PA5 (HIGH = ON, LOW = OFF)
 * USART2 TX : PA2 (AF7)
 * USART2 RX : PA3 (AF7)
 * Clock     : HSI = 16MHz, APB1 = 16MHz 가정
 * Baudrate  : 9600bps, 8N1
 *
 * HAL 없이 레지스터 직접 접근
 *
 * 참고:
 * RM0390 - STM32F446 Reference Manual
 * UM1724 - NUCLEO-64 Board User Manual
 *
 *
Moder

00	Input
01	Output
10	Alternate Function
11	Analog
 */

// 1. RCC_BASE 주소 정의
//전처리 과정에서서 치환
#define RCC_BASE 0x40023800UL

// 2. GPIOA_BASE 주소 정의
// GPIOA는 LED(PA5), USART2(PA2/PA3)에 사용
#define GPIOA_BASE 0x40020000UL

// 3. GPIOD_BASE 주소 정의
// 참고용: 현재 실습에서는 사용하지 않음
#define GPIOD_BASE 0x40020C00UL

// 4. USART2_BASE 주소 정의
#define USART2_BASE 0x40004400UL

// ==========================================
// 2. RCC 레지스터 정의
// ==========================================

// RCC_AHB1ENR 레지스터 정의
// Offset : 0x30
// GPIOA 클럭 활성화에 사용
#define RCC_AHB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x30))

// RCC_APB1ENR 레지스터 정의
// Offset : 0x40
// USART2 클럭 활성화에 사용
#define RCC_APB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x40))
// ==========================================
// GPIO 레지스터 정의
// ==========================================

// GPIOA_MODER 레지스터 정의
//  Offset : 0x00
//  PA2, PA3, PA5 모드 설정
#define GPIOA_MODER (*(volatile uint32_t *)(GPIOA_BASE + 0x00))
// GPIOA_ODR 레지스터 정의
// Offset : 0x14
// PA5 LED 출력 제어
#define GPIOA_ODR (*(volatile uint32_t *)(GPIOA_BASE + 0x14))
// GPIOA_AFRL 레지스터 정의
// PA2, PA3을 AF7로 설정
#define GPIOA_AFRL (*(volatile uint32_t *)(GPIOA_BASE + 0x20))
// ==========================================
// USART2 레지스터 정의
// STM32F446RE 기준 (RM0390)
// ==========================================

// USART2_SR 레지스터 정의
// Offset : 0x00
// TXE(bit 7), RXNE(bit 5) 확인
#define USART2_SR (*(volatile uint32_t *)(USART2_BASE + 0x00))
// USART2_DR 레지스터 정의
//  Offset : 0x04
//  수신 데이터 읽기 / 송신 데이터 쓰기
#define USART2_DR (*(volatile uint32_t *)(USART2_BASE + 0x04))

// USART2_BRR 레지스터 정의
//  Offset : 0x08
//  Baudrate 설정
#define USART2_BRR (*(volatile uint32_t *)(USART2_BASE + 0x08))

// USART2_CR1 레지스터 정의
//  Offset : 0x0C
//  UE(bit 13), TE(bit 3), RE(bit 2)
#define USART2_CR1 (*(volatile uint32_t *)(USART2_BASE + 0x0C))

// 14. USART2_CR2 레지스터 정의
// Offset : 0x10
// Stop bit 설정 등
#define USART2_CR2 (*(volatile uint32_t *)(USART2_BASE + 0x10))

// USART2_CR3 레지스터 정의
//  Offset : 0x14
//  DMA, 하드웨어 흐름 제어 등
#define USART2_CR3 (*(volatile uint32_t *)(USART2_BASE + 0x14))

// ==========================================
// 5. USART2 초기화 함수
// ==========================================

void UART2_Init(void)
{
    // GPIOA 클럭 활성화
    // RCC_AHB1ENR bit 0
    //Flash .text 영역에 저장
    //RCC레지스터 값 읽기
    //0번 비트를 1로 설정
    //변경된 값 쓰기
    //하드웨어 레지스터에 저장
    RCC_AHB1ENR |= (1U << 0);
    // USART2 클럭 활성화
    // RCC_APB1ENR bit 17
    RCC_APB1ENR |= (1U << 17);
    // PA5를 Output 모드(01)로 설정
    // MODER에서 PA5 = bit 11:10
    GPIOA_MODER &= ~(3U << 10);
    GPIOA_MODER |= (1U << 10);
    // LED 초기 상태 OFF
    // PA5 LOW(0) → LED OFF
    GPIOA_ODR &= ~(1U << 5);
    // 20. PA2, PA3을 Alternate Function 모드(10)로 설정
    // PA2 = MODER bit 5:4
    // PA3 = MODER bit 7:6
    GPIOA_MODER &= ~((3U << 4) | (3U << 6));
    GPIOA_MODER |= ((2U << 4) | (2U << 6));
    // PA2, PA3을 AF7로 설정
    // AFRL은 핀당 4비트
    // PA2 = bit 11:8
    // PA3 = bit 15:12
    GPIOA_AFRL &= ~((0xFU << 8) | (0xFU << 12));
    GPIOA_AFRL |= ((0x7U << 8) | (0x7U << 12));
    // USART2 비활성화 및 CR1, CR2, CR3 초기화
    USART2_CR1 = 0U;
    USART2_CR2 = 0U;
    USART2_CR3 = 0U;
    // 23. Baudrate를 9600bps로 설정
    // APB1 = 16MHz, OVER8 = 0
    // BRR = 16000000 / 9600 ≈ 1667
    USART2_BRR = 1667U;
    // USART2 TX 활성화
    // CR1 TE = bit 3
    USART2_CR1 |= (1U << 3);
    // 25. USART2 RX 활성화
    // CR1 RE = bit 2
    USART2_CR1 |= (1U << 2);
    // 26. USART2 활성화
    // CR1 UE = bit 13
    USART2_CR1 |= (1U << 13);
}

// ==========================================
// 6. UART 문자 수신 함수
// ==========================================

char UART2_ReceiveChar(void)
{
    // 27. RXNE 플래그가 1이 될 때까지 대기
    // SR bit 5
    // Polling 방식
    //.text
    while (USART2_SR & ((1U << 5) == 0U))
        ;
    return (char)(USART2_DR & (0xFFU));
}

// ==========================================
// 7. 메인 함수
// ==========================================

int main(void)
{  
    //stack
    char received_char = 0;

    UART2_Init();

    while (1)
    {
        // 30. UART로 문자 1개 수신
        char receiver_char = UART2_ReceiveChar();
        // 31. 수신 문자가 '1'이면

        if (receiver_char == '1')
        {
            GPIOA_ODR |= (1U << 5);
        }
        if (receiver_char == '0')
        {
            GPIOA_ODR &= ~(1U << 5);
        }
    }
}


/*
fetch:PC(Program Counter)에 저장된 명령어 주소를 Address Bus를 통해 메모리에 전달하고, 해당 주소에 있는 명령어를 읽어 IR(Instruction Register)에 저장한다.
decode:

*/

/*
===============================================================================
 CPU 명령어 실행 과정 (Fetch → Decode → Execute)
===============================================================================

 대상 MCU : STM32F446RE
 CPU      : ARM Cortex-M4
 구조     : Harvard 구조의 특징을 가진 프로세서
 목적     : CPU가 명령어를 가져오고, 해독하고, 실행하는 과정 이해


===============================================================================
 1. CPU 주요 구성 요소
===============================================================================

 [PC - Program Counter]
 - CPU 내부의 프로그램 카운터 (R15)
 - 실행할 명령어의 주소를 관리한다.
 - 명령어 자체가 아니라 명령어 주소를 다룬다.
 - 분기 명령어에 따라 실행 흐름이 변경된다.

 [IR - Instruction Register]
 - 메모리에서 가져온 명령어를 보관하는 CPU 내부 요소
 - 명령어 해독에 사용된다.
 - 프로그래머가 직접 접근하는 R0~R15 레지스터와는 다르다.

 [CU - Control Unit]
 - 명령어를 해독하고 CPU 동작을 제어한다.
 - 연산 종류, 레지스터 선택, 메모리 읽기/쓰기 등을 제어한다.
 - CU 자체는 데이터를 저장하는 일반 레지스터가 아니다.

 [ALU - Arithmetic Logic Unit]
 - 산술 연산과 논리 연산을 수행한다.
 - ADD, SUB, AND, ORR, EOR 등의 연산을 처리한다.
 - 연산 결과를 CPU 레지스터 등에 반영한다.

 [CPU Register]
 - R0~R12 : 범용 레지스터
 - R13    : SP (Stack Pointer)
 - R14    : LR (Link Register)
 - R15    : PC (Program Counter)
 - xPSR   : 프로그램 상태 관련 레지스터

 - CPU 내부에서 주소, 연산값, 결과 등을 임시로 보관한다.


===============================================================================
 2. 버스(Bus)의 종류
===============================================================================

 [Address Bus - 주소 버스]
 - CPU가 접근할 메모리 또는 주변장치의 주소를 전달한다.
 - 예: 0x08000100 (Flash)
 - 예: 0x40020000 (GPIOA_MODER)

 [Data Bus - 데이터 버스]
 - CPU와 메모리 또는 주변장치 사이에서 데이터를 전달한다.
 - 명령어 인출 시 기계어가 전달되는 경로도 존재한다.
 - 읽기(READ)와 쓰기(WRITE)에 사용된다.

 [Control Bus - 제어 신호]
 - 메모리 읽기/쓰기 등의 동작을 제어한다.
 - READ/WRITE 여부 및 전송 관련 제어 정보를 전달한다.

 ※ Address Bus, Data Bus, Control Bus는 개념적인 분류이다.
 ※ 실제 Cortex-M4는 AHB-Lite 등의 버스 인터페이스를 사용한다.
 ※ CPU 내부 레지스터 간 연산은 외부 메모리 버스 접근 없이 가능하다.


===============================================================================
 3. FETCH (명령어 인출)
===============================================================================

 [1] PC (Program Counter)

 - CPU가 실행할 명령어의 주소를 관리한다.

   예:
   PC = 0x08000100

          |
          v

 [2] Address Bus

 - PC가 가리키는 명령어 주소를 메모리 시스템에 전달한다.

   Address = 0x08000100

          |
          v

 [3] Control Signal

 - 메모리 시스템에 읽기 요청을 전달한다.

   READ

          |
          v

 [4] Flash Memory (.text)

 - 해당 주소에 저장된 기계어를 읽는다.
 - 일반적인 STM32 Bare-Metal 프로젝트에서
   실행 명령어는 Flash의 .text 영역에 배치된다.

          |
          v

 [5] Instruction Bus / Read Data Path

 - Flash에서 읽은 기계어를 CPU 내부로 전달한다.

          |
          v

 [6] IR (Instruction Register)

 - 가져온 명령어를 CPU 내부에 보관한다.
 - 명령어를 Decode 단계로 전달한다.


===============================================================================
 4. DECODE (명령어 해독)
===============================================================================

 [7] CU (Control Unit)

 - IR에 있는 명령어를 해독한다.
 - 어떤 연산을 수행할지 결정한다.
 - 사용할 CPU 레지스터를 확인한다.
 - 실행에 필요한 제어 신호를 생성한다.

 예:

   ADD R0, R1, R2

   ADD : 덧셈 명령어
   R0  : 결과를 기록할 레지스터
   R1  : 첫 번째 피연산자
   R2  : 두 번째 피연산자

          |
          v

 [8] Register File (CPU 내부 레지스터)

   R1 = 10
   R2 = 20

 - CPU 내부 레지스터에서 필요한 값을 준비한다.
 - 레지스터 값은 CPU 내부 데이터 경로를 통해 전달된다.

 ※ 일반적인 레지스터 간 연산의 피연산자를 준비할 때는
   외부 메모리 Address Bus / Data Bus 접근이 필요하지 않다.


===============================================================================
 5. EXECUTE (명령어 실행)
===============================================================================

 명령어 종류에 따라 실행 방법이 달라진다.


 -------------------------------------------------------------------------------
 A. ALU 연산 (ADD, SUB, AND, ORR 등)
 -------------------------------------------------------------------------------

 예:

   ADD R0, R1, R2

   R1 = 10
   R2 = 20

          |
          v

   ALU

   10 + 20 = 30

          |
          v

   R0 = 30

 - CPU 내부에서 연산한다.
 - 결과를 CPU 레지스터에 기록한다.
 - 명령어에 따라 APSR 상태 플래그가 갱신될 수 있다.
 - ALU 연산 자체에 외부 메모리 버스 접근은 필요하지 않다.


 -------------------------------------------------------------------------------
 B. 메모리 읽기 (LDR)
 -------------------------------------------------------------------------------

 예:

   LDR R1, [R0]

   R0 = 0x40020000

          |
          v

 [1] Address Bus

   Address = 0x40020000

 - 읽을 대상의 주소를 전달한다.

          |
          v

 [2] Control Signal

   READ

 - 메모리 또는 주변장치 읽기를 요청한다.

          |
          v

 [3] GPIOA_MODER (하드웨어 레지스터)

 - GPIOA_MODER의 현재 값을 읽는다.

          |
          v

 [4] Read Data Path

 - 읽은 값을 CPU로 전달한다.

          |
          v

 [5] CPU Register

   R1 = GPIOA_MODER에서 읽은 값

 - 읽은 데이터가 R1에 기록된다.


 -------------------------------------------------------------------------------
 C. 메모리 쓰기 (STR)
 -------------------------------------------------------------------------------

 예:

   STR R1, [R0]

   R0 = 0x40020000
   R1 = 0x00000400

          |
          v

 [1] Address Bus

   Address = 0x40020000

 - 값을 기록할 대상 주소를 전달한다.

          |
          v

 [2] Write Data Path

   Data = 0x00000400

 - 기록할 데이터를 전달한다.

          |
          v

 [3] Control Signal

   WRITE

 - 메모리 또는 주변장치 쓰기를 요청한다.

          |
          v

 [4] GPIOA_MODER

 - 전달된 값이 하드웨어 레지스터에 기록된다.

 ※ STR 명령어는 레지스터 값을 메모리 또는
   메모리 매핑된 주변장치에 기록하는 명령어이다.


 -------------------------------------------------------------------------------
 D. 분기 명령어 (B, BL 등)
 -------------------------------------------------------------------------------

 예:

   B loop

 - 지정한 주소로 실행 흐름을 변경한다.
 - PC가 관리하는 명령어 주소가 변경된다.
 - 이후 해당 주소에서 명령어를 가져온다.

 ※ while, if, for 같은 C 제어문은
   비교 및 조건부 분기 명령어 등으로 변환될 수 있다.


===============================================================================
 6. 실제 STM32 GPIO 코드에 적용
===============================================================================

 C 코드:

   GPIOA_MODER |= (1U << 10);

 의미:

 - GPIOA_MODER 레지스터의 현재 값을 읽는다.
 - 10번 비트를 1로 설정한다.
 - 변경된 값을 GPIOA_MODER에 다시 기록한다.

 실행될 수 있는 ARM Thumb Assembly 예시:

   LDR R0, =0x40020000    ; GPIOA_MODER 주소 준비
   LDR R1, [R0]           ; 하드웨어 레지스터 읽기
   ORR R1, R1, #0x400     ; 10번 비트를 1로 설정
   STR R1, [R0]           ; 하드웨어 레지스터에 쓰기

 ※ 실제 명령어 구성은 컴파일러와 최적화 옵션에 따라 달라진다.
 ※ LDR R0, =주소는 어셈블러 의사 명령어이다.


 [1] LDR R0, =0x40020000

 - 주소 상수를 CPU 레지스터 R0에 준비한다.

   R0 = 0x40020000


 [2] LDR R1, [R0]

 - Address Bus : 0x40020000 전달
 - Control     : READ
 - Data Bus    : GPIOA_MODER 값 전달
 - CPU R1      : 읽은 값 저장


 [3] ORR R1, R1, #0x400

 - ALU에서 OR 연산을 수행한다.

   R1 = R1 | 0x00000400

 - CPU 내부 연산이다.
 - ORR 연산 자체에는 외부 메모리 접근이 필요하지 않다.


 [4] STR R1, [R0]

 - Address Bus : 0x40020000 전달
 - Data Bus    : R1 값 전달
 - Control     : WRITE
 - GPIOA_MODER : 변경된 값 기록


===============================================================================
 7. 메모리 영역과 CPU 레지스터 구분
===============================================================================

 [.text]
 - 실행할 기계어가 저장되는 영역
 - 일반적인 STM32 프로젝트에서는 Flash에 배치된다.
 - while, if, 함수 호출, 레지스터 접근 등의 명령어가 포함된다.

 [.rodata]
 - 읽기 전용 상수 데이터가 주로 배치되는 영역

 [.data]
 - 초기값이 있는 수정 가능한 전역/static 변수가 주로 배치된다.
 - 초기값은 Flash에 보관되고 부팅 시 RAM으로 복사된다.

 [.bss]
 - 0으로 초기화되는 전역/static 변수가 주로 배치된다.
 - 부팅 시 RAM에서 0으로 초기화된다.

 [Stack]
 - 함수 실행에 필요한 지역 데이터와 보존 레지스터 등에 사용된다.
 - 실제 사용량은 컴파일러가 생성한 코드에 따라 달라진다.

 [Heap]
 - malloc() 등을 통한 동적 메모리 할당에 사용된다.

 [CPU Register]
 - R0~R15 등 CPU 내부 레지스터
 - 주소, 연산값, 결과, 실행 흐름을 관리한다.
 - .text, .data, .bss 같은 메모리 섹션이 아니다.

 [Peripheral Register]
 - RCC, GPIO, USART 등의 하드웨어 레지스터
 - 메모리 매핑된 주소를 통해 CPU가 접근한다.
 - 일반 RAM의 .data 또는 .bss 영역과 다르다.


===============================================================================
 8. 최종 핵심 정리
===============================================================================

 FETCH
 - PC가 명령어 주소를 관리한다.
 - 주소 버스를 통해 명령어 위치를 지정한다.
 - Flash에서 명령어를 읽어 CPU 내부로 가져온다.

 DECODE
 - CU가 명령어를 해독한다.
 - 연산 종류와 사용할 레지스터 등을 결정한다.

 EXECUTE
 - ADD/ORR → CPU 내부 ALU 연산
 - LDR     → Address Bus + Read Data Path로 메모리 읽기
 - STR     → Address Bus + Write Data Path로 메모리 쓰기
 - B/BL    → 실행 흐름 변경

 버스의 역할
 - Address Bus : 어디에 접근할 것인가?
 - Data Bus    : 어떤 데이터를 전달할 것인가?
 - Control     : 읽을 것인가, 쓸 것인가?

 주의
 - Fetch, Decode, Execute는 개념적으로 구분한 단계이다.
 - Cortex-M4는 파이프라인을 사용하므로 여러 명령어의 단계가
   시간적으로 겹쳐 실행될 수 있다.
 - 모든 명령어가 외부 메모리 버스를 이용하는 것은 아니다.
 - CPU 내부 Register File과 외부 메모리 버스는 구분해야 한다.

===============================================================================
*/
