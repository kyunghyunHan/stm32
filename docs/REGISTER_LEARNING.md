
# STM32 C - 레지스터 직접 제어 학습

> 현재 VS Code/Make 설정은 **STM32F767VI** 기준입니다. 아래의 기존 F446 설명 대신 [F767 빌드 방법](docs/F767_BUILD.md)을 참고하세요. C 소스는 수정하지 않았으며, F446용 예제는 F767 빌드 대상에서 제외했습니다.

STM32 마이크로컨트롤러(MCU)의 레지스터를 C언어로 직접 제어하는 학습용 프로젝트입니다.

HAL 라이브러리 대신 레지스터 직접 접근 방식을 사용하여 GPIO, CLOCK, ADC, UART, CAN 등 MCU 주변장치의 동작 원리를 학습합니다.

---

## 1. 개발 환경

| 항목 | 내용 |
|---|---|
| MCU | STM32F446xE |
| CPU | ARM Cortex-M4 |
| Flash Memory | 512 KiB |
| RAM | 128 KiB |
| 프로그래밍 언어 | C |
| 컴파일러 | arm-none-eabi-gcc |
| SDK | STM32CubeF4 (CMSIS) |
| 빌드 시스템 | Makefile |
| 다운로드 및 디버깅 | ST-LINK, OpenOCD |

현재 Makefile은 STM32F446xE를 대상으로 설정되어 있습니다.

STM32F767 관련 학습 소스는 별도로 관리하며, 현재 F446 빌드 대상에는 포함되지 않습니다.

---

## 2. 프로젝트 예제

| 예제 | 기능 |
|---|---|
| `led` | PA5 LED를 약 3초마다 반전 (16 MHz 기준) |
| `led_swicth` | PC13 스위치 입력으로 PA5 LED 제어 |
| `can` | CAN1 내부 루프백 통신 테스트 |

### LED 예제

- PA5를 GPIO 출력으로 설정합니다.
- 약 3초 간격으로 LED 상태를 반전시킵니다.
- GPIO 클럭 설정과 출력 레지스터 제어를 학습합니다.

### LED + SWITCH 예제

- PC13을 GPIO 입력으로 설정합니다.
- PA5를 GPIO 출력으로 설정합니다.
- PC13이 LOW이면 LED ON, HIGH이면 LED OFF로 동작합니다.
- GPIO 입력 레지스터와 출력 레지스터 사용법을 학습합니다.

> `led_swicth`는 기존 파일 이름을 그대로 사용합니다. 실제 LED 및 버튼의 극성과 배선은 보드 회로도를 확인해야 합니다.

### CAN 예제

- CAN1 내부 루프백(Loopback) 모드로 통신합니다.
- 송신한 CAN 메시지를 내부적으로 수신합니다.
- 디버거에서 수신 데이터와 상태를 확인합니다.

---

## 3. 개발 환경 준비

### 필요한 프로그램

- Arm GNU Toolchain
  - `arm-none-eabi-gcc`
  - `arm-none-eabi-objcopy`
  - `arm-none-eabi-size`
- STM32CubeF4 SDK
- OpenOCD
- ST-LINK
- clang-format

macOS 환경에서는 `/Applications/STM32CubeIDE.app`에 포함된 툴체인을 우선 사용합니다.

해당 툴체인이 없으면 시스템 PATH에 등록된 `arm-none-eabi-*`를 사용합니다.

직접 경로를 지정할 수도 있습니다.

```bash
make stm32-build CROSS_COMPILE=/path/to/bin/arm-none-eabi-
```

### STM32CubeF4 SDK

기본 SDK 탐색 경로:

```bash
~/STM32Cube/Repository/STM32Cube_FW_F4_V*
```

다른 위치에 설치되어 있다면 직접 지정합니다.

```bash
make stm32-build CUBE_F4=/path/to/STM32Cube_FW_F4_V1.28.3
```

링커 스크립트는 STM32F446xE와 동일한 Flash/RAM 용량을 가진 F446ZE 템플릿을 사용합니다.

필요한 경우 다음과 같이 지정합니다.

```bash
make stm32-build LINKER_SCRIPT=/path/to/board.ld
```

---

## 4. Makefile 사용 방법

### 기본 빌드

```bash
make stm32-build
```

기본 예제인 `led`를 빌드합니다.

### 컴파일 검사

```bash
make stm32-check
```

또는

```bash
make check
```

선택한 예제의 C 컴파일을 검사합니다.

### 보드에 프로그램 다운로드

```bash
make stm32-run
```

ST-LINK를 이용하여 MCU Flash Memory에 프로그램을 기록하고 실행합니다.

### 예제 선택

```bash
make stm32-build EXAMPLE=led
make stm32-build EXAMPLE=led_swicth
make stm32-build EXAMPLE=can
```

### 전체 명령어

| 명령어 | 설명 |
|---|---|
| `make stm32-check` | 선택한 예제 컴파일 검사 |
| `make stm32-build` | ELF/BIN 빌드 및 링크 |
| `make stm32-run` | ST-LINK로 다운로드 및 실행 |
| `make check-all` | 세 가지 F446 예제 컴파일 검사 |
| `make lint` | 컴파일 경고를 오류로 검사 |
| `make fmt` | C 소스코드 포맷 정리 |
| `make fmt-check` | C 소스코드 포맷 검사 |
| `make clean` | 빌드 결과 삭제 |
| `make help` | 명령어 안내 |

### 빌드 결과

빌드 결과는 다음 경로에 생성됩니다.

```text
target/
 ├── led/
 │   ├── *.elf
 │   ├── *.bin
 │   └── *.map
 ├── led_swicth/
 └── can/
```

- ELF : 실행 코드와 디버깅 정보 등을 포함하는 파일
- BIN : MCU Flash에 기록할 수 있는 바이너리 파일
- MAP : 메모리 및 심볼 배치 정보를 확인하는 파일

---

# STM32 핵심 개념 정리

## 5. MCU란?

MCU(Microcontroller Unit)는 CPU, 메모리, 주변장치가 하나의 반도체 칩에 통합된 제어용 컴퓨터입니다.

주요 구성 요소:

- CPU
- Flash Memory
- RAM
- CLOCK
- BUS
- GPIO
- ADC
- TIMER
- UART
- I2C
- CAN

MCU 내부 구성 개념:

```text
             STM32 MCU
                 |
       +---------+---------+
       |                   |
      CPU                Memory
       |                   |
       |              +----+----+
       |              |         |
       |             Flash     RAM
       |
       +------ BUS ------+
                         |
             +-----------+-----------+
             |           |           |
            GPIO        ADC         TIMER
             |           |           |
            LED       Sensor        PWM
                         |
                       UART
                         |
                     External
                      Device
```

---

## 6. Flash Memory

Flash Memory는 전원이 꺼져도 데이터가 유지되는 비휘발성 메모리입니다.

### 주요 특징

- MCU에서 실행할 프로그램을 저장합니다.
- 전원을 꺼도 프로그램이 유지됩니다.
- ST-LINK를 통해 펌웨어를 다운로드할 수 있습니다.
- 일반적으로 프로그램 코드와 상수 데이터 등을 저장합니다.

예시:

```text
Flash Memory
    |
    +-- Startup Code
    |
    +-- Interrupt Vector Table
    |
    +-- Program Code
    |
    +-- Constant Data
```

STM32F446xE의 Flash Memory 용량은 512 KiB입니다.

---

## 7. RAM

RAM(Random Access Memory)은 프로그램 실행 중 데이터를 임시로 저장하는 메모리입니다.

### 주요 특징

- 전원이 꺼지면 일반적으로 데이터가 사라집니다.
- 변수, 스택, 힙 등을 저장하는 데 사용합니다.
- CPU가 실행 중인 데이터를 읽거나 쓸 수 있습니다.

예시:

```c
int count = 0;
float voltage = 3.3f;
```

이러한 일반 변수는 실행 중 RAM에 배치될 수 있습니다.

STM32F446xE의 RAM 용량은 128 KiB입니다.

### Flash와 RAM 차이

| 구분 | Flash | RAM |
|---|---|---|
| 전원 OFF | 데이터 유지 | 데이터 소멸 |
| 주요 목적 | 프로그램 저장 | 실행 중 데이터 저장 |
| 프로그램 다운로드 | 사용 | 일반적으로 사용하지 않음 |
| 주요 데이터 | 코드, 상수 | 변수, 스택, 힙 |

---

## 8. BUS

BUS는 MCU 내부에서 CPU, 메모리, 주변장치 사이에 데이터와 제어 정보를 전달하는 통로입니다.

STM32에서는 대표적으로 AHB와 APB 버스를 사용합니다.

### AHB (Advanced High-performance Bus)

고속 데이터 전송을 위한 버스입니다.

주요 연결 대상:

- CPU 및 메모리 시스템
- GPIO
- DMA
- 고속 주변장치

### APB (Advanced Peripheral Bus)

상대적으로 낮은 속도로 동작하는 주변장치를 연결하는 버스입니다.

STM32F4/F7에서 대표적으로 다음과 같이 구성됩니다.

| 버스 | 주요 주변장치 예시 |
|---|---|
| AHB1 | GPIO, DMA |
| APB1 | USART2, USART3, TIM2, I2C |
| APB2 | TIM1, ADC1, SPI1, USART1 |

주변장치를 사용하려면 RCC 레지스터에서 해당 주변장치의 클럭을 활성화해야 합니다.

---

## 9. CLOCK

CLOCK은 MCU 내부 회로가 일정한 타이밍에 맞춰 동작하도록 하는 기준 신호입니다.

### 주요 용어

| 용어 | 의미 |
|---|---|
| HSI | 내부 고속 RC 발진기 |
| HSE | 외부 고속 클럭 소스 |
| PLL | 입력 클럭의 주파수를 변환하는 회로 |
| SYSCLK | 시스템 클럭 |
| HCLK | AHB 버스 및 코어 관련 클럭 |
| PCLK1 | APB1 주변장치 클럭 |
| PCLK2 | APB2 주변장치 클럭 |

### CLOCK 구조

```text
HSI / HSE
    |
    v
   PLL
    |
    v
  SYSCLK
    |
    v
 AHB Prescaler
    |
    v
   HCLK
    |
    +----------------+
    |                |
    v                v
APB1 Prescaler   APB2 Prescaler
    |                |
    v                v
  PCLK1            PCLK2
```

### PLL 클럭 계산

STM32F7의 일반적인 PLL 시스템 클럭 계산식:

```text
SYSCLK = (HSE / PLLM) × PLLN / PLLP
```

예시:

```text
HSE  = 16 MHz
PLLM = 8
PLLN = 216
PLLP = 2

SYSCLK = (16 / 8) × 216 / 2
       = 216 MHz
```

이 계산은 STM32F767의 클럭 설정 예시이며 STM32F446의 최대 동작 클럭을 의미하지 않습니다.

STM32F446의 최대 SYSCLK는 180 MHz입니다.

---

## 10. GPIO

GPIO(General Purpose Input/Output)는 MCU의 핀을 디지털 입력 또는 출력으로 사용하는 기능입니다.

### 입력 예시

- 버튼
- 스위치
- 디지털 센서

### 출력 예시

- LED
- 부저
- 외부 장치 제어 신호

GPIO를 사용하려면 다음 과정을 수행합니다.

1. GPIO 주변장치 클럭 활성화
2. GPIO 동작 모드 설정
3. 입력 또는 출력 기능 사용

### GPIO 주요 레지스터

| 레지스터 | 기능 |
|---|---|
| MODER | 핀 동작 모드 설정 |
| OTYPER | 출력 타입 설정 |
| OSPEEDR | 출력 신호 속도 특성 설정 |
| PUPDR | Pull-up / Pull-down 설정 |
| IDR | 입력 상태 읽기 |
| ODR | 출력 상태 설정 및 읽기 |
| BSRR | 출력 비트 Set/Reset |
| AFR | Alternate Function 설정 |

---

## 11. MODER

MODER(Mode Register)는 GPIO 핀의 동작 모드를 설정하는 레지스터입니다.

각 GPIO 핀은 2비트를 사용합니다.

| MODER 값 | 동작 |
|---|---|
| `00` | Input |
| `01` | General Purpose Output |
| `10` | Alternate Function |
| `11` | Analog |

### PA5를 출력으로 설정

STM32F446 기준:

```c
#include "stm32f446xx.h"

void GPIO_Init(void)
{
    // GPIOA 클럭 활성화
    RCC->AHB1ENR |= (1U << 0);

    // PA5 MODER 비트 초기화
    GPIOA->MODER &= ~(3U << 10);

    // PA5를 출력 모드(01)로 설정
    GPIOA->MODER |= (1U << 10);
}
```

PA5의 MODER 비트 위치:

```text
MODER5[1:0] = bit[11:10]

00 : Input
01 : Output
10 : Alternate Function
11 : Analog
```

---

## 12. IDR

IDR(Input Data Register)는 GPIO 핀의 실제 디지털 입력 상태를 읽는 레지스터입니다.

- `0` : LOW
- `1` : HIGH

### PC13 스위치 입력 읽기

```c
if ((GPIOC->IDR & (1U << 13)) == 0U)
{
    // PC13 입력이 LOW
}
else
{
    // PC13 입력이 HIGH
}
```

실제 스위치의 눌림 상태는 회로의 Pull-up / Pull-down 및 배선에 따라 결정됩니다.

---

## 13. ODR

ODR(Output Data Register)는 GPIO 출력 상태를 설정하거나 읽는 레지스터입니다.

### PA5 HIGH 출력

```c
GPIOA->ODR |= (1U << 5);
```

### PA5 LOW 출력

```c
GPIOA->ODR &= ~(1U << 5);
```

### PA5 출력 반전

```c
GPIOA->ODR ^= (1U << 5);
```

### 비트 연산 정리

| 연산자 | 의미 |
|---|---|
| `\|` | OR |
| `&` | AND |
| `~` | NOT |
| `^` | XOR |
| `<<` | Left Shift |
| `>>` | Right Shift |

---

## 14. BSRR

BSRR(Bit Set/Reset Register)는 GPIO 출력 비트를 원자적으로 설정하거나 초기화하는 레지스터입니다.

### 비트 구조

```text
BSRR[31:16] : Reset
BSRR[15:0]  : Set
```

### PA5 HIGH

```c
GPIOA->BSRR = (1U << 5);
```

### PA5 LOW

```c
GPIOA->BSRR = (1U << 21);
```

BSRR은 ODR의 Read-Modify-Write 과정 없이 특정 출력 비트를 변경할 수 있습니다.

인터럽트 등 여러 실행 흐름에서 GPIO를 제어할 때 유용합니다.

---

## 15. ADC

ADC(Analog-to-Digital Converter)는 아날로그 전압을 디지털 데이터로 변환하는 주변장치입니다.

### 주요 용도

- 배터리 전압 측정
- 전류 센서 측정
- 온도 센서 측정
- 가변저항 측정
- 스로틀 전압 측정

### 12-bit ADC

12비트 ADC의 디지털 출력 범위:

```text
0 ~ 4095
```

입력 전압 환산식:

```text
Voltage = ADC_Value × VREF / 4095
```

예시:

```text
VREF      = 3.3 V
ADC_Value = 2048

Voltage = 2048 × 3.3 / 4095
        ≈ 1.65 V
```

### 관련 레지스터

| 레지스터 | 기능 |
|---|---|
| ADC_CR1 | ADC 동작 제어 |
| ADC_CR2 | ADC 활성화 및 변환 시작 |
| ADC_SMPR | 샘플링 시간 설정 |
| ADC_SQR | 변환 채널 순서 설정 |
| ADC_SR | ADC 상태 확인 |
| ADC_DR | 변환 결과 데이터 |

레지스터 구성과 세부 비트 정의는 MCU 제품군에 따라 달라질 수 있습니다.

---

## 16. UART

UART(Universal Asynchronous Receiver/Transmitter)는 비동기 직렬 통신을 위한 주변장치입니다.

### 주요 특징

- TX : 데이터 송신
- RX : 데이터 수신
- 별도의 클럭 라인 없이 통신
- Baud Rate를 송신기와 수신기가 동일하게 설정해야 함

### 연결 구조

```text
STM32                 External Device

TX  ----------------> RX
RX  <---------------- TX
GND ----------------- GND
```

### UART 통신 설정 예시

```text
Baud Rate : 115200 bps
Data Bits : 8
Parity    : None
Stop Bits : 1
```

위 설정을 일반적으로 `115200 8N1`이라고 표현합니다.

### 주요 레지스터

- USART_CR1 : UART 활성화, 송수신 설정
- USART_BRR : Baud Rate 설정
- USART_SR / USART_ISR : 통신 상태 확인
- USART_DR / USART_TDR / USART_RDR : 송수신 데이터

레지스터 명칭은 STM32 제품군에 따라 차이가 있습니다.

---

## 17. I2C

I2C(Inter-Integrated Circuit)는 두 개의 신호선을 사용하는 동기식 직렬 통신 방식입니다.

### 신호선

- SDA : Serial Data
- SCL : Serial Clock

### 연결 구조

```text
             STM32
               |
         +-----+-----+
         |           |
        SDA         SCL
         |           |
   +-----+-----------+-----+
   |                       |
 EEPROM                  SENSOR
```

### 주요 특징

- 두 개의 신호선 사용
- 장치 주소를 이용하여 통신
- 하나의 버스에 여러 장치 연결 가능
- SDA와 SCL에 적절한 Pull-up 저항 필요

### 주요 활용

- EEPROM
- 온도 센서
- 각종 디지털 센서
- 외부 설정 장치

---

## 18. CAN

CAN(Controller Area Network)은 높은 신뢰성이 필요한 장치 간 통신에 사용하는 메시지 기반 통신 방식입니다.

### 주요 특징

- 여러 노드 간 통신 가능
- 메시지 ID 기반 우선순위 처리
- 오류 검출 및 재전송 기능
- 차량 및 산업용 제어 시스템에서 널리 사용

### 내부 루프백 모드

```text
             CAN1
              |
          Transmit
              |
              v
        Internal Loopback
              |
              v
           Receive
```

내부 루프백 모드는 외부 CAN 트랜시버 없이 CAN 컨트롤러의 송수신 동작을 테스트할 수 있습니다.

---

## 19. TIMER / COUNTER

Timer/Counter는 클럭을 기준으로 시간을 측정하거나 이벤트 횟수를 계산하는 주변장치입니다.

### 주요 기능

- 주기적 인터럽트
- PWM 생성
- 시간 측정
- 입력 캡처
- 출력 비교
- 이벤트 카운트

### 주요 레지스터

| 레지스터 | 기능 |
|---|---|
| TIM_PSC | Prescaler |
| TIM_ARR | Auto Reload |
| TIM_CNT | 현재 카운터 값 |
| TIM_CCR | Capture / Compare |
| TIM_CR1 | 타이머 동작 설정 |
| TIM_DIER | 인터럽트 및 DMA 요청 설정 |
| TIM_SR | 타이머 상태 확인 |

### 타이머 주파수 계산

일반적인 Up-counting 모드 기준:

```text
Counter Clock = TIM_CLK / (PSC + 1)

Update Frequency = TIM_CLK / ((PSC + 1) × (ARR + 1))
```

예시:

```text
TIM_CLK = 16 MHz
PSC     = 15999
ARR     = 999

Counter Clock = 16 MHz / 16000
              = 1000 Hz

Update Frequency = 1000 / 1000
                 = 1 Hz
```

즉, 약 1초마다 Update Event가 발생합니다.

---

## 20. ST-LINK

ST-LINK는 STM32 MCU의 프로그램 다운로드 및 디버깅을 지원하는 하드웨어 디버거입니다.

### 주요 기능

- Flash Memory 프로그램 다운로드
- MCU Reset
- Breakpoint 설정
- Step 실행
- 변수 및 레지스터 확인
- 메모리 읽기

### 기본 연결

```text
ST-LINK               STM32

SWDIO  -------------- SWDIO
SWCLK  -------------- SWCLK
GND    -------------- GND
VTref  -------------- Target VDD
NRST   -------------- NRST (선택)
```

VTref는 일반적으로 타깃 전압 감지용이며, ST-LINK 모델에 따라 전원 공급 기능은 다릅니다.

---

## 21. JTAG / SWD

JTAG과 SWD는 MCU의 디버깅 및 프로그래밍에 사용되는 인터페이스입니다.

### JTAG

- TMS
- TCK
- TDI
- TDO
- 선택적으로 TRST

### SWD

- SWDIO
- SWCLK

STM32 개발에서는 핀 사용량이 적은 SWD 인터페이스를 자주 사용합니다.

---

# MCU 레지스터 이해

## 22. 레지스터란?

레지스터(Register)는 MCU 내부의 하드웨어 기능을 제어하거나 상태 및 데이터를 읽기 위해 사용하는 저장 공간입니다.

STM32 주변장치 레지스터는 일반적으로 메모리 주소에 매핑되어 있습니다.

이를 Memory-Mapped I/O라고 합니다.

### 주요 레지스터 종류

| 종류 | 역할 | 예시 |
|---|---|---|
| Control Register | 하드웨어 동작 설정 | GPIO_MODER, USART_CR1 |
| Status Register | 하드웨어 상태 확인 | ADC_SR, TIM_SR |
| Data Register | 데이터 읽기/쓰기 | GPIO_IDR, GPIO_ODR, ADC_DR |

### Control Register

주요 설정 기능:

- 클럭 활성화
- 통신속도 설정
- 인터럽트 활성화
- 동작 모드 설정
- 하드웨어 기능 활성화

### Status Register

주요 상태 확인 기능:

- 통신 완료 여부
- ADC 변환 완료 여부
- 클럭 준비 완료 여부
- 인터럽트 이벤트 발생 여부

### Data Register

주요 데이터 처리 기능:

- ADC 변환 데이터 읽기
- UART 수신 데이터 읽기
- UART 송신 데이터 쓰기
- GPIO 입력/출력 데이터 처리

---

## 23. 레지스터 비트 제어

레지스터 직접 제어에서는 비트 연산을 이해하는 것이 중요합니다.

### 특정 비트 SET

```c
REG |= (1U << 5);
```

5번 비트를 1로 설정하고 나머지 비트는 유지합니다.

### 특정 비트 CLEAR

```c
REG &= ~(1U << 5);
```

5번 비트를 0으로 설정하고 나머지 비트는 유지합니다.

### 특정 비트 TOGGLE

```c
REG ^= (1U << 5);
```

5번 비트 값을 반전합니다.

### 특정 비트 READ

```c
if (REG & (1U << 5))
{
    // 5번 비트가 1일 경우
}
```

> 주의: 모든 레지스터에 일반적인 Read-Modify-Write 연산을 적용할 수 있는 것은 아닙니다. Write-only 레지스터와 W1C(Write 1 to Clear) 방식의 상태 레지스터는 데이터시트 및 Reference Manual에 정의된 방법대로 접근해야 합니다.

---

## 24. 레지스터 직접 접근 방식

### CMSIS 구조체 접근

```c
RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

GPIOA->MODER &= ~(3U << 10);
GPIOA->MODER |=  (1U << 10);

GPIOA->BSRR = (1U << 5);
```

### 주소 기반 직접 접근

STM32F446의 GPIOA 클럭 활성화 예시:

```c
#include <stdint.h>

#define RCC_BASE              0x40023800UL
#define RCC_AHB1ENR_OFFSET    0x30UL

#define RCC_AHB1ENR \
    (*(volatile uint32_t *)(RCC_BASE + RCC_AHB1ENR_OFFSET))

int main(void)
{
    // GPIOAEN = bit0 = 1
    RCC_AHB1ENR |= (1U << 0);

    while (1)
    {
    }
}
```

### volatile 키워드

`volatile`은 해당 메모리 값이 프로그램의 일반적인 흐름 외부에서 변경될 수 있음을 컴파일러에 알리는 키워드입니다.

하드웨어 레지스터처럼 읽기/쓰기 자체가 의미 있는 메모리에 사용합니다.

---

## 25. STM32 레지스터 제어 기본 순서

STM32 주변장치를 제어할 때 일반적으로 다음 순서를 따릅니다.

```text
1. RCC 클럭 활성화
          |
          v
2. GPIO 핀 모드 설정
          |
          v
3. 주변장치 레지스터 설정
          |
          v
4. 주변장치 활성화
          |
          v
5. 데이터 읽기 / 쓰기
          |
          v
6. 상태 확인 / 인터럽트 처리
```

주변장치 종류에 따라 실제 초기화 순서와 필요한 추가 설정은 달라집니다.

---

## 26. F767 학습 소스

현재 프로젝트에는 STM32F446 예제 외에 STM32F767 학습을 위한 소스도 포함되어 있습니다.

| 파일 | 설명 |
|---|---|
| `interrupt.c` | 인터럽트 학습 |
| `timer_counter.c` | 타이머 및 카운터 학습 |
| `urtc.c` | F767 관련 학습 소스 |
| `throttle.c` | 스로틀 관련 초기화 코드 |
| `clock.c` | 클럭 초기화 코드 |
| `register_offset.c` | 레지스터 주소 및 구조체 정의 |

`clock.c`와 `throttle.c`는 독립적인 `main()` 함수가 없는 초기화 코드입니다.

F767 코드를 빌드하려면 다음 항목을 별도로 설정해야 합니다.

- STM32CubeF7 CMSIS 헤더
- Cortex-M7 컴파일 옵션
- STM32F767 Startup Code
- F767 메모리 구성에 맞는 Linker Script
- 해당 보드에 맞는 GPIO 및 주변장치 설정

> STM32F446과 STM32F767은 CPU 코어, 클럭 구조, 주변장치 레지스터 및 메모리 구성이 다르므로 소스코드를 그대로 호환된다고 가정하면 안 됩니다.



---

## 29. 참고 자료

- [STMicroelectronics 공식 홈페이지](https://www.st.com/)
- [STM32F446 제품 정보 및 데이터시트](https://www.st.com/en/microcontrollers-microprocessors/stm32f446.html)
- [STM32F767 제품 정보 및 데이터시트](https://www.st.com/en/microcontrollers-microprocessors/stm32f767zi.html)
- [Arm CMSIS 공식 문서](https://www.arm.com/technologies/cmsis)
- [STM32CubeF4](https://github.com/STMicroelectronics/STM32CubeF4)
- [STM32CubeF7](https://github.com/STMicroelectronics/STM32CubeF7)

---


## STM32 Env
- [개발환경](./docs/stm32_env.md)