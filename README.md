# STM32F767VIT6 CLI 프로젝트

STM32CubeMX의 `Core/` 디렉터리 없이 GNU Make와 ARM GCC로 빌드한다.
레지스터 직접 제어와 HAL을 `MODE`로, 실행 프로그램을 `EXAMPLE`로 선택한다.
CMSIS 헤더·GCC startup·HAL 드라이버는 외부 STM32CubeF7 SDK를 참조한다.

```text
stm32/
├── Makefile
├── README.md
├── .gitignore
├── STM32F767VI_FLASH.ld
├── src/
│   ├── register/
│   │   ├── led.c
│   │   ├── adc.c
│   │   ├── interrupt.c
│   │   ├── timer_counter.c
│   │   ├── urtc.c
│   │   ├── clock.c               # main 없는 학습 코드
│   │   ├── register_offset.c
│   │   ├── led_f446.c            # 기존 F446 코드 보존
│   │   ├── can_f446.c
│   │   ├── led_swicth_f446.c
│   │   ├── main_legacy.c          # 기존 빈 main.c 보존
│   │   └── throttle.c
│   ├── hal/
│   │   ├── led.c
│   │   ├── adc.c
│   │   ├── stm32f7xx_it.c        # HAL SysTick 구현
│   │   └── stm32f7xx_hal_msp.c    # HAL MSP 구현
│   └── system/
│       ├── system_stm32f7xx.c
│       └── runtime.c
├── include/
│   └── stm32f7xx_hal_conf.h
├── docs/
└── target/                      # 생성물; Git 제외
```

## 도구와 외부 SDK

필수 도구는 GNU Make, `arm-none-eabi-gcc`, 같은 툴체인의 `objcopy`와 `size`다.
macOS에서는 STM32CubeIDE에 포함된 GCC를 우선 사용하고, 없으면 PATH를 사용한다.
SDK는 아래 경로에서 자동 탐색한다. 여러 버전이 있으면 정렬된 마지막 경로를 선택하므로
재현 가능한 빌드에서는 `CUBE_F7`를 명시한다.

```text
~/STM32Cube/Repository/STM32Cube_FW_F7_V*
~/STM32Cube/Repository/STM32CubeF7
```

```bash
make stm32-build MODE=hal EXAMPLE=adc CUBE_F7=/path/to/STM32CubeF7
make stm32-build MODE=register EXAMPLE=led CROSS_COMPILE=/path/to/bin/arm-none-eabi-
```

SDK에는 `Drivers/CMSIS/Include`, `Drivers/CMSIS/Device/ST/STM32F7xx`,
`Drivers/STM32F7xx_HAL_Driver`의 실제 파일이 있어야 한다.
Git 저장소로 SDK를 받았다면 해당 드라이버 서브모듈도 준비해야 한다.
프로젝트의 `include/`에 SDK 헤더를 복사하지 않는다.
SDK 원본 system 템플릿은 `src/system/system_stm32f7xx.c`에 복사하여 프로젝트가 관리한다.
해당 파일의 ST 저작권 문구와 동봉 CMSIS 장치 라이선스를 유지한다.

## 빌드

```bash
make stm32-build MODE=register EXAMPLE=led
make stm32-build MODE=hal EXAMPLE=led
make stm32-build MODE=register EXAMPLE=adc
make stm32-build MODE=hal EXAMPLE=adc

make check-all   # 두 모드의 지원 예제 전체 + 학습 조각 문법 검사
make build-all   # 두 모드의 지원 예제 전체 컴파일·링크·BIN 생성
make lint        # check-all에 -Werror 적용
make clean      # target/ 전체 제거
```

기본값은 `MODE=register EXAMPLE=adc`다. `check-all`과 `build-all`은 지정된 MODE와 관계없이
두 모드를 모두 순회한다. 지원하지 않는 모드·예제는 빌드 전에 오류로 알려 준다.
`stm32-check`는 선택한 조합만 문법 검사한다.

| MODE | EXAMPLE | 기능 및 핀 |
|---|---|---|
| register | led | PA5 출력, 3초마다 반전; HSI 16 MHz, SysTick polling |
| hal | led | PA5 출력, HAL_Delay(3000); HSI 16 MHz |
| register | adc | ADC1 채널 7 / PA7 단일 변환, adc_raw·throttle_V; 기존 216 MHz 초기화 유지 |
| hal | adc | ADC1 채널 7 / PA7 단일 변환 polling, adc_raw·throttle_V; HSI 16 MHz, ADC 8 MHz |
| register | interrupt | PD4 하강 에지 EXTI4로 PC6 반전 |
| register | timer_counter | TIM1 업데이트 인터럽트로 PC6 반전 |
| register | urtc | PD5/PD6 USART2 9600 bps, 수신 값 + 1 송신 |

산출물은 `target/<MODE>/<EXAMPLE>/<EXAMPLE>.elf`, `.bin`, `.map`이다.
각 빌드는 명시한 예제 `.c` 하나와 공통 system 코드만 선택한다. 다른 예제와 보존용 파일을
wildcard로 링크하지 않으므로 `main()`과 초기화 함수가 충돌하지 않는다.
매번 다시 컴파일하여 모드·SDK·헤더·컴파일 옵션 변경 시 오래된 결과를 사용하지 않는다.

HAL 모드만 `USE_HAL_DRIVER`를 정의한다. HAL 설정은 필요한 기본 모듈만 활성화하고,
ADC 예제에서 ADC/ADCEx 및 드라이버 의존성인 DMA/DMAEx를 추가한다.
`src/hal/stm32f7xx_it.c`가 유일한 프로젝트 SysTick 핸들러다.
MSP는 `src/hal/stm32f7xx_hal_msp.c`에서만 구현한다.
ADC는 polling을 사용하므로 별도 ADC/DMA 인터럽트 핸들러가 필요하지 않다.
레지스터 모드는 이 HAL 공통 소스와 드라이버를 전혀 링크하지 않는다.

## Startup과 메모리

대상은 `STM32F767xx`, Cortex-M7, Thumb, `fpv5-d16`, hard-float ABI다.
외부 SDK의 `startup_stm32f767xx.s`를 사용한다. 이 startup이 사용하는
`_estack`, `_sidata`, `_sdata`, `_edata`, `_sbss`, `_ebss`를 링커가 제공한다.
`USER_VECT_TAB_ADDRESS`로 벡터 테이블을 AXI Flash 시작 주소에 설정한다.
`-nostartfiles`로 일반 CRT 시작 코드를 제외하고, `runtime.c`에서 newlib의
`__libc_init_array()`에 필요한 `_init`/`_fini`를 제공한다.

| 영역 | 시작 주소 | 크기 | 현재 배치 |
|---|---|---|---|
| Flash | 0x08000000 | 2048 KiB | 벡터, 코드, 상수, .data 초기값 |
| DTCM | 0x20000000 | 128 KiB | .data, .bss, heap, stack |
| SRAM1 | 0x20020000 | 368 KiB | 별도 선언, 기본 미사용 |
| SRAM2 | 0x2007C000 | 16 KiB | 별도 선언, 기본 미사용 |

초기 SP는 `0x20020000`이며 스택은 DTCM 안에서 아래로 자란다. heap 512 B와 stack 2 KiB의
최소 공간을 예약하고 영역 초과를 링크 시 검사한다. 이는 실행 중 스택 사용량 검사는 아니다.
총 512 KiB SRAM에는 DTCM 128 KiB가 이미 포함된다. ITCM 16 KiB와 backup SRAM 4 KiB는
이번 배치에서 사용하지 않는다. 메모리 용량은 [ST 제품 정보](https://www.st.com/en/microcontrollers-microprocessors/stm32f767vi.html)와
[DS11532 데이터시트](https://www.st.com/resource/en/datasheet/stm32f767vi.pdf)를 기준으로 확인했다.

작업 시작 시 기존 `STM32F767VI_FLASH.ld` 파일은 없었다.
남아 있던 `target/core/core.map`에서 Flash `0x08000000 / 0x200000`과
RAM `0x20000000 / 0x20000` 배치를 확인했다. 새 스크립트는 이 배치를 유지하며
미사용 SRAM1/SRAM2를 분리해서 명시한다. 기존 스크립트 원문 전체의 검증은 불가능하다.

## 기존 코드 보존과 보드 전제

작업 시작 시 `Core/` 전체가 없었으므로 `Core/Src/main.c`의 구현을 분석하거나 이동할 수는 없었다.
실제 `src/main.c`는 빈 파일이며 `src/register/main_legacy.c`에 그대로 보존했다.
기존 F767 `adc.c`, `interrupt.c`, `timer_counter.c`, `urtc.c`는 `src/register/` 바로 아래에
내용 변경 없이 이동했다. `clock.c`, `register_offset.c`도 같은 폴더에 둔다.
F446용 원본은 `led_f446.c`, `led_swicth_f446.c`, `can_f446.c`로 이름을 바꾸어 보존했고,
빈 `throttle.c`도 같은 폴더에 보존했다. 이 파일들은 펌웨어 링크 대상에서 제외한다.
새 register/led는 원래 PA5/SysTick 동작을 F767 CMSIS 헤더와 SystemCoreClock 기반으로 이식했다.
기존 README는 [학습 문서](docs/REGISTER_LEARNING.md)에 원문을 보존했다.

기존 register ADC/EXTI/TIM/UART 예제는 외부 HSE 16 MHz와 3.3 V 전원을 전제로 한
216 MHz 초기화를 그대로 사용한다. 보드 크리스털 주파수와 배선을 확인해야 한다.
새 LED/HAL 예제는 리셋 상태의 HSI 16 MHz를 사용한다. PA5가 실제 보드 LED에
연결되어 있다는 보장은 없으므로 해당 핀의 외부 LED 배선을 기준으로 한다.
ADC 전압 환산은 VREF+ = 3.3 V 가정이다.

빌드·검증 명령은 보드에 Flash를 기록하지 않는다. 이 Makefile은 Flash 기록 타깃을 제공하지 않는다.
컴파일·링크 검증과 별도로 실제 보드의 클록, 핀 및 주변장치 동작 검증은 아직 수행하지 않았다.

검증 환경과 예제별 크기는 [빌드 검증 기록](docs/BUILD_VALIDATION.md)을 참고한다.

## VS Code IntelliSense

`.vscode/c_cpp_properties.json`에 외부 SDK includePath, ARM GCC와 MCU 옵션을 설정했다.
기본 구성은 `STM32F767 HAL ADC`다. 명령 팔레트의 `C/C++: Select a Configuration`에서
`STM32F767 HAL LED` 또는 `STM32F767 Register`를 선택할 수 있다.
이 선택은 편집기 분석에만 적용되며, 실제 빌드는 Makefile의 MODE/EXAMPLE을 따른다.

SDK 위치를 변경하면 이 파일의 `env.stm32Sdk`를 Makefile의 `CUBE_F7`와 같은 경로로 맞춘다.
툴체인을 바꾸면 각 구성의 `compilerPath`도 수정한다.
설정 적용 후에도 이전 include 오류가 남으면 `C/C++: Reset IntelliSense Database`를 실행하고
`Developer: Reload Window`로 창을 다시 불러온다.
