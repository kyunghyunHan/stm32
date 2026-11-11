# 전체 펌웨어 파일 분석

검토 기준: 로컬 Git 시작점 `e5dec91`, 2026-11-11. 파일 이름 대신 실행 코드와 설정으로 분류했습니다.

사용자 확인에 따라 기존 대상 미확정 코드와 F439ZI로 잘못 표기된 코드를 모두 F429ZI로 통합했습니다. Rust는 경로의 rust와 구현 설명으로 Register/HAL을 구분합니다.

모든 원본 파일의 이동 및 체크섬은 [file_moves.json](file_moves.json), 기계 판독용 분석은 [examples.json](examples.json)에 있습니다.

아래 PASS는 컴파일·링크만 의미합니다. 모든 펌웨어의 보드 실행 검증은 **HARDWARE REQUIRED**입니다.

## `src/f446re/register/gpio_led_blink.c`

- 이전 경로: `src/register/led.c`
- MCU / 언어 / 방식: f446re / C / register
- 기능: SysTick 폴링으로 3초마다 PA5 반전
- 핀/AF: PA5
- 클럭: HSI 16 MHz 가정
- 직접 의존성: `stdint.h`
- 빌드: PASS; 하드웨어: HARDWARE REQUIRED
- 비고/개선: 기존 F767 빌드도 이 소스를 사용함. 사용한 RCC/GPIO 주소는 F767 CMSIS와도 일치; PA5 실제 LED 배선 확인 필요

## `src/f429zi/register/gpio_button_toggle.c`

- 이전 경로: `src/register/led_button.c`
- MCU / 언어 / 방식: f429zi / C / register
- 기능: PC13 하강 에지에서 PF15 반전, 디바운스 없음
- 핀/AF: PC13, PF15
- 클럭: HSI 16 MHz 가정
- 직접 의존성: `stdint.h`
- 빌드: PASS (F429ZI); 하드웨어: HARDWARE REQUIRED
- 비고/개선: 기존 F446/F767 빌드 보존; F446RE에 PF15 없음. F767의 사용한 GPIO 주소는 CMSIS와 일치하지만 PF15 패키지/배선 확인 필요

## `src/f767vi/register/adc_polling.c`

- 이전 경로: `src/register/adc.c`
- MCU / 언어 / 방식: f767vi / C / register
- 기능: ADC1 채널7 단발 변환 및 전압 환산
- 핀/AF: PA7
- 클럭: HSE/PLL, M=8 N=216 P=2; HSE 16 MHz일 때 SYSCLK 216 MHz; APB2=108 MHz, ADC /6=18 MHz (HSE 16 MHz 조건)
- 직접 의존성: `stm32f7xx.h`
- 빌드: PASS; 하드웨어: HARDWARE REQUIRED
- 비고/개선: 소스 주석은 F767VG, 기존 빌드는 VI; 외부 발진기와 전압 조건 확인 필요

## `src/f767vi/register/exti_button_led_toggle.c`

- 이전 경로: `src/register/interrupt.c`
- MCU / 언어 / 방식: f767vi / C / register
- 기능: EXTI4 하강 에지마다 PC6 반전
- 핀/AF: PD4, PC6
- 클럭: HSE/PLL, M=8 N=216 P=2; HSE 16 MHz일 때 SYSCLK 216 MHz
- 직접 의존성: `stm32f7xx.h`
- 빌드: PASS; 하드웨어: HARDWARE REQUIRED
- 비고/개선: 소스 주석은 F767VG, 기존 빌드는 VI; 외부 발진기와 전압 조건 확인 필요

## `src/f767vi/register/timer_interrupt_led_toggle.c`

- 이전 경로: `src/register/timer_counter.c`
- MCU / 언어 / 방식: f767vi / C / register
- 기능: TIM1 update 인터럽트마다 PC6 반전
- 핀/AF: PC6
- 클럭: HSE/PLL, M=8 N=216 P=2; HSE 16 MHz일 때 SYSCLK 216 MHz
- 직접 의존성: `stm32f7xx.h`
- 빌드: PASS; 하드웨어: HARDWARE REQUIRED
- 비고/개선: 소스 주석은 F767VG, 기존 빌드는 VI; 외부 발진기와 전압 조건 확인 필요

## `src/f767vi/register/uart_rx_increment_tx_polling.c`

- 이전 경로: `src/register/urtc.c`
- MCU / 언어 / 방식: f767vi / C / register
- 기능: USART2 수신 바이트+1 송신
- 핀/AF: PD5 TX / PD6 RX AF7
- 클럭: HSE/PLL, M=8 N=216 P=2; HSE 16 MHz일 때 SYSCLK 216 MHz; APB1=54 MHz, BRR=5625 → 9600 baud (HSE 16 MHz 조건)
- 직접 의존성: `stm32f7xx.h`
- 빌드: PASS; 하드웨어: HARDWARE REQUIRED
- 비고/개선: 소스 주석은 F767VG, 기존 빌드는 VI; 외부 발진기와 전압 조건 확인 필요

## `src/f767vi/register/clock_pll_init.c`

- 이전 경로: `src/register/clock.c`
- MCU / 언어 / 방식: f767vi / C / register
- 기능: Initialize_MCU 초기화 함수, main 없음
- 핀/AF: GPIO 없음
- 클럭: HSE/PLL M=8 N=216 P=2
- 직접 의존성: Rust 의존성은 Cargo.toml; 조각/메모는 내용 참조
- 빌드: NOT TESTED; 하드웨어: HARDWARE REQUIRED
- 비고/개선: 헤더 강제 include가 필요한 학습 조각

## `src/f429zi/register/gpio_register_layout.c`

- 이전 경로: `src/register/register_offset.c`
- MCU / 언어 / 방식: f429zi / C / register
- 기능: 주소 매크로 및 GPIO 구조체 정의; main 없음
- 핀/AF: GPIOD (핀 사용 없음)
- 클럭: HSI 16 MHz 가정
- 직접 의존성: `stdint.h`
- 빌드: NOT TESTED; 하드웨어: HARDWARE REQUIRED
- 비고/개선: RCC_AHB1ENR_OFFSET 정의 누락; 매크로 미사용이라 구문 검사로 검출 안 됨

## `src/f429zi/register/gpio_external_button_led.c`

- 이전 경로: `src/register/ext_led_ext_button.c`
- MCU / 언어 / 방식: f429zi / C / register
- 기능: PA10 LOW이면 PA8 HIGH
- 핀/AF: PA10 pull-up / PA8
- 클럭: HSI 16 MHz 가정
- 직접 의존성: `stdint.h`
- 빌드: PASS (F429ZI); 하드웨어: HARDWARE REQUIRED
- 비고/개선: 사용자 확인에 따라 F429ZI로 분류; 실제 배선 확인 필요

## `src/f429zi/register/gpio_pc13_external_led.c`

- 이전 경로: `src/register/ext_led_int_button.c`
- MCU / 언어 / 방식: f429zi / C / register
- 기능: PC13 LOW이면 PA8 HIGH
- 핀/AF: PC13 / PA8
- 클럭: HSI 16 MHz 가정
- 직접 의존성: `stdint.h`
- 빌드: PASS (F429ZI); 하드웨어: HARDWARE REQUIRED
- 비고/개선: 사용자 확인에 따라 F429ZI로 분류; 실제 배선 확인 필요

## `src/f429zi/register/gpio_button_toggle_open_drain.c`

- 이전 경로: `src/register/inpiut.c`
- MCU / 언어 / 방식: f429zi / C / register
- 기능: PF14 하강 에지마다 PF15 open-drain 반전
- 핀/AF: PF14 pull-up / PF15
- 클럭: HSI 16 MHz 가정
- 직접 의존성: `stdint.h`
- 빌드: PASS (F429ZI); 하드웨어: HARDWARE REQUIRED
- 비고/개선: 사용자 확인에 따라 F429ZI로 분류; 실제 배선 확인 필요

## `src/f429zi/register/gpio_pc13_toggle_open_drain.c`

- 이전 경로: `src/register/led_button_ot.c`
- MCU / 언어 / 방식: f429zi / C / register
- 기능: PC13 하강 에지마다 PF15 open-drain 반전
- 핀/AF: PC13 / PF15
- 클럭: HSI 16 MHz 가정
- 직접 의존성: `stdint.h`
- 빌드: PASS (F429ZI); 하드웨어: HARDWARE REQUIRED
- 비고/개선: 사용자 확인에 따라 F429ZI로 분류; 실제 배선 확인 필요

## `src/f429zi/register/gpio_button_event_counter.c`

- 이전 경로: `src/register/3led_button.c`
- MCU / 언어 / 방식: f429zi / C / register
- 기능: 버튼 이벤트 큐, 3비트 LED 카운터 및 외부 입력 출력
- 핀/AF: PC13, PB0/PB7/PB14, PF15 입력 / PF13 출력
- 클럭: HSI 16 MHz 가정
- 직접 의존성: `stdint.h`, `stdbool.h`
- 빌드: PASS; 하드웨어: HARDWARE REQUIRED
- 비고/개선: 실제 보드의 핀 배선·입력 극성 확인 필요

## `src/f429zi/register/gpio_button_event_counter_variant.c`

- 이전 경로: `src/register/test.c`
- MCU / 언어 / 방식: f429zi / C / register
- 기능: 버튼 이벤트 큐, 3비트 LED 카운터 및 외부 입력 출력 별도 실습
- 핀/AF: PC13, PB0/PB7/PB14, PF15 입력 / PF13 출력
- 클럭: HSI 16 MHz 가정
- 직접 의존성: `stdint.h`, `stdbool.h`
- 빌드: PASS; 하드웨어: HARDWARE REQUIRED
- 비고/개선: 실제 보드의 핀 배선·입력 극성 확인 필요

## `src/f429zi/register/exti_both_edges_led.c`

- 이전 경로: `src/register/exti_button_edge.c`
- MCU / 언어 / 방식: f429zi / C / register
- 기능: PC13 양 에지 인터럽트에서 PB7이 입력 상태를 반영
- 핀/AF: PC13 / PB7
- 클럭: HSI 16 MHz 가정
- 직접 의존성: `stdint.h`
- 빌드: PASS; 하드웨어: HARDWARE REQUIRED
- 비고/개선: 실제 보드의 핀 배선·입력 극성 확인 필요

## `src/f429zi/register/exti_rising_led_toggle.c`

- 이전 경로: `src/register/interrupt_int_led.c`
- MCU / 언어 / 방식: f429zi / C / register
- 기능: PC13 상승 인터럽트에서 PB7 반전
- 핀/AF: PC13 / PB7
- 클럭: HSI 16 MHz 가정
- 직접 의존성: `stdint.h`
- 빌드: PASS (F429ZI); 하드웨어: HARDWARE REQUIRED
- 비고/개선: F429ZI의 PC13 상승 에지/PB7 반전 실습

## `src/f429zi/register/gpio_led_morse_sos.c`

- 이전 경로: `src/register/sos_led.c`
- MCU / 언어 / 방식: f429zi / C / register
- 기능: PB7 LED로 SOS 반복
- 핀/AF: PB7
- 클럭: HSI 16 MHz 가정
- 직접 의존성: `stdint.h`
- 빌드: PASS; 하드웨어: HARDWARE REQUIRED
- 비고/개선: 실제 보드의 핀 배선·입력 극성 확인 필요

## `src/f429zi/register/timer_polling_uart_tx.c`

- 이전 경로: `src/register/tim_poling_int_led.c`
- MCU / 언어 / 방식: f429zi / C / register
- 기능: TIM2 1초 폴링마다 USART3 점 송신
- 핀/AF: PD8 TX AF7
- 클럭: HSI 16 MHz 가정
- 직접 의존성: `stdint.h`
- 빌드: PASS; 하드웨어: HARDWARE REQUIRED
- 비고/개선: USART3 BRR=139, APB1 16 MHz에서 약 115108 baud (115200 설정용)

## `src/f429zi/register/uart_heap_memory_dump.c`

- 이전 경로: `src/register/uart_print.c`
- MCU / 언어 / 방식: f429zi / C / register
- 기능: calloc/free 주소와 고정 RAM 덤프를 USART3로 출력
- 핀/AF: PD8 TX / PD9 RX AF7
- 클럭: HSI 16 MHz 가정
- 직접 의존성: `stdint.h`, `stdlib.h`, `stdio.h`, `stdarg.h`, `inttypes.h`
- 빌드: PASS; 하드웨어: HARDWARE REQUIRED
- 비고/개선: APB 16 MHz, BRR=1667 → 약 9598 baud (9600 설정용)

## `src/f446re/register/systick_interrupt_led_blink.c`

- 이전 경로: `src/register/led_interrupt.c`
- MCU / 언어 / 방식: f446re / C / register
- 기능: SysTick 1ms IRQ 누적, 3초마다 PA5 반전
- 핀/AF: PA5
- 클럭: HSI 16 MHz 가정
- 직접 의존성: `stdint.h`
- 빌드: PASS; 하드웨어: HARDWARE REQUIRED
- 비고/개선: 실제 보드의 핀 배선·입력 극성 확인 필요

## `src/f446re/register/timer_polling_led_blink.c`

- 이전 경로: `src/register/timer_led.c`
- MCU / 언어 / 방식: f446re / C / register
- 기능: TIM2 PSC=15999 ARR=999; 1초 update마다 PA5 반전
- 핀/AF: PA5
- 클럭: HSI 16 MHz 가정
- 직접 의존성: `stdint.h`
- 빌드: PASS; 하드웨어: HARDWARE REQUIRED
- 비고/개선: 실제 보드의 핀 배선·입력 극성 확인 필요

## `src/f446re/register/i2c_oled_hello.c`

- 이전 경로: `src/register/oled_hello.c`
- MCU / 언어 / 방식: f446re / C / register
- 기능: SSD1306 주소 0x3c, HELLO STM32 표시 및 UART 진단
- 핀/AF: PB8 SCL / PB9 SDA AF4, PA2 TX AF7
- 클럭: HSI 16 MHz 가정
- 직접 의존성: `stdint.h`
- 빌드: PASS; 하드웨어: HARDWARE REQUIRED
- 비고/개선: 실제 보드의 핀 배선·입력 극성 확인 필요

## `src/f446re/register/uart_rx_external_led.c`

- 이전 경로: `src/register/uart_ext_led.c`
- MCU / 언어 / 방식: f446re / C / register
- 기능: USART2 1/0 명령으로 외부 LED 제어
- 핀/AF: PA2/PA3 AF7 / PA8
- 클럭: HSI 16 MHz 가정
- 직접 의존성: `stdint.h`
- 빌드: PASS; 하드웨어: HARDWARE REQUIRED
- 비고/개선: APB 16 MHz, BRR=1667 → 약 9598 baud (9600 설정용)

## `src/f446re/register/uart_button_level_tx.c`

- 이전 경로: `src/register/uart_gpio_input.c`
- MCU / 언어 / 방식: f446re / C / register
- 기능: PC13 변화 시 0/1 CRLF 송신
- 핀/AF: PC13 / PA2 TX AF7
- 클럭: HSI 16 MHz 가정
- 직접 의존성: `stdint.h`
- 빌드: PASS; 하드웨어: HARDWARE REQUIRED
- 비고/개선: APB 16 MHz, BRR=1667 → 약 9598 baud (9600 설정용)

## `src/f446re/register/uart_rx_increment_tx_polling.c`

- 이전 경로: `src/register/urtc2.c`
- MCU / 언어 / 방식: f446re / C / register
- 기능: USART2 수신 바이트+1 송신
- 핀/AF: PA2 TX / PA3 RX AF7
- 클럭: HSI 16 MHz 가정
- 직접 의존성: `stdint.h`
- 빌드: PASS; 하드웨어: HARDWARE REQUIRED
- 비고/개선: APB 16 MHz, BRR=1667 → 약 9598 baud (9600 설정용)

## `src/f446re/register/uart_rx_led_control.c`

- 이전 경로: `src/register/uart_led.c`
- MCU / 언어 / 방식: f446re / C / register
- 기능: USART2 1/0 명령으로 PA5 제어 의도
- 핀/AF: PA2/PA3 AF7 / PA5
- 클럭: HSI 16 MHz 가정
- 직접 의존성: `stdint.h`
- 빌드: PASS; 하드웨어: HARDWARE REQUIRED
- 비고/개선: RXNE 대기 괄호 오류 유지; APB 16 MHz, BRR=1667 → 약 9598 baud (9600 설정용)

## `src/f446re/register/uart_rx_led_control_variant.c`

- 이전 경로: `src/register/uart_systick.c`
- MCU / 언어 / 방식: f446re / C / register
- 기능: uart_led.c와 동일한 학습 소스
- 핀/AF: PA2/PA3 AF7 / PA5
- 클럭: HSI 16 MHz 가정
- 직접 의존성: `stdint.h`
- 빌드: PASS; 하드웨어: HARDWARE REQUIRED
- 비고/개선: RXNE 대기 괄호 오류 유지; APB 16 MHz, BRR=1667 → 약 9598 baud (9600 설정용)

## `src/f429zi/register/uart_rx_three_led_select.c`

- 이전 경로: `src/register/uart_ext_3led.c`
- MCU / 언어 / 방식: f429zi / C / register
- 기능: 0/1/2 선택 출력, 3 전체 끄기
- 핀/AF: PA3 RX AF7, PA8/PA9/PC7 LED
- 클럭: HSI 16 MHz 가정
- 직접 의존성: `stdint.h`
- 빌드: PASS (F429ZI); 하드웨어: HARDWARE REQUIRED
- 비고/개선: APB 16 MHz, BRR=1667 → 약 9598 baud (9600 설정용)

## `src/f429zi/register/uart_external_button_tx.c`

- 이전 경로: `src/register/uart_ext_button.c`
- MCU / 언어 / 방식: f429zi / C / register
- 기능: PA10 상승 에지에서 Button Pressed 송신
- 핀/AF: PA10 pull-down / PA2 TX AF7
- 클럭: HSI 16 MHz 가정
- 직접 의존성: `stdint.h`
- 빌드: PASS (F429ZI); 하드웨어: HARDWARE REQUIRED
- 비고/개선: APB 16 MHz, BRR=1667 → 약 9598 baud (9600 설정용)

## `src/f429zi/register/uart_button_led_report.c`

- 이전 경로: `src/register/uart_ext_led_ext_button.c`
- MCU / 언어 / 방식: f429zi / C / register
- 기능: 버튼 상태를 LED에 출력, 상승 시 UART 보고
- 핀/AF: PA10 pull-down / PA8 LED / PA2 TX AF7
- 클럭: HSI 16 MHz 가정
- 직접 의존성: `stdint.h`
- 빌드: PASS (F429ZI); 하드웨어: HARDWARE REQUIRED
- 비고/개선: APB 16 MHz, BRR=1667 → 약 9598 baud (9600 설정용)

## `notes/gpio_pullup_open_drain_circuits.c`

- 이전 경로: `src/register/ii.c`
- MCU / 언어 / 방식: f429zi (주석) / C / Register draft
- 기능: 전체가 주석인 GPIO pull-up/pull-down, push-pull/open-drain 회로 학습 메모
- 핀/AF: 주석 예시: PF14/PF15
- 클럭: 미설정
- 직접 의존성: Rust 의존성은 Cargo.toml; 조각/메모는 내용 참조
- 빌드: NOT TESTED; 하드웨어: HARDWARE REQUIRED
- 비고/개선: 독립 실행 불가

## `notes/adc_throttle_draft.c`

- 이전 경로: `src/register/throttle.c`
- MCU / 언어 / 방식: unassigned / C / Register draft
- 기능: RCC_BASE 빈 매크로만 존재
- 핀/AF: 해당 없음
- 클럭: 미설정
- 직접 의존성: Rust 의존성은 Cargo.toml; 조각/메모는 내용 참조
- 빌드: NOT TESTED; 하드웨어: HARDWARE REQUIRED
- 비고/개선: 독립 실행 불가

## `src/f767vi/hal/gpio_led_blink.c`

- 이전 경로: `src/hal/led.c`
- MCU / 언어 / 방식: f767vi / C / hal
- 기능: HAL_Delay 3초마다 PA5 반전
- 핀/AF: PA5
- 클럭: HSI 16 MHz
- 직접 의존성: `stm32f7xx_hal.h`
- 빌드: PASS; 하드웨어: HARDWARE REQUIRED
- 비고/개선: 실제 보드의 핀 배선·입력 극성 확인 필요

## `src/f767vi/hal/adc_polling.c`

- 이전 경로: `src/hal/adc.c`
- MCU / 언어 / 방식: f767vi / C / hal
- 기능: ADC1 채널7 56 cycles 샘플링, 전압 환산
- 핀/AF: PA7
- 클럭: HSI 16 MHz; ADC clock 8 MHz
- 직접 의존성: `stm32f7xx_hal.h`
- 빌드: PASS; 하드웨어: HARDWARE REQUIRED
- 비고/개선: 실제 보드의 핀 배선·입력 극성 확인 필요

## `src/f446re/register/uart_command_error_counters.c`

- 이전 경로: `src/register/uart_led_error.c`
- MCU / 언어 / 방식: f446re / C / register
- 기능: 라인 명령으로 LED 3개 제어, 버튼 디바운스 및 UART 오류 카운터
- 핀/AF: PA2/PA3 AF7, PA5/PB0/PB1, PC13
- 클럭: HSI 16 MHz
- 직접 의존성: `stdint.h`
- 빌드: PASS; 하드웨어: HARDWARE REQUIRED
- 비고/개선: UART 9600 8N1; PC13 active-high 해석은 실제 회로 확인 필요; APB 16 MHz, BRR=1667 → 약 9598 baud (9600 설정용)

## `src/f446re/hal/uart_command_error_counters.c`

- 이전 경로: `src/hal/uart_led_error.c`
- MCU / 언어 / 방식: f446re / C / hal
- 기능: 라인 명령으로 LED 3개 제어, 버튼 디바운스 및 UART 오류 카운터
- 핀/AF: PA2/PA3 AF7, PA5/PB0/PB1, PC13
- 클럭: HSI 16 MHz
- 직접 의존성: `stm32f4xx_hal.h`, `stdint.h`
- 빌드: PASS; 하드웨어: HARDWARE REQUIRED
- 비고/개선: UART 9600 8N1; PC13 active-high 해석은 실제 회로 확인 필요

## `src/f446re/register/uart_button_led_sync.c`

- 이전 경로: `src/register/u_to_u/STM32F446RE_LED.c`
- MCU / 언어 / 방식: f446re / C / register
- 기능: USART1으로 버튼 상태 교환, 20ms 표본 디바운스
- 핀/AF: PA9 TX / PA10 RX AF7, PC13 / PA5
- 클럭: HSI 16 MHz 가정
- 직접 의존성: `stdint.h`
- 빌드: PASS; 하드웨어: HARDWARE REQUIRED
- 비고/개선: APB 16 MHz, BRR=1667 → 약 9598 baud (9600 설정용)

## `src/f429zi/register/uart_button_led_sync.c`

- 이전 경로: `src/register/u_to_u/STM32F439ZI_LED.c`
- MCU / 언어 / 방식: f429zi / C / register
- 기능: USART1으로 버튼 상태 교환, 20ms 표본 디바운스
- 핀/AF: PA9 TX / PA10 RX AF7, PC13 / PB7
- 클럭: HSI 16 MHz 가정
- 직접 의존성: `stdint.h`
- 빌드: PASS (F429ZI); 하드웨어: HARDWARE REQUIRED
- 비고/개선: 사용자 확인에 따라 F429ZI로 정정; APB 16 MHz, BRR=1667 → 약 9598 baud (9600 설정용)

## `src/f446re/register/uart_button_led_sync_blocking.c`

- 이전 경로: `src/register/u_to_u/stm32_uart.c`
- MCU / 언어 / 방식: f446re / C / register
- 기능: USART1 버튼 전송 / LED 수신, blocking 20ms 디바운스
- 핀/AF: PA9 TX / PA10 RX AF7, PC13 / PA5
- 클럭: HSI 16 MHz 가정
- 직접 의존성: `stdint.h`
- 빌드: PASS; 하드웨어: HARDWARE REQUIRED
- 비고/개선: APB 16 MHz, BRR=1667 → 약 9598 baud (9600 설정용)

## `src/f429zi/register/uart6_button_led_sync.c`

- 이전 경로: `src/register/u_to_u/stm32_uart2.c`
- MCU / 언어 / 방식: f429zi / C / register
- 기능: USART6 버튼 전송 / LED 수신
- 핀/AF: PG14 TX / PG9 RX AF8, PC13 / PB7
- 클럭: HSI 16 MHz 가정
- 직접 의존성: `stdint.h`
- 빌드: PASS; 하드웨어: HARDWARE REQUIRED
- 비고/개선: APB 16 MHz, BRR=1667 → 약 9598 baud (9600 설정용)

## `src/f446re/register/uart_three_node_hub.c`

- 이전 경로: `src/register/u_to_u3/test.c`
- MCU / 언어 / 방식: f446re / C / register
- 기능: F429/ESP32 상태 OR 결합; 20ms debounce / 200ms 재전송
- 핀/AF: PA9/PA10 AF7 USART1, PA0/PA1 AF8 UART4, PC13 / PA5
- 클럭: HSI 16 MHz 가정
- 직접 의존성: `stdint.h`
- 빌드: PASS; 하드웨어: HARDWARE REQUIRED
- 비고/개선: APB 16 MHz, BRR=1667 → 약 9598 baud (9600 설정용)

## `src/f429zi/register/can_loopback_led.c`

- 이전 경로: `src/can_int_led.c`
- MCU / 언어 / 방식: f429zi / C / register
- 기능: CAN 내부 loopback ID 0x123, 데이터 일치 시 PB7 HIGH
- 핀/AF: PB7; CAN 외부 핀 없음
- 클럭: HSI 16 MHz 가정
- 직접 의존성: `stdint.h`
- 빌드: PASS (F429ZI); 하드웨어: HARDWARE REQUIRED
- 비고/개선: CAN sleep 해제 확인 필요; timeout 없음

## `src/f446re/register/can_silent_loopback.c`

- 이전 경로: `src/register/can_f446.c`
- MCU / 언어 / 방식: f446re / C / register
- 기능: CAN silent loopback ID 0x100, SOC=70 TEMP=30, debugger 변수 확인
- 핀/AF: 외부 CAN 핀 없음
- 클럭: HSI 16 MHz 가정
- 직접 의존성: `stm32f446xx.h`, `stdint.h`
- 빌드: PASS; 하드웨어: HARDWARE REQUIRED
- 비고/개선: 실제 보드의 핀 배선·입력 극성 확인 필요

## `src/f446re/rust/gpio_led_blink.rs`

- 이전 경로: `src/register/led.rs`
- MCU / 언어 / 방식: f446re / Rust / Rust/Register
- 기능: SysTick 폴링 3초 PA5 반전
- 핀/AF: PA5
- 클럭: HSI 16 MHz 가정
- 직접 의존성: Rust 의존성은 Cargo.toml; 조각/메모는 내용 참조
- 빌드: NOT TESTED; 하드웨어: HARDWARE REQUIRED
- 비고/개선: Cargo 미등록; panic_halt 의존성 없음

## `src/f446re/rust/gpio_button_led_level.rs`

- 이전 경로: `src/register/led_button.rs`
- MCU / 언어 / 방식: f446re / Rust / Rust/Register
- 기능: PC13 LOW이면 PA5 HIGH
- 핀/AF: PC13 / PA5
- 클럭: HSI 16 MHz 가정
- 직접 의존성: Rust 의존성은 Cargo.toml; 조각/메모는 내용 참조
- 빌드: PASS; 하드웨어: HARDWARE REQUIRED
- 비고/개선: 실제 보드의 핀 배선·입력 극성 확인 필요

## `src/f446re/rust/uart_rx_led_control.rs`

- 이전 경로: `src/register/uart_led.rs`
- MCU / 언어 / 방식: f446re / Rust / Rust/Register
- 기능: USART2 1/0로 PA5 출력
- 핀/AF: PA2/PA3 AF7 / PA5
- 클럭: HSI 16 MHz 가정
- 직접 의존성: Rust 의존성은 Cargo.toml; 조각/메모는 내용 참조
- 빌드: PASS; 하드웨어: HARDWARE REQUIRED
- 비고/개선: 실제 보드의 핀 배선·입력 극성 확인 필요

## `src/f446re/rust/uart_echo_greeting.rs`

- 이전 경로: `src/register/uart.rs`
- MCU / 언어 / 방식: f446re / Rust / Rust/Register
- 기능: USART3 Hello World 후 echo
- 핀/AF: PD8/PD9 AF7
- 클럭: HSI 16 MHz 가정
- 직접 의존성: Rust 의존성은 Cargo.toml; 조각/메모는 내용 참조
- 빌드: PASS; 하드웨어: HARDWARE REQUIRED
- 비고/개선: 주석 F429ZI와 Cargo F446 feature 불일치

## `src/f767vi/rust/rust_hal_gpio_led_blink.rs`

- 이전 경로: `src/hal/led.rs`
- MCU / 언어 / 방식: f767vi / Rust / Rust/HAL
- 기능: HAL GPIO PA5 blink 의도
- 핀/AF: PA5
- 클럭: HSI 16 MHz 가정
- 직접 의존성: Rust 의존성은 Cargo.toml; 조각/메모는 내용 참조
- 빌드: NOT TESTED; 하드웨어: HARDWARE REQUIRED
- 비고/개선: Cargo 미등록; stm32f7xx_hal/panic_halt 의존성 없음

## `src/firmware.rs`

- 이전 경로: `src/firmware.rs`
- MCU / 언어 / 방식: f446re/f767vi / Rust / Rust/PAC
- 기능: PAC take 후 nop 무한 루프; 주변장치 예제 아님
- 핀/AF: 없음
- 클럭: 리셋 클럭
- 직접 의존성: Rust 의존성은 Cargo.toml; 조각/메모는 내용 참조
- 빌드: PASS; 하드웨어: HARDWARE REQUIRED
- 비고/개선: 실제 보드의 핀 배선·입력 극성 확인 필요

## `src/f429zi/register/gpio_led_blink.c`

- 이전 경로: `team/led.c`
- MCU / 언어 / 방식: f429zi / C / register
- 기능: PB7 1초 blink
- 핀/AF: PB7
- 클럭: HSI 16 MHz
- 직접 의존성: `stdint.h`
- 빌드: PASS; 하드웨어: HARDWARE REQUIRED
- 비고/개선: 실제 보드의 핀 배선·입력 극성 확인 필요

## `src/f429zi/register/gpio_button_toggle_debounce.c`

- 이전 경로: `team/led_button.c`
- MCU / 언어 / 방식: f429zi / C / register
- 기능: PC13 active-low 20ms debounce로 PA10 toggle
- 핀/AF: PC13 pull-up / PA10
- 클럭: HSI 16 MHz
- 직접 의존성: `stm32f429xx.h`, `stdint.h`
- 빌드: PASS; 하드웨어: HARDWARE REQUIRED
- 비고/개선: 실제 보드의 핀 배선·입력 극성 확인 필요

## `src/f429zi/register/gpio_button_toggle_raw.c`

- 이전 경로: `team/led_button_3.c`
- MCU / 언어 / 방식: f429zi / C / register
- 기능: PC13 하강 에지 PA10 반전; debounce 없음
- 핀/AF: PC13 / PA10
- 클럭: HSI 16 MHz
- 직접 의존성: `stdint.h`
- 빌드: PASS; 하드웨어: HARDWARE REQUIRED
- 비고/개선: 실제 보드의 핀 배선·입력 극성 확인 필요

## `src/f429zi/register/uart_echo_greeting.c`

- 이전 경로: `team/uart1.c`
- MCU / 언어 / 방식: f429zi / C / register
- 기능: USART3 인사 후 echo
- 핀/AF: PD8/PD9 AF7
- 클럭: HSI 16 MHz
- 직접 의존성: `stdint.h`
- 빌드: PASS; 하드웨어: HARDWARE REQUIRED
- 비고/개선: APB 16 MHz, BRR=1667 → 약 9598 baud (9600 설정용)

## `src/f429zi/register/uart_echo.c`

- 이전 경로: `team/uart2.c`
- MCU / 언어 / 방식: f429zi / C / register
- 기능: USART3 echo
- 핀/AF: PD8/PD9 AF7
- 클럭: HSI 16 MHz
- 직접 의존성: `stdint.h`
- 빌드: PASS; 하드웨어: HARDWARE REQUIRED
- 비고/개선: APB 16 MHz, BRR=1667 → 약 9598 baud (9600 설정용)

## `src/f429zi/register/uart_dual_init_draft.c`

- 이전 경로: `team/uart3.c`
- MCU / 언어 / 방식: f429zi / C / register
- 기능: uart3_init/uart1_init 호출만 존재; 정의 없음
- 핀/AF: 미설정
- 클럭: HSI 16 MHz
- 직접 의존성: `stdint.h`
- 빌드: FAIL; 하드웨어: HARDWARE REQUIRED
- 비고/개선: 실제 보드의 핀 배선·입력 극성 확인 필요

## `src/f429zi/register/systick_interrupt_uart_tx.c`

- 이전 경로: `team/uart_excepion.c`
- MCU / 언어 / 방식: f429zi / C / register
- 기능: 500ms SysTick_UserHandler에서 별표 송신 의도
- 핀/AF: PD8 AF7
- 클럭: HSI 16 MHz
- 직접 의존성: `stdint.h`
- 빌드: PASS; 하드웨어: HARDWARE REQUIRED
- 비고/개선: SDK vector와 UserHandler 이름 불일치; USART3 BRR=139, APB1 16 MHz에서 약 115108 baud (115200 설정용)

## `src/f429zi/register/systick_polling_uart_tx.c`

- 이전 경로: `team/uart_systick.c`
- MCU / 언어 / 방식: f429zi / C / register
- 기능: HCLK/8 SysTick 1초 폴링 점 출력
- 핀/AF: PD8 AF7
- 클럭: HSI 16 MHz
- 직접 의존성: `stdint.h`
- 빌드: PASS; 하드웨어: HARDWARE REQUIRED
- 비고/개선: USART3 BRR=139, APB1 16 MHz에서 약 115108 baud (115200 설정용)

## `src/f429zi/register/systick_pll_polling_uart_tx.c`

- 이전 경로: `team/uart_systick2.c`
- MCU / 언어 / 방식: f429zi / C / register
- 기능: 168MHz PLL, 500ms count 2회로 점 출력
- 핀/AF: PD8 AF7
- 클럭: HSI→PLL 168 MHz; APB1 42 MHz
- 직접 의존성: `stdint.h`
- 빌드: PASS; 하드웨어: HARDWARE REQUIRED
- 비고/개선: USART3 BRR=365, APB1 42 MHz에서 약 115068 baud (115200 설정용)

## `src/f429zi/register/systick_pll_interrupt_uart_tx.c`

- 이전 경로: `team/uart_systick_excepion.c`
- MCU / 언어 / 방식: f429zi / C / register
- 기능: 1ms SysTick_UserHandler, 별표/점 출력 의도
- 핀/AF: PD8 AF7
- 클럭: HSI→PLL 168 MHz; APB1 42 MHz
- 직접 의존성: `stdint.h`
- 빌드: PASS; 하드웨어: HARDWARE REQUIRED
- 비고/개선: SDK vector와 UserHandler 이름 불일치; USART3 BRR=365, APB1 42 MHz에서 약 115068 baud (115200 설정용)

## `src/f429zi/register/timer_interrupt_polling_uart_tx.c`

- 이전 경로: `team/uart_tim6.c`
- MCU / 언어 / 방식: f429zi / C / register
- 기능: TIM6_UserHandler @ 및 CNT rollover ! 출력 의도
- 핀/AF: PD8 AF7
- 클럭: HSI 16 MHz
- 직접 의존성: `stdint.h`
- 빌드: PASS; 하드웨어: HARDWARE REQUIRED
- 비고/개선: SDK vector와 UserHandler 이름 불일치; USART3 BRR=139, APB1 16 MHz에서 약 115108 baud (115200 설정용)

## `notes/gpio_pullup_open_drain_notes.c`

- 이전 경로: `team/bit.c`
- MCU / 언어 / 방식: unassigned / C / notes
- 기능: 풀업/오픈드레인/LED 관련 한국어 메모
- 핀/AF: 없음
- 클럭: 없음
- 직접 의존성: Rust 의존성은 Cargo.toml; 조각/메모는 내용 참조
- 빌드: NOT TESTED; 하드웨어: HARDWARE REQUIRED
- 비고/개선: C 구문 아님; 파일 내용 그대로 유지
