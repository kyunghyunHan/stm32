# STM32

STM32F446RE / STM32F429ZI / STM32F767VI의 C·Rust 학습 코드입니다.
파일명과 보드별 위치를 정리했으며 기존 펌웨어 본문은 보존했습니다.

```text
src/
  f446re/
    register/   C 레지스터 실습 및 UART 통합 코드
    hal/        C HAL 실습
    rust/       Rust 실습
  f429zi/
    register/
  f767vi/
    register/
    hal/
    rust/
  firmware.rs           F446/F767 Rust 시작 템플릿
system/                 메모리 배치, C runtime, CMSIS system, HAL 설정
notes/                  원본 학습 메모와 미완성 조각
docs/                   실행 안내와 파일별 분석
```

`system/`은 예제 코드와 분리한 빌드 기반 파일입니다. `.ld`와 `.x`는 Flash/RAM 배치, `runtime.c`는 C 초기화 지원, `system_stm32f7xx.c`는 F7 시작 설정, HAL 설정/인터럽트 파일은 C HAL 예제에 필요합니다. 외부 SDK의 MCU별 startup Assembly와 함께 사용합니다.

**Python과 scripts 폴더는 제거했습니다.** 실행은 기존처럼 Rust는 `probe-rs run`, C는 `openocd`를 직접 사용합니다. 두 도구는 이미 빌드된 파일을 다운로드하므로 소스를 수정했다면 먼저 다시 빌드해야 합니다.

## Rust 실행 예시

Rust 파일은 [src/f446re/rust](src/f446re/rust)에 있습니다. 실행할 코드는 `Cargo.toml`의 binary `path`로 선택합니다.
`active-f446`은 현재 UART LED 예제(`uart_rx_led_control.rs`)를 가리킵니다. 터미널은 9600 baud / 8N1, `1`로 PA5 LED 켜기, `0`으로 끄기입니다.

```sh
# 소스 수정 후 빌드
cargo f446 --bin active-f446

# 보드 다운로드·실행
probe-rs run \
  --chip STM32F446RE \
  --protocol swd \
  --speed 1000 \
  --verify \
  target/rust-f446/thumbv7em-none-eabihf/debug/active-f446
```

기존 `led_button`, `uart_led`, `uart`, `stm32` binary 이름도 유지합니다. `active-f446`과 `uart_led`는 같은 소스를 가리키는 호환 이름이므로 Cargo가 중복 target 경고를 표시할 수 있습니다.

## C 실행 예시

F429ZI UART echo를 빌드했다면 다음처럼 직접 실행합니다.

```sh
openocd \
  -f interface/stlink.cfg \
  -f target/stm32f4x.cfg \
  -c "adapter speed 1000" \
  -c "program target/f429/register/uart_echo/uart_echo.elf verify reset exit"
```

기존 C 빌드 설정인 Makefile은 소스 경로를 맞춰 유지했습니다. 위 파일을 다시 빌드하려면 `make MCU=f429 MODE=register EXAMPLE=uart_echo`를 사용할 수 있습니다. Make로 Flash/실행할 필요는 없습니다. MCU·예제에 맞는 ELF를 선택하세요.

- [Rust·C 직접 실행 명령](docs/BUILD_RUN.md)
- [현재 구조 변경 사항](docs/REFACTOR_REPORT.md)
- [파일별 분석](docs/INVENTORY.md)
- [레지스터·MCU 학습](docs/REGISTER_LEARNING.md)
- [개발 환경](docs/stm32_env.md), [USART](docs/register.md)

실제 보드 실행은 이번 정리에서 검증하지 않았습니다. 미완성 코드와 기존 결함은 임의로 수정하지 않았습니다.
