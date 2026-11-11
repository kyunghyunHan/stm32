# 파일 정리 결과

사용자 정정 요청에 따라 주변장치별 하위 폴더와 projects/platform/scripts 구분을 줄이고 실행 방식을 기존 직접 명령으로 유지했습니다.

## 현재 구조

- `src/<보드>/register/`: C 레지스터 코드. 통합 UART 프로젝트도 같은 위치에 둡니다.
- `src/<보드>/hal/`: C HAL 코드.
- `src/<보드>/rust/`: Rust 코드. F767 Rust HAL 초안도 보존합니다.
- `src/firmware.rs`: F446/F767 공용 Rust 시작 템플릿.
- `system/`: linker/memory, CMSIS system, C runtime, HAL 설정과 인터럽트 지원. 빌드에 필요한 파일입니다.
- `notes/`: 원본 메모·미완성 조각.
- `docs/`: 분석과 실행 명령.

Rust 파일을 포함한 원본 펌웨어 61개의 실행 코드는 변경하지 않았습니다. UART 동기화 파일의 잘못된 MCU 주석은 사용자 확인에 따라 STM32F439ZI에서 STM32F429ZI로 정정했습니다. Python 3개(원래 있던 active-file helper와 이전 정리에서 추가한 catalog/verify 도구)는 사용자의 삭제 요청에 따라 제거했습니다. 원래 없던 디렉터리별 README와 examples.mk도 제거하고 소스 선택표는 기존 Makefile 안에 합쳤습니다. 파일 이동 기록은 [file_moves.json](file_moves.json), 펌웨어별 내용은 [INVENTORY.md](INVENTORY.md), 중복 분석은 [DUPLICATES.md](DUPLICATES.md)에 남겼습니다.

## 직접 실행

Rust: `probe-rs run ... target/rust-f446/thumbv7em-none-eabihf/debug/active-f446`.
C: `openocd ... -c "program <선택한 ELF> verify reset exit"`.
구체적인 명령은 [BUILD_RUN.md](BUILD_RUN.md)에 있습니다. Make flash/debug 실행 래퍼는 제거했습니다. Makefile은 원래 역할인 C 컴파일 설정으로 유지했습니다.

`active-f446`은 이제 Python이 아닌 Cargo binary입니다. 현재 UART LED 소스를 가리키며 Cargo.toml의 path로 실행할 파일을 선택합니다. 기존 `uart_led` 이름도 유지하여 두 target이 같은 파일이라는 Cargo 경고가 있을 수 있습니다. 임의의 현재 파일을 자동 탐지하는 Python 기능은 제거했습니다.

F446/F429의 기존 C ELF 경로, F767의 기존 ELF 경로, 기존 Rust binary 이름을 유지합니다. 빌드된 파일이 있어야 직접 다운로드 명령을 사용할 수 있으며 소스를 수정했다면 다시 컴파일해야 합니다.

## 검증

첫 정리에서 정규 C 38개와 Rust 5개 빌드에 성공했고, 기준이 있는 C 37개와 기존 명령 11개의 BIN SHA-256이 일치했습니다. 이번 단순화 후에는 C 전체 38개와 Cargo F446의 모든 binary(기존 4개+active-f446), F767 템플릿을 재빌드해 경로 변경을 확인했습니다. 펌웨어와 기반 소스/메모리 파일의 원본 체크섬도 대조해 일치함을 확인했습니다. 기존 C 출력 경로 11개도 다시 빌드해 BIN 일치를 확인했습니다.

기존 빌드 요약은 [build_results.json](build_results.json), 기존 출력 경로 비교는 [legacy_build_results.json](legacy_build_results.json)에 있습니다. Python 없이 운영할 수 있으며 분석 JSON은 참고 기록입니다. 미완성 초안 1개는 기존부터 빌드 실패였고, 당시 대상 미확정/조각/미등록 의존성 코드 18개는 전체 링크 미검증이었습니다. 이후 사용자 확인에 따라 11개를 F429ZI로 추가 빌드했습니다.

실제 Flash/Debug나 MCU 실행은 수행하지 않았습니다. 기존 결함을 임의로 수정하지 않았고 커밋·push도 하지 않았습니다.

## 보존한 코드 결함과 제약


아래 항목은 구조 변경과 구분하기 위해 수정하지 않았습니다. PASS가 동작 정상임을 뜻하지 않습니다.

| 항목 | 원인·영향 | 처리 |
|---|---|---|
| F446 uart_rx_led_control 및 variant | `USART2_SR & ((1U << 5) == 0U)`의 오른쪽 비교가 항상 0이라 RXNE 대기를 하지 않음 | 두 원본 변형 보존, unused variable 경고 기록 |
| F429 systick_interrupt_uart_tx / systick_pll_interrupt_uart_tx | 함수명이 SysTick_UserHandler이지만 외부 SDK vector는 SysTick_Handler를 요구 | 링크 성공해도 default handler로 진입; 이름 변경은 별도 기능 수정 필요 |
| F429 timer_interrupt_polling_uart_tx | TIM6_UserHandler가 startup의 TIM6_DAC_IRQHandler와 연결되지 않음 | 링크 성공과 IRQ 동작 구분 |
| uart_dual_init_draft | uart3_init/uart1_init 선언·정의 누락 | 변경 전후 FAIL, 정규 목록 제외 |
| 기존 F446 register/led_button | PC13/PF15 코드이나 F446RE LQFP64에 PF15 없음 | 호환 별칭만 보존, 정규 F446 목록 제외 |
| 기존 F767 register/led 및 led_button | 사용한 GPIO/RCC 주소는 F767 CMSIS와도 일치; 보드 배선 정보는 미확정 | 기존 소스와 별칭 보존; PA5/PF15 패키지·배선 확인 필요 |
| F767 PLL 예제 | HSE 기반 M=8 N=216 P=2, 외부 HSE=16MHz 조건에서만 SYSCLK 216MHz; system 기본 HSE_VALUE는 25MHz | 실제 발진기·전압·클럭 분주 확인 필요; 대기 루프 timeout 없음 |
| Rust uart echo | Cargo는 F446 PAC/메모리를 사용하나 소스 주석은 F429ZI | 현재 F446 build target을 그대로 보존; F429용 Rust 지원으로 주장하지 않음 |
| 미등록 Rust LED 2개 | panic_halt 및 Rust HAL crate가 Cargo 의존성에 없음 | 초안 유지, 정규 target 아님 |
| register layout 조각 | RCC_AHB1ENR_OFFSET 정의 없음; 매크로가 미사용이라 구문 검사를 통과 | 독립 펌웨어 아님; 확장 시 수정 필요 |
| GPIO/통신 예제 다수 | 버튼 입력 극성·풀업/풀다운이 서로 다름; 외부 배선 필요, raw edge는 debounce 없음 | 핀/극성/학습 변형 유지 |
| UART command/time polling | 긴 blocking 송신 중 SysTick COUNTFLAG 여러 회를 놓칠 수 있음 | 1ms 스캔의 실시간 보장 없음; 별도 ISR/queue 개선 대상으로 기록 |
| UART heap dump | 고정 RAM 주소 덤프는 linker/heap 배치가 달라지면 관심 객체와 일치하지 않을 수 있음 | 학습용 실험으로 유지 |
| CAN loopback LED | sleep 해제 여부 및 무기한 대기 경로 확인 필요 | 실제 칩 확인 필요; 성공으로 추정하지 않음 |

F446 핀 제약은 [ST STM32F446xC/E 데이터시트](https://www.st.com/resource/en/datasheet/DM00141306.pdf)의 패키지별 핀 표와 대조했습니다. 핸들러 이름은 설치된 CubeF4 startup_stm32f429xx.s와 대조했습니다. 그 밖의 전기적 조건/타이밍 전체를 공식 매뉴얼로 전수 검증하거나 실제 보드에서 측정한 것은 아닙니다.


## F429ZI 통합

사용자 확인에 따라 대상 미확정 폴더의 11개 파일과 F439ZI 폴더의 1개 파일을 모두 `src/f429zi/register/`로 옮겼습니다. 이름 충돌은 없었습니다. main 없는 gpio_register_layout.c는 학습 조각으로 유지하며, 나머지 11개를 F429 빌드 목록에 추가했습니다. Rust와 직접 다운로드 명령은 변경하지 않았습니다.

통합 후 F429ZI 정규 예제 28개 빌드 성공, gpio_register_layout.c는 구문 검사 성공(main 없는 조각)입니다. 실제 보드 실행은 HARDWARE REQUIRED입니다.
