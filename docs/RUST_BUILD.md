# Rust 펌웨어 빌드: F446 / F767

## probe-rs 실행 및 디버깅

```sh
cargo run-f446  # F446RE uart_led.rs 빌드 → 다운로드 검증 → 실행
cargo run-f767  # F767VI 빌드 → 다운로드 검증 → 실행
```

해당 보드를 USB ST-LINK에 연결한 뒤 실행합니다. SWD 속도는 1000 kHz입니다.
`cargo run`은 기본 F446RE를 사용합니다. F767 실행은 `cargo run-f767`을 사용해
빌드 기능과 probe-rs 칩 이름을 함께 선택하세요.
MCU feature만 바꾼 `cargo run --no-default-features --features f767`은
기본 F446 runner를 사용하므로 실행 명령으로 사용하지 마세요.

VS Code에서는 왼쪽 **실행 및 디버그**에서 해당 보드의
`STM32F446RE: UART LED` 또는 `STM32F767VI: Flash & Debug`를 선택하고
**F5**를 누릅니다. 자동 빌드 후 다운로드하고 리셋 지점에서 멈춥니다.
F446은 `src/register/uart_led.rs`, F767은 `src/firmware.rs`에 중단점을
설정하고 **F5**로 계속 실행하세요.
**F10**은 한 줄씩 실행, **Shift+F5**는 디버깅 종료입니다.

터미널의 `cargo run-f446`/`cargo run-f767`은 실행 중 연결을 유지합니다.
두 펌웨어에는 로그 출력이 없어 화면이 조용한 것이 정상입니다.
터미널에서 **Ctrl+C**로 연결을 종료합니다.

### F446 UART LED 사용

`cargo run-f446`은 `src/register/uart_led.rs`를 실행합니다.
NUCLEO-F446RE의 ST-LINK 가상 시리얼 포트를 시리얼 터미널로 열고
**9600 baud, 8 data bits, no parity, 1 stop bit, 흐름 제어 없음**으로 설정하세요.
기본 보드 배선의 USART2(PA2/PA3)를 사용하며 PA5의 LD2 LED를 제어합니다.
문자 `1`을 보내면 켜지고, `0`을 보내면 꺼집니다. 다른 문자는 무시합니다.
UART 응답/에코는 송신하지 않습니다. 이 설정은 리셋 직후 HSI/APB1 16 MHz 기준입니다.

연결 확인:

```sh
probe-rs list
probe-rs info --protocol swd --verbose
```

자동 칩 식별이 실패해도 위의 verbose 연결 확인과 명시적 칩 설정은 사용할 수 있습니다.
ST-LINK 사용 중 오류가 나면 다른 디버그 세션과 STM32CubeProgrammer 연결을 종료하세요.
F767은 해당 보드에 연결한 상태에서 별도 확인이 필요합니다.

참고: [probe-rs 실행](https://probe.rs/docs/tools/probe-rs/),
[VS Code 디버거 설정](https://probe.rs/docs/tools/debugger/).

## 빌드만 수행

두 크레이트를 프로젝트에 유지하고, 빌드할 때 MCU 하나를 선택합니다.
`cargo build`의 기본 MCU는 STM32F446RE입니다.
두 칩 모두 Rust target은 `thumbv7em-none-eabihf`를 사용합니다.

```sh
# 이미 설치되어 있다면 생략
rustup target add thumbv7em-none-eabihf

# STM32F446RE
cargo f446

# STM32F767VI
cargo f767
```

결과 파일:

```text
target/rust-f446/thumbv7em-none-eabihf/debug/stm32
target/rust-f767/thumbv7em-none-eabihf/debug/stm32
```

확장자가 없어도 ELF 펌웨어입니다. 두 별칭은 출력 폴더를 분리합니다.
빌드만 수행하며 보드에 다운로드하지 않습니다.

직접 옵션을 지정할 수도 있습니다:

```sh
cargo build --no-default-features --features f446
cargo build --no-default-features --features f767
```

`--features f767`만 쓰면 기본 `f446`도 켜지므로 반드시
`--no-default-features`를 함께 사용하세요. `--all-features`는 사용하지 않습니다.

기본 `stm32` 바이너리의 진입점은 `src/firmware.rs`입니다. `#![no_std]`, `#![no_main]`,
`cortex-m-rt` 시작 코드와 패닉 핸들러가 설정되어 있습니다.
현재 펌웨어는 주변장치 핸들을 얻고 반복 대기합니다.
LED/UART 제어는 해당 MCU와 보드 핀 구성에 맞춰 추가해야 합니다.

`uart_led` 바이너리는 `src/register/uart_led.rs`이며 F446 기능에서만 빌드됩니다.
`src/main.rs`와 나머지 `src/register/*.rs`는 현재 Cargo 빌드에 포함하지 않습니다.
C 예제는 기존 Makefile을 사용합니다.

메모리 설정은 `memory/f446.x`와 `memory/f767.x`에서 선택합니다.
F767은 기존 C 링커 스크립트와 동일하게 데이터/스택에 DTCM 128 KiB를 사용하며,
SRAM1/SRAM2는 별도 배치하지 않습니다. 다른 칩 세부 모델은 메모리 용량을 확인하세요.

참고: [STM32 PAC](https://github.com/stm32-rs/stm32-rs),
[cortex-m-rt 메모리 및 링커 설정](https://docs.rs/cortex-m-rt/latest/cortex_m_rt/).
