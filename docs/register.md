
# STM32 USART 레지스터 정리

## 1. USART 주요 레지스터

USART(Universal Synchronous/Asynchronous Receiver/Transmitter)는 MCU에서 데이터를 송수신하는 주변장치이다.

| 레지스터 | 영어 원문 | 역할 |
|---|---|---|
| ISR | Interrupt and Status Register | 인터럽트 및 상태 확인 |
| RDR | Receive Data Register | 수신 데이터 읽기 |
| TDR | Transmit Data Register | 송신 데이터 쓰기 |
| SR | Status Register | 송수신 상태 확인 |
| DR | Data Register | 송수신 데이터 접근 |
| BRR | Baud Rate Register | 통신 속도 설정 |
| CR1 | Control Register 1 | USART 활성화, 송수신 활성화 |
| CR2 | Control Register 2 | 정지 비트 등 설정 |
| CR3 | Control Register 3 | DMA, 흐름 제어 등 설정 |

## 2. STM32 모델별 차이

| 기능 | STM32F446RE | STM32F767 |
|---|---|---|
| CPU | Cortex-M4 | Cortex-M7 |
| 상태 확인 | SR | ISR |
| 데이터 송신 | DR | TDR |
| 데이터 수신 | DR | RDR |
| USART 활성화 | CR1 bit 13 | CR1 bit 0 |

※ CPU 코어 자체의 차이가 아니라 USART 주변장치의 레지스터 설계 차이이다.

## 3. 주요 상태 비트

### TXE (Transmit Data Register Empty)
- 송신 데이터 레지스터가 비어 있음을 의미한다.
- TXE = 1이면 다음 데이터를 기록할 수 있다.
- STM32F446RE: SR bit 7

### RXNE (Read Data Register Not Empty)
- 수신 데이터 레지스터에 읽을 데이터가 존재한다.
- RXNE = 1이면 수신 데이터를 읽을 수 있다.
- STM32F446RE: SR bit 5

### UE (USART Enable)
- USART 주변장치를 활성화하는 비트이다.
- STM32F446RE: CR1 bit 13
- STM32F767: CR1 bit 0

### TE / RE
- TE (Transmitter Enable): 송신 활성화
- RE (Receiver Enable): 수신 활성화
- 두 모델 모두 CR1의 bit 3과 bit 2

## 4. STM32F446RE 코드 예시

### 송신

```c
while ((USART2_SR & (1U << 7)) == 0U)
    ;

USART2_DR = 'A';
```

- SR의 TXE 비트 확인
- DR에 'A'를 기록하여 송신

### 수신

```c
while ((USART2_SR & (1U << 5)) == 0U)
    ;

char c = (char)(USART2_DR & 0xFFU);
```

- SR의 RXNE 비트 확인
- DR에서 수신 문자 읽기

## 5. 핵심 정리

- SR / ISR: USART 상태 확인
- DR: 송신과 수신을 하나의 데이터 레지스터로 처리
- RDR: 수신 전용 데이터 레지스터
- TDR: 송신 전용 데이터 레지스터
- BRR: Baudrate 설정
- CR1 / CR2 / CR3: USART 동작 설정
- TXE: 다음 데이터 송신 가능 여부
- RXNE: 수신 데이터 존재 여부
- UE: USART 활성화

## 6. 공식 문서

- STM32F446RE: RM0390 Reference Manual
- STM32F767: RM0410 Reference Manual

레지스터를 직접 제어할 때는 해당 MCU의 Reference Manual을 확인해야 한다.
