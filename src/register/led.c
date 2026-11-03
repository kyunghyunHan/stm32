#include <stdint.h>

/*
 * RCC (Reset and Clock Control)
 *
 * STM32에서 Clock과 Reset을 관리하는 하드웨어
 * GPIO, UART, SPI, TIMER 등의 주변장치에 클럭 공급
 */

#define RCC_BASE 0x40023800UL

/*
 * AHB1ENR (AHB1 Peripheral Clock Enable Register)
 *
 * AHB1 : Advanced High-performance Bus 1
 * EN   : Enable
 * R    : Register
 *
 * 0 : 주변장치 클럭 비활성화
 * 1 : 주변장치 클럭 활성화
 *
 * RCC_BASE       = 0x40023800
 * AHB1ENR Offset = 0x30
 * RCC_AHB1ENR    = 0x40023830
 */

#define RCC_AHB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x30UL))

/*
 * GPIOA 레지스터
 */

#define GPIOA_BASE 0x40020000UL

// GPIOA Mode Register
#define GPIOA_MODER (*(volatile uint32_t *)(GPIOA_BASE + 0x00UL))

// GPIOA Output Data Register
#define GPIOA_ODR (*(volatile uint32_t *)(GPIOA_BASE + 0x14UL))

/*
 * SysTick
 *
 * ARM Cortex-M 코어에 내장된 24비트 카운트다운 타이머
 *
 * 용도:
 * - ms 단위 시간 지연
 * - RTOS 작업 스케줄링을 위한 Tick
 * - 주기적인 인터럽트 발생
 */

#define SYSTICK_BASE 0xE000E010UL

#define SYSTICK_CTRL (*(volatile uint32_t *)(SYSTICK_BASE + 0x00UL))
#define SYSTICK_LOAD (*(volatile uint32_t *)(SYSTICK_BASE + 0x04UL))
#define SYSTICK_VAL (*(volatile uint32_t *)(SYSTICK_BASE + 0x08UL))

/*
 * CPU Clock = 16MHz 가정
 *
 * 16,000,000Hz / 1000 = 16,000
 * LOAD = 16,000 - 1 = 15,999
 */

#define CPU_CLOCK_HZ 16000000UL

void delay_ms(uint32_t ms)
{
    // 1ms에 해당하는 Reload 값 설정
    SYSTICK_LOAD = (CPU_CLOCK_HZ / 1000U) - 1U;

    /*
     * 현재 카운터 초기화
     *
     * LOAD = 15999
     * VAL  = 0
     *
     * SysTick 활성화 후
     *
     * 15999
     * 15998
     * 15997
     * ...
     * 1
     * 0
     *
     * 다시 15999부터 반복
     */
    SYSTICK_VAL = 0U;

    /*
     * SysTick CTRL
     *
     * Bit 2 CLKSOURCE = 1 : Processor Clock 사용
     * Bit 1 TICKINT   = 0 : 인터럽트 비활성화
     * Bit 0 ENABLE    = 1 : SysTick 활성화
     */
    SYSTICK_CTRL = (1U << 2) | (1U << 0);

    for (uint32_t i = 0; i < ms; i++)
    {
        /*
         * COUNTFLAG (Bit 16)
         *
         * 0 : 아직 카운트 완료 안 됨
         * 1 : 카운터가 0에 도달했음
         */
        while ((SYSTICK_CTRL & (1U << 16)) == 0U)
        {
        }
    }

    // SysTick 비활성화
    SYSTICK_CTRL = 0U;
}

int main(void)
{
    /*
     * 1. GPIOA 클럭 활성화
     *
     * RCC_AHB1ENR Bit 0 = GPIOAEN
     */
    RCC_AHB1ENR |= (1U << 0);

    /*
     * 2. PA5 Output 설정
     *
     * MODER 11:10
     *
     * 00 : Input
     * 01 : Output
     * 10 : Alternate Function
     * 11 : Analog
     */

    // PA5 MODER 초기화
    GPIOA_MODER &= ~(3U << 10);

    // PA5 Output(01)
    GPIOA_MODER |= (1U << 10);

    /*
     * 3. LED Toggle
     */
    while (1)
    {
        // PA5 상태 반전
        GPIOA_ODR ^= (1U << 5);

        // 3초 지연
        delay_ms(3000);
    }
}
/*
실행단계
main.c
   ↓ 전처리
main.i
   ↓ 컴파일
main.s
   ↓ 어셈블
main.o
   ↓ 링킹
firmware.elf
   ↓ objcopy
firmware.bin
   ↓ Flash Programming
STM32 Flash


메모리 영역	일반적인 위치	내용
.isr_vector	Flash	벡터 테이블
.text	Flash	실행할 명령어
.rodata	Flash	읽기 전용 상수
.data	RAM	초기값이 있는 전역·정적 변수
.bss	RAM	0으로 초기화되는 전역·정적 변수
Stack	RAM	함수 호출, 지역변수, 임시 저장
Heap	RAM	동적 메모리 할당


벡터 테이블 — 일반적인 Flash 부팅 예시
주소	저장 내용
0x08000000	초기 MSP 값
0x08000004	Reset_Handler 주소
0x08000008	NMI_Handler 주소
0x0800000C	HardFault_Handler 주소


startUp
*/


/*
================================================================================
 STM32 프로그램 실행 과정 — 완전 순서 정리
================================================================================

 MCU          : STM32F446RE
 CPU          : ARM Cortex-M4
 Architecture : ARMv7E-M
 Boot Mode    : Main Flash Boot (BOOT0 = 0)

 학습 목표

 C 프로그램이 어떻게 기계어로 변환되고,
 Flash에 저장된 Reset_Handler가 어떻게 실행되며,
 CPU의 MSP, PC, LR, Stack, RAM이 어떤 순서로 사용되는지 이해한다.


 전체 실행 순서

 [01] C 소스 코드 작성
 [02] 전처리 (Preprocessing)
 [03] 컴파일 (Compilation)
 [04] 어셈블 (Assembly)
 [05] 링킹 (Linking)
 [06] Flash Programming
 [07] Power ON
 [08] Hardware Reset
 [09] Boot Memory Mapping
 [10] Vector Table 0번 읽기
 [11] MSP 초기화
 [12] Vector Table 1번 읽기
 [13] PC 초기화 및 Thumb 상태 설정
 [14] Reset_Handler 명령어 Fetch
 [15] Reset_Handler 실행
 [16] SystemInit 호출
 [17] .data 초기화
 [18] .bss 초기화
 [19] C Runtime 초기화
 [20] main() 호출
 [21] 사용자 프로그램 실행

 ※ STM32F446RE의 대표적인 ST GCC Startup Code 기준이다.
 ※ Startup 구현에 따라 SystemInit과 메모리 초기화 순서가 달라질 수 있다.
 ※ 아래 주소 중 일부는 동작 원리 설명을 위한 예시이다.


================================================================================
 [01] C 소스 코드 작성
================================================================================

 개발자가 main.c 파일을 작성한다.

 예시

 int main(void)
 {
     while (1)
     {
     }
 }


 중요한 점

 - C 언어는 CPU가 직접 실행하는 명령어가 아니다.
 - CPU는 ARM Thumb 기계어를 실행한다.
 - 따라서 C 소스 코드를 기계어로 변환해야 한다.


================================================================================
 [02] 전처리 (Preprocessing)
================================================================================

 입력 파일 : main.c
 출력 파일 : main.i


 처리 내용

 1. #include 헤더 파일 처리
 2. #define 매크로 치환
 3. #if, #ifdef 등 조건부 컴파일 처리
 4. 주석 제거


 실행 흐름

 main.c
    |
    v
 Preprocessor
    |
    v
 main.i


 ※ 전처리 결과는 아직 기계어가 아니다.


================================================================================
 [03] 컴파일 (Compilation)
================================================================================

 입력 파일 : main.i
 출력 파일 : main.s


 처리 내용

 1. C 언어 문법 및 의미 분석
 2. 타입 검사
 3. 최적화
 4. ARM Thumb 어셈블리 명령어 생성


 실행 흐름

 main.i
    |
    v
 Compiler
    |
    v
 main.s


 ※ main.s는 사람이 읽을 수 있는 어셈블리 코드이다.


================================================================================
 [04] 어셈블 (Assembly)
================================================================================

 입력 파일 : main.s
 출력 파일 : main.o


 처리 내용

 1. 어셈블리 명령어를 기계어로 변환
 2. Section 정보 생성
 3. Symbol 정보 생성
 4. 필요한 재배치 정보 생성


 실행 흐름

 main.s
    |
    v
 Assembler
    |
    v
 main.o


 중요

 main.o는 아직 최종 실행 이미지가 아니다.

 외부 함수와 일부 심볼의 주소가
 최종적으로 결정되지 않았을 수 있다.


================================================================================
 [05] 링킹 (Linking)
================================================================================

 입력 파일

 - main.o
 - startup_stm32f446xx.o
 - system_stm32f4xx.o
 - 기타 라이브러리 및 오브젝트 파일


 링커 스크립트 예시

 STM32F446RETX_FLASH.ld


 처리 내용

 1. 여러 오브젝트 파일 결합
 2. 외부 Symbol 주소 해결
 3. Reset_Handler 주소 결정
 4. main() 주소 결정
 5. Vector Table 배치
 6. .text / .rodata / .data / .bss 배치
 7. 초기 Stack Pointer 값 결정
 8. ELF 파일 생성


 실행 흐름

 main.o
 startup.o
 system.o
 libraries
    |
    v
 Linker + Linker Script
    |
    v
 firmware.elf


 여기서 중요한 사실

 Reset_Handler의 주소는 링커가 최종 결정한다.

 Vector Table의 Reset_Handler 항목에는
 링커가 결정한 주소가 기록된다.


================================================================================
 [06] ELF / BIN / Flash Programming
================================================================================

 ELF에는 다음 정보가 포함될 수 있다.

 - Vector Table
 - 실행 기계어
 - 초기 데이터
 - Section 정보
 - Symbol 정보
 - 디버깅 정보


 필요하면 objcopy로 BIN 파일을 생성한다.

 firmware.elf
      |
      v
   objcopy
      |
      v
 firmware.bin


 이후 ST-LINK 등을 이용해 Flash에 기록한다.


 Flash Programming 이후

 MCU 내부 Flash에는 다음 내용이 저장된다.


 Flash Memory

 0x08000000  +----------------------------------+
             | Vector Table                     |
             |                                  |
             | [0] Initial MSP Value            |
             | [1] Reset_Handler Address        |
             | [2] NMI_Handler Address          |
             | [3] HardFault_Handler Address    |
             | ...                              |
             +----------------------------------+
             | .text                            |
             |                                  |
             | Reset_Handler 기계어             |
             | SystemInit 기계어                |
             | main() 기계어                    |
             | 기타 함수 기계어                 |
             +----------------------------------+
             | .rodata                          |
             | 읽기 전용 데이터                 |
             +----------------------------------+
             | .data 초기값                     |
             +----------------------------------+


 중요

 Reset_Handler에는 두 가지 정보가 관련된다.

 1. Reset_Handler의 주소
    - Vector Table에 저장

 2. Reset_Handler의 실제 기계어
    - Flash의 .text 영역 등에 저장


 Reset_Handler는 Reset 시 새로 생성되는 것이 아니다.

 Flash에 이미 존재하는 코드이다.


================================================================================
 [07] Power ON
================================================================================

 MCU에 전원이 공급된다.


 순서

 1. VDD 전원 공급
 2. 내부 전원 회로 동작
 3. Power-On Reset 회로 동작
 4. 전원 및 리셋 조건 확인


 이 시점에는 main()이 실행되지 않는다.

 Reset_Handler도 아직 실행되지 않는다.


================================================================================
 [08] Hardware Reset
================================================================================

 MCU 내부 리셋 회로가 시스템을 초기 상태로 만든다.


 대표적인 Reset 원인

 - Power-On Reset
 - NRST 핀 Reset
 - Software Reset
 - Watchdog Reset


 Reset 해제 후 CPU는 정해진 부팅 절차를 수행한다.


 중요

 CPU는 Reset 이후 어디서 실행을 시작해야 하는지
 ARM Cortex-M 아키텍처의 하드웨어 규칙으로 알고 있다.


================================================================================
 [09] Boot Memory Mapping
================================================================================

 STM32F446RE에서 BOOT0 = 0이면 Main Flash로 부팅한다.


 실제 Main Flash 주소

 0x08000000


 부팅 시 Flash 시작 부분이
 0x00000000 주소 영역에도 매핑된다.


 Memory Alias

 0x00000000  ---->  0x08000000
 0x00000004  ---->  0x08000004


 의미

 CPU가 부팅 주소 0x00000000을 읽으면
 Main Flash의 첫 번째 데이터를 읽는다.


 주의

 Flash의 데이터를 RAM으로 복사하는 것이 아니다.

 하드웨어 메모리 매핑에 의해
 동일한 Flash 내용을 접근하는 것이다.


================================================================================
 [10] Vector Table 0번 항목 읽기
================================================================================

 ARM Cortex-M4는 Reset 시
 Vector Table의 첫 번째 32비트 값을 읽는다.


 Vector Table

 Index  Address       Content
 -------------------------------------------------------
 [0]    0x08000000    Initial MSP Value
 [1]    0x08000004    Reset_Handler Address
 [2]    0x08000008    NMI_Handler Address
 [3]    0x0800000C    HardFault_Handler Address


 예시

 Flash 주소           저장된 값
 -------------------------------------------------------
 0x08000000           0x20020000
 0x08000004           0x08000101


 첫 번째 접근

 CPU
  |
  | 0x00000000 주소의 값 요청
  v
 Boot Memory Mapping
  |
  v
 Flash 0x08000000
  |
  | 저장된 값 읽기
  v
 0x20020000


 여기서 구분해야 하는 것

 0x08000000 = 읽는 메모리 주소

 0x20020000 = 그 주소에 저장된 데이터


 아직 Reset_Handler는 실행되지 않았다.


================================================================================
 [11] MSP 초기화
================================================================================

 MSP = Main Stack Pointer

 SP  = Stack Pointer
 R13 = SP


 Cortex-M4에는 두 종류의 Stack Pointer가 있다.

 - MSP : Main Stack Pointer
 - PSP : Process Stack Pointer


 Reset 직후 CPU는 MSP를 사용한다.


 앞 단계에서 읽은 데이터

 0x20020000


 하드웨어가 수행하는 동작

 MSP <- 0x20020000


 결과

 MSP = 0x20020000


 의미

 MSP는 RAM의 Stack 위치를 가리킨다.


 CPU 내부

 +--------------------------+
 | MSP Register             |
 |                          |
 | 0x20020000               |
 +--------------------------+
             |
             | Stack 위치를 가리킴
             v
 RAM 0x20020000


 중요한 사실

 1. MSP는 CPU 내부 레지스터이다.
 2. MSP에는 Stack 주소가 저장된다.
 3. Reset_Handler의 기계어는 MSP에 저장되지 않는다.
 4. 이 시점에 Stack에 데이터를 PUSH한 것은 아니다.


 왜 MSP를 먼저 초기화할까?

 Reset_Handler 실행 중 함수 호출이나
 레지스터 저장 등으로 Stack이 필요할 수 있기 때문이다.

 따라서 명령어 실행을 시작하기 전에
 Stack Pointer를 준비해야 한다.


================================================================================
 [12] Vector Table 1번 항목 읽기
================================================================================

 MSP 초기값을 가져온 뒤
 CPU는 Vector Table 두 번째 항목을 읽는다.


 Index 1

 Flash 주소 : 0x08000004


 예시

 Flash[0x08000004] = 0x08000101


 하드웨어 접근

 CPU
  |
  | 0x00000004 주소의 값 요청
  v
 Boot Memory Mapping
  |
  v
 Flash 0x08000004
  |
  | 저장된 값 읽기
  v
 0x08000101


 여기서 매우 중요한 차이

 0x08000004
 - Reset_Handler 주소가 저장된 위치

 0x08000101
 - Reset_Handler를 가리키는 벡터 값

 0x08000100
 - Reset_Handler의 실제 명령어 시작 위치


 즉,

 Vector Table 1번에는
 Reset_Handler의 기계어가 들어 있는 것이 아니다.

 Reset_Handler가 어디에 있는지를 알려주는
 주소 정보가 저장되어 있다.


================================================================================
 [13] PC 초기화 / Thumb 상태 설정
================================================================================

 PC = Program Counter
 R15 = PC


 CPU가 읽은 Reset Vector

 0x08000101


 Cortex-M4는 Thumb 상태로 명령어를 실행한다.

 Reset Vector의 Bit 0은 Thumb 상태를 나타낸다.


 0x08000101
           |
           +---- Bit 0 = 1


 실제 명령어 시작 주소

 0x08000100


 하드웨어는 Reset Vector를 해석하여
 실행 시작 위치와 Thumb 상태를 설정한다.


 개념적인 결과

 PC -> 0x08000100


 주의

 CPU가 0x08000101이라는 홀수 주소에서
 명령어를 읽는 것은 아니다.


================================================================================
 [14] Reset_Handler 첫 번째 명령어 Fetch
================================================================================

 현재 CPU 상태 예시

 MSP = 0x20020000

 PC  = Reset_Handler 시작 주소


 CPU는 PC가 나타내는 Flash 주소에서
 첫 번째 기계어 명령어를 가져온다.


 CPU
  |
  | PC가 가리키는 주소
  v
 Flash 0x08000100
  |
  | Instruction Fetch
  v
 CPU Instruction Decoder
  |
  v
 Instruction Execute


 명령어 실행 과정

 1. Fetch
    - Flash에서 명령어 가져오기

 2. Decode
    - 가져온 명령어 해석

 3. Execute
    - 명령어 실행


 이 순간부터 Reset_Handler의
 실제 기계어 실행이 시작된다.


================================================================================
 [15] Reset_Handler 실행
================================================================================

 Reset_Handler는 Startup Assembly에 정의된 함수이다.


 대표 파일

 startup_stm32f446xx.s


 Startup Assembly 예시

 Reset_Handler:
     bl SystemInit

     ... .data 초기화 ...

     ... .bss 초기화 ...

     bl __libc_init_array
     bl main


 ST의 대표적인 STM32F446 GCC Startup 구현에서는

 1. SystemInit 호출
 2. .data 초기화
 3. .bss 초기화
 4. C Runtime 초기화
 5. main 호출

 순서로 진행된다.


 ※ 실제 Startup Code 구현에 따라 순서는 달라질 수 있다.


================================================================================
 [16] SystemInit() 함수 호출
================================================================================

 대표적인 명령어

 BL SystemInit


 BL = Branch with Link


 BL 명령어의 주요 동작

 1. 복귀 주소를 LR에 저장
 2. PC를 SystemInit 시작 위치로 변경
 3. SystemInit 명령어 실행


 LR = Link Register
 R14 = LR


 예시

 Reset_Handler의 BL 명령어 실행
              |
              v
 LR <- 복귀 주소
              |
              v
 PC -> SystemInit
              |
              v
 SystemInit 실행


 SystemInit의 역할

 - 시스템 제어 레지스터 초기 설정
 - RCC 관련 기본 설정
 - 필요 시 FPU 설정
 - Vector Table 관련 설정


 실행이 완료되면 복귀 주소를 사용하여
 Reset_Handler의 다음 명령어로 돌아온다.


================================================================================
 [17] .data 초기화
================================================================================

 .data는 초기값이 있는 전역변수와
 정적변수 등을 위한 메모리 영역이다.


 예시

 int a = 10;


 프로그램 기록 후

 Flash
 - 변수 a의 초기값 10 보관

 RAM
 - 변수 a의 실행 시 저장 공간


 Reset_Handler는 Flash의 초기값을
 RAM의 .data 영역으로 복사한다.


 실제 동작

 1. Flash의 .data 초기값 주소 준비
 2. RAM의 .data 시작 주소 준비
 3. Flash에서 데이터 읽기
 4. CPU 레지스터에 데이터 저장
 5. RAM 주소에 데이터 기록
 6. .data 끝까지 반복


 ARM Assembly 예시

 LDR R0, =_sdata
 LDR R1, =_edata
 LDR R2, =_sidata
 MOVS R3, #0


 각 레지스터 역할

 R0 = RAM .data 시작 주소
 R1 = RAM .data 끝 주소
 R2 = Flash 초기값 시작 주소
 R3 = 복사 오프셋


 실제 데이터 복사

 LDR R4, [R2, R3]
 STR R4, [R0, R3]
 ADDS R3, R3, #4


 예시 주소

 R2 = 0x08004000
 R0 = 0x20000000
 R3 = 0


 Flash[0x08004000] = 0x0000000A


 LDR 실행

 R4 <- Flash[0x08004000]

 R4 = 10


 STR 실행

 RAM[0x20000000] <- R4

 RAM[0x20000000] = 10


 결과

 RAM의 .data 영역에 초기값이 준비된다.


================================================================================
 [18] .bss 초기화
================================================================================

 .bss는 0으로 초기화해야 하는
 전역변수와 정적변수 등을 위한 영역이다.


 예시

 int b;


 Reset_Handler의 동작

 1. .bss 시작 주소 확인
 2. .bss 끝 주소 확인
 3. CPU 레지스터에 0 준비
 4. RAM에 0 기록
 5. 끝 주소까지 반복


 ARM Assembly 예시

 LDR R2, =_sbss
 LDR R4, =_ebss
 MOVS R3, #0


 반복 동작

 STR R3, [R2]
 ADDS R2, R2, #4
 CMP R2, R4
 BCC FillZerobss


 결과

 .bss 영역 = 0


 중요

 .data는 Flash에서 초기값을 RAM으로 복사한다.

 .bss는 RAM 영역을 0으로 초기화한다.


================================================================================
 [19] C Runtime 초기화
================================================================================

 Startup Code는 필요한 C Runtime 초기화 작업을 수행한다.


 대표 함수

 __libc_init_array()


 주요 역할

 - 필요한 초기화 함수 실행
 - C/C++ Runtime 초기화
 - C++ 전역 객체 생성자 호출 등


 그 이후 main()을 실행할 준비가 완료된다.


================================================================================
 [20] main() 함수 호출
================================================================================

 Reset_Handler에서 다음 명령어를 실행한다.


 BL main


 실제 동작

 1. 현재 함수의 복귀 주소를 LR에 기록
 2. main()의 실행 주소로 분기
 3. PC가 main() 시작 위치를 가리킴
 4. main() 명령어 Fetch
 5. Decode
 6. Execute


 흐름

 Reset_Handler
       |
       v
    BL main
       |
       +---- LR <- 복귀 주소
       |
       +---- PC -> main 시작 주소
                      |
                      v
                 main() 실행


 주의

 main() 또한 Flash에 저장된 기계어 코드이다.

 CPU는 C 소스 파일을 직접 읽지 않는다.


================================================================================
 [21] main() 이후의 일반적인 실행 원리
================================================================================

 main()이 시작되면 CPU는 프로그램 명령어를 실행한다.


 일반적인 동작

 1. PC가 명령어 주소를 가리킴
 2. CPU가 명령어 Fetch
 3. 명령어 Decode
 4. 명령어 Execute
 5. 다음 명령어 실행


 함수 호출 시

 BL Function
      |
      +---- LR에 복귀 주소 기록
      |
      +---- PC가 Function으로 분기


 함수 실행 중 필요하면

 PUSH
 - CPU 레지스터를 Stack에 저장
 - SP 감소

 POP
 - Stack 데이터를 CPU 레지스터로 복원
 - SP 증가


 함수가 복귀하면

 PC가 호출한 코드의 복귀 위치로 이동한다.


================================================================================
 [22] Stack 사용 시 실제 주소 변화
================================================================================

 초기 MSP 예시

 MSP = 0x20020000


 함수 실행 중

 PUSH {R4, LR}


 저장할 데이터

 R4 = 4Byte
 LR = 4Byte

 총 8Byte


 PUSH 전

 SP = 0x20020000


 PUSH 후

 SP = 0x2001FFF8


 RAM

 0x2001FFF8  R4 값
 0x2001FFFC  LR 값
 0x20020000  이전 SP 위치


 함수 복귀 시

 POP {R4, PC}


 동작

 1. R4 값 복원
 2. 저장된 복귀 주소를 PC로 복원
 3. SP 8Byte 증가


 결과

 SP = 0x20020000


 ※ BL 자체가 반드시 Stack에 값을 저장하지는 않는다.
 ※ 실제 PUSH/POP 여부는 컴파일러가 생성한 코드에 따라 다르다.


================================================================================
 [23] 프로그램 실행 중 인터럽트가 발생하면?
================================================================================

 정상 프로그램 실행
       |
       v
 인터럽트 발생
       |
       v
 NVIC가 인터럽트 우선순위와 허용 상태 판단
       |
       v
 Cortex-M Exception Entry
       |
       v
 현재 실행 상태 일부를 Stack에 저장
       |
       v
 Vector Table에서 Handler 주소 조회
       |
       v
 PC가 해당 Handler로 분기
       |
       v
 ISR 실행
       |
       v
 Exception Return
       |
       v
 Stack에 저장된 상태 복원
       |
       v
 원래 프로그램 실행 재개


 기본 Exception Stack Frame

 R0
 R1
 R2
 R3
 R12
 LR
 PC
 xPSR


 총 기본 8개 레지스터 = 32Byte


 ※ 추가 정렬 공간이나 FPU 상태 저장이 발생할 수 있다.
 ※ Reset은 일반적인 인터럽트 진입과 다르다.
 ※ Reset_Handler를 실행할 때 이전 실행 문맥을
    일반 ISR처럼 Stack에 저장하지 않는다.


================================================================================
 [24] Vector Table의 전체 기본 구조
================================================================================

 Cortex-M4 Vector Table

 Index  Address Offset   Content
 -----------------------------------------------------------------------
 0      0x00             Initial MSP
 1      0x04             Reset_Handler
 2      0x08             NMI_Handler
 3      0x0C             HardFault_Handler
 4      0x10             MemManage_Handler
 5      0x14             BusFault_Handler
 6      0x18             UsageFault_Handler
 7      0x1C             Reserved
 8      0x20             Reserved
 9      0x24             Reserved
 10     0x28             Reserved
 11     0x2C             SVC_Handler
 12     0x30             DebugMon_Handler
 13     0x34             Reserved
 14     0x38             PendSV_Handler
 15     0x3C             SysTick_Handler
 16     0x40             IRQ0 Handler
 17     0x44             IRQ1 Handler
 ...    ...              ...


 중요

 Index 0
 - 초기 Stack Pointer 값
 - Handler 함수가 아니다.

 Index 1
 - Reset_Handler 주소
 - Reset 발생 시 사용한다.

 Index 2
 - NMI 발생 시 사용한다.

 Index 3
 - HardFault 발생 시 사용한다.


 Vector Table은 0, 1, 2, 3 순서로
 함수를 실행하는 목록이 아니다.

 각 예외가 발생했을 때
 해당 Handler 주소를 찾기 위한 테이블이다.


================================================================================
 [25] 왜 Initial MSP가 0번이고 Reset_Handler가 1번인가?
================================================================================

 ARM Cortex-M 아키텍처의 하드웨어 규칙이기 때문이다.


 Reset 시 CPU가 필요한 정보

 첫 번째:
 - Stack을 사용할 주소

 두 번째:
 - 실행을 시작할 명령어 주소


 따라서

 Vector Table[0] = Initial MSP

 Vector Table[1] = Reset_Handler Address


 CPU 하드웨어가 Reset 시 두 값을 읽는다.


 CPU는 C 코드로

 MSP = ...
 PC = ...

 라는 명령을 먼저 실행하는 것이 아니다.

 하드웨어 리셋 시퀀스가 자동으로 수행한다.


================================================================================
 [26] 최종 전체 흐름
================================================================================

 main.c 작성
      |
      v
 Preprocessing
      |
      v
 Compilation
      |
      v
 Assembly
      |
      v
 Linking
      |
      v
 firmware.elf
      |
      v
 firmware.bin (필요한 경우)
      |
      v
 Flash Programming
      |
      v
 Flash에 Vector Table + Reset_Handler + main 저장
      |
      v
 Power ON
      |
      v
 Hardware Reset
      |
      v
 Boot Memory Mapping
      |
      v
 Vector Table[0] 읽기
      |
      v
 MSP <- 초기 Stack 주소
      |
      v
 Vector Table[1] 읽기
      |
      v
 PC <- Reset_Handler 실행 주소
      |
      v
 Reset_Handler 첫 명령어 Fetch
      |
      v
 Decode / Execute
      |
      v
 SystemInit()
      |
      v
 .data 초기화
      |
      v
 .bss 초기화
      |
      v
 C Runtime 초기화
      |
      v
 BL main
      |
      v
 PC -> main() 시작 주소
      |
      v
 main() 실행
      |
      v
 함수 호출 / 메모리 접근 / 주변장치 제어
      |
      v
 프로그램 실행 지속


================================================================================
 [27] 반드시 기억할 핵심
================================================================================

 1. Reset_Handler는 Flash에 저장된 기계어 코드이다.

 2. Reset_Handler의 주소는 Vector Table[1]에 저장된다.

 3. Vector Table[0]에는 초기 MSP 값이 저장된다.

 4. Reset 후 CPU 하드웨어가 MSP와 PC를 초기화한다.

 5. MSP에는 Stack 주소가 들어간다.

 6. PC에는 명령어 실행 위치가 들어간다.

 7. Reset_Handler는 MSP 안에 저장되지 않는다.

 8. Reset_Handler는 main()보다 먼저 실행된다.

 9. .data는 Flash에서 RAM으로 복사된다.

10. .bss는 RAM에서 0으로 초기화된다.

11. main()은 Startup 초기화 이후 호출된다.

12. LR은 함수 복귀 주소를 관리한다.

13. SP는 Stack 위치를 관리한다.

14. PC는 명령어 실행 흐름을 관리한다.

15. Vector Table은 실행 순서표가 아니라
    초기 MSP 값과 예외 Handler 주소를 저장한 테이블이다.


================================================================================
*/
