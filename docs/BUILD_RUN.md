# Rust·C 빌드 및 실행 명령

프로젝트 루트에서 실행합니다. **코드 저장 → 빌드 성공 확인 → 보드 실행** 순서입니다.
Rust는 `probe-rs`, C는 `OpenOCD`로 올립니다. 두 실행 도구는 소스를 빌드하지 않습니다.
ST-LINK USB 포트에 연결하고 다른 디버그 세션은 종료하세요.

## 1. Rust — F446RE

### LED 버튼: src/register/led_button.rs

빌드:

```sh
cargo f446 --bin led_button
```

보드 실행:

```sh
probe-rs run \
  --chip STM32F446RE \
  --protocol swd \
  --speed 1000 \
  --verify \
  target/rust-f446/thumbv7em-none-eabihf/debug/led_button
```

### UART LED: src/register/uart_led.rs

빌드:

```sh
cargo f446 --bin uart_led
```

보드 실행:

```sh
probe-rs run \
  --chip STM32F446RE \
  --protocol swd \
  --speed 1000 \
  --verify \
  target/rust-f446/thumbv7em-none-eabihf/debug/uart_led
```

시리얼 터미널은 9600 baud, 8N1, 흐름 제어 없음으로 설정합니다.
문자 `1`은 LED 켜기, `0`은 끄기입니다. 수신 응답은 출력하지 않습니다.

### 새 Rust 파일을 Cargo.toml에 등록하지 않고 실행

빌드 명령의 파일 경로를 원하는 독립 펌웨어 `.rs` 파일로 바꿉니다.
파일에는 `#![no_std]`, `#![no_main]`, `#[entry]` 진입점과 패닉 핸들러가 필요합니다.

```sh
python3 scripts/build_active_f446.py src/register/uart_led.rs
```

이 방식의 출력 이름은 항상 `active-f446`입니다.

```sh
probe-rs run \
  --chip STM32F446RE \
  --protocol swd \
  --speed 1000 \
  --verify \
  target/rust-f446/thumbv7em-none-eabihf/debug/active-f446
```

VS Code에서는 실행할 파일을 열고 저장한 뒤,
`STM32F446RE: Current Rust File`을 선택하고 F5를 누르면 빌드와 다운로드를 수행합니다.
오류가 나면 수정한 뒤 다시 실행하세요.

## 2. C — F446RE

### LED 버튼: src/register/led_button.c

빌드:

```sh
make stm32-build MCU=f446 MODE=register EXAMPLE=led_button
```

보드 실행:

```sh
openocd \
  -f interface/stlink.cfg \
  -f target/stm32f4x.cfg \
  -c "adapter speed 1000" \
  -c "program target/f446/register/led_button/led_button.elf verify reset exit"
```

### LED: src/register/led.c

빌드:

```sh
make stm32-build MCU=f446 MODE=register EXAMPLE=led
```

보드 실행:

```sh
openocd \
  -f interface/stlink.cfg \
  -f target/stm32f4x.cfg \
  -c "adapter speed 1000" \
  -c "program target/f446/register/led/led.elf verify reset exit"
```

현재 F446 C 빌드는 `MODE=register`, `EXAMPLE=led|led_button`을 지원합니다.
새 C 예제는 Makefile의 F446용 `REGISTER_EXAMPLES` 목록에 추가해야 합니다.

## 3. Rust — F767VI 보드 사용 시

대상 소스는 `src/firmware.rs`입니다.

빌드:

```sh
cargo f767 --bin stm32
```

보드 실행:

```sh
probe-rs run \
  --chip STM32F767VI \
  --protocol swd \
  --speed 1000 \
  --verify \
  target/rust-f767/thumbv7em-none-eabihf/debug/stm32
```

## 4. C — F767VI 보드 사용 시

빌드:

```sh
make stm32-build MCU=f767 MODE=register EXAMPLE=led
```

보드 실행:

```sh
openocd \
  -f interface/stlink.cfg \
  -f target/stm32f7x.cfg \
  -c "adapter speed 1000" \
  -c "program target/register/led/led.elf verify reset exit"
```

| MODE | EXAMPLE |
|---|---|
| register | led, led_button, adc, interrupt, timer_counter, urtc |
| hal | led, adc |

다른 예제를 실행하려면 `MODE`와 `EXAMPLE`을 바꾸고, 실행 경로도 `target/<MODE>/<EXAMPLE>/<EXAMPLE>.elf`로 맞춥니다.

## 5. C — STM32F429ZITx 외부 LED·버튼

대상은 `team/led_button.c`입니다. PA10 → 저항 → LED → GND,
PC13 → 버튼 → GND로 연결합니다. PC13은 내부 풀업을 사용합니다.
버튼을 누를 때마다 LED가 켜지거나 꺼지며, 20ms 디바운스를 적용합니다.

빌드:

```sh
make stm32-build MCU=f429 MODE=team EXAMPLE=led_button
```

F429 보드에 다운로드·실행:

```sh
openocd \
  -f interface/stlink.cfg \
  -f target/stm32f4x.cfg \
  -c "adapter speed 1000" \
  -c "program target/f429/team/led_button/led_button.elf verify reset exit"
```

F429 전용 시작 코드와 2 MiB Flash / 192 KiB SRAM을 사용합니다.
64 KiB CCM은 이 예제에서 사용하지 않습니다.

## 6. 최초 설정과 연결 오류

Rust의 ARM 타깃이 설치되지 않았다면 한 번 실행합니다.

```sh
rustup target add thumbv7em-none-eabihf
```

C 컴파일러는 STM32CubeIDE 안의 Arm GNU Toolchain을 우선 사용합니다.
SDK는 `~/STM32Cube/Repository/STM32Cube_FW_F4_V*` / `STM32Cube_FW_F7_V*`에서 찾습니다.
다른 위치라면 `make` 명령에 `CUBE_F4=/path/to/SDK` 또는 `CUBE_F7=/path/to/SDK`를 추가합니다.

| 증상 | 확인할 내용 |
|---|---|
| `open failed` | ST-LINK 포트·USB 케이블·다른 디버거 사용 여부 |
| `UNEXPECTED idcode` | F446·F429는 `stm32f4x.cfg`, F767은 `stm32f7x.cfg`. ELF도 같은 MCU용으로 빌드 |
| 빌드 실패 | 수정 후 빌드 성공을 확인하고 실행 |

`probe-rs run`은 Ctrl+C로 종료합니다.
OpenOCD의 `verify reset exit`는 기록 검증 후 리셋해 실행하고 OpenOCD를 종료합니다.
