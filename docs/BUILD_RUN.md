# Rust·C 직접 다운로드 및 실행

프로젝트 루트에서 실행합니다. `probe-rs run`과 `openocd`는 소스를 컴파일하지 않고 빌드된 펌웨어를 다운로드합니다. 코드 수정 뒤에는 먼저 다시 빌드하세요. ST-LINK에 연결하고 다른 디버그 세션은 종료합니다.

## Rust — STM32F446RE

Rust 소스는 `src/f446re/rust/`에 있습니다.

| Cargo binary | 소스 파일 | 기능 |
|---|---|---|
| led_button | gpio_button_led_level.rs | PC13 입력에 따라 PA5 출력 |
| uart_led | uart_rx_led_control.rs | USART2에서 1/0 수신, PA5 제어 |
| uart | uart_echo_greeting.rs | USART3 인사 및 echo; 원본 주석의 F429 표기와 Cargo F446 설정은 다름 |
| active-f446 | uart_rx_led_control.rs | 위 uart_led와 동일한 소스를 출력 이름만 유지해 빌드 |

### active-f446

Python은 사용하지 않습니다. 실행할 F446 소스를 바꾸려면 `Cargo.toml`에서 `name = "active-f446"` 아래 `path`를 원하는 `.rs` 파일로 수정합니다. 현재는 `src/f446re/rust/uart_rx_led_control.rs`입니다.

```sh
cargo f446 --bin active-f446
```

빌드한 파일을 보드에 다운로드·실행:

```sh
probe-rs run \
  --chip STM32F446RE \
  --protocol swd \
  --speed 1000 \
  --verify \
  target/rust-f446/thumbv7em-none-eabihf/debug/active-f446
```

`active-f446`과 기존 `uart_led`를 같은 소스에 연결했으므로 Cargo에서 같은 파일이 두 target에 있다는 경고가 나올 수 있습니다. 펌웨어 코드는 같습니다.

### 다른 binary

예를 들어 PC13/PA5 버튼 LED 실습은 다음과 같습니다.

```sh
cargo f446 --bin led_button
probe-rs run \
  --chip STM32F446RE \
  --protocol swd \
  --speed 1000 \
  --verify \
  target/rust-f446/thumbv7em-none-eabihf/debug/led_button
```

`uart_led`는 USART2 PA2 TX / PA3 RX AF7, 9600 baud / 8N1입니다. `1`은 PA5 켜기, `0`은 끄기입니다. 입력한 문자에 대한 응답은 없습니다.

## C — 기존 F446 출력 경로

기존 빌드 결과 경로와 다운로드 명령을 유지했습니다.

```sh
openocd \
  -f interface/stlink.cfg \
  -f target/stm32f4x.cfg \
  -c "adapter speed 1000" \
  -c "program target/f446/register/led_button/led_button.elf verify reset exit"
```

주의: 이 기존 `led_button.c`의 내용은 PC13/PF15이고 F446RE에는 PF15가 없습니다. 경로 보존과 하드웨어 적합성은 별개입니다. 코드를 PA5로 임의 변경하지 않았습니다. PA5만 반전하는 C 예제는 `src/f446re/register/gpio_led_blink.c`이고 기존 별칭으로 빌드하면 출력은 `target/f446/register/led/led.elf`입니다.

```sh
openocd \
  -f interface/stlink.cfg \
  -f target/stm32f4x.cfg \
  -c "adapter speed 1000" \
  -c "program target/f446/register/led/led.elf verify reset exit"
```

## C — F446 UART 명령 예제

대상: `src/f446re/register/uart_command_error_counters.c`. USART2 PA2 TX / PA3 RX, 9600 baud / 8N1. PA5는 led2, PB0/PB1은 led1/led3이며 외부 배선이 필요합니다.

```sh
openocd \
  -f interface/stlink.cfg \
  -f target/stm32f4x.cfg \
  -c "adapter speed 1000" \
  -c "program target/f446/register/uart_command_error_counters/uart_command_error_counters.elf verify reset exit"
```

시리얼 터미널에서 `led2 on`, `status`, `led2 off`, `errors`를 입력하고 Enter를 누릅니다.
HAL 버전은 소스의 `register`를 `hal`로, ELF 출력 경로의 `register`도 `hal`로 바꿉니다.

## C — F429 UART echo

대상: `src/f429zi/register/uart_echo.c`. USART3 PD8 TX / PD9 RX AF7, HSI 16 MHz 가정, 9600 baud / 8N1.

```sh
openocd \
  -f interface/stlink.cfg \
  -f target/stm32f4x.cfg \
  -c "adapter speed 1000" \
  -c "program target/f429/register/uart_echo/uart_echo.elf verify reset exit"
```

기존 F429 LED 버튼 출력도 `target/f429/team/led_button/led_button.elf`로 유지합니다.

## F767

Rust의 `src/firmware.rs`는 주변장치 설정 없는 시작 템플릿입니다.

```sh
cargo f767 --bin stm32
probe-rs run \
  --chip STM32F767VI \
  --protocol swd \
  --speed 1000 \
  --verify \
  target/rust-f767/thumbv7em-none-eabihf/debug/stm32
```

C는 `stm32f7x.cfg`와 해당 F767 ELF를 사용합니다.

```sh
openocd \
  -f interface/stlink.cfg \
  -f target/stm32f7x.cfg \
  -c "adapter speed 1000" \
  -c "program target/hal/adc/adc.elf verify reset exit"
```

## 소스 수정 뒤 C 재빌드

기존 C 빌드용 Makefile을 유지했으며, 다운로드 도구와 구분합니다. 기존 compiler 명령을 직접 사용해도 됩니다. 아래는 기존 빌드 설정을 이용하는 방법입니다.

```sh
make MCU=f446 MODE=register EXAMPLE=led
make MCU=f446 MODE=register EXAMPLE=led_button
make MCU=f446 MODE=register EXAMPLE=uart_command_error_counters
make MCU=f429 MODE=register EXAMPLE=uart_echo
make MCU=f767 MODE=hal EXAMPLE=adc
```

F446/F429 출력은 `target/<MCU>/<MODE>/<EXAMPLE>/<EXAMPLE>.elf`, F767은 `target/<MODE>/<EXAMPLE>/<EXAMPLE>.elf`입니다. 소스 이동 후에도 기존 이름의 출력 위치는 유지합니다.

## 개발 환경 및 확인 범위

Rust ARM target이 없다면 `rustup target add thumbv7em-none-eabihf`로 설치합니다. C 빌드에는 Arm GCC 및 STM32CubeF4/F7 SDK가 필요합니다. `system/`에는 직접 삭제하면 링크나 HAL 빌드가 깨지는 메모리 배치·runtime·system·HAL 지원 파일이 들어 있습니다.

이번에는 파일 경로와 빌드를 확인했으며 Flash/Debug 또는 실제 MCU 동작은 검증하지 않았습니다. `probe-rs run`은 Ctrl+C로 종료합니다. OpenOCD의 `verify reset exit`는 기록 검증 후 리셋하고 종료합니다.
