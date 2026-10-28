# STM32 개발 환경 정리

## 1. Host System과 Target System

- **Host System**: 코드를 작성하고 컴파일하는 개발 컴퓨터. 예: macOS(Apple Silicon), Windows(x86-64), Linux.
- **Target System**: 컴파일된 프로그램을 실제로 실행하는 MCU. 예: STM32F446RE(ARM Cortex-M4).
- 개발 PC의 CPU와 MCU의 CPU 아키텍처가 다르므로 일반적으로 **Cross Compiler(크로스 컴파일러)**를 사용한다.

> 주의: Host가 반드시 x86인 것은 아니다. Mac M1/M2/M3 등은 ARM64를 사용하지만, STM32용으로 별도의 타깃 컴파일을 한다.

```text
Host PC (macOS / Windows / Linux)
    C/Rust 소스코드
          |
          v
    Cross Compiler + Linker
          |
          v
     ELF (필요시 BIN/HEX)
          |
          v
  다운로드/디버깅 소프트웨어
  (OpenOCD / probe-rs / STM32CubeProgrammer 등)
          |
          v
      ST-Link (하드웨어)
          | SWD
          v
Target STM32 MCU
    Flash 기록 -> CPU 실행
```

## 2. 프로그램 및 장비의 역할

| 도구/장비 | 종류 | 역할 |
|---|---|---|
| **STM32CubeMX** | PC 소프트웨어 | MCU의 클록, GPIO, UART 등 설정 및 초기화 코드/프로젝트 생성 |
| **STM32CubeIDE** | 통합 개발 환경(IDE) | 코드 작성, 빌드, 다운로드, 디버깅을 통합 제공(CubeMX 기능 통합) |
| **VSCode** | 코드 편집기 | 코드 작성·탐색. 빌드/플래시/디버그 도구는 별도 구성 |
| **arm-none-eabi-gcc** | 크로스 컴파일러 | C/C++ 코드를 Cortex-M용 기계어로 컴파일·링크 |
| **rustc** + ARM target | 크로스 컴파일러 | Rust 코드를 Cortex-M용으로 컴파일·링크 |
| **Make / CMake / Ninja** | 빌드 도구 | 컴파일·링크 명령과 의존 관계 관리 |
| **OpenOCD** | PC 소프트웨어 | ST-Link 같은 디버그 프로브를 통해 플래시 기록 및 GDB 디버깅 지원 |
| **probe-rs** | PC 소프트웨어 | ST-Link 등 지원 프로브를 통한 플래시 기록 및 디버깅 지원. Rust 프로젝트에서도 자주 사용 |
| **STM32CubeProgrammer** | PC 소프트웨어 | ST-Link 등을 사용해 펌웨어 다운로드 및 메모리 관리 |
| **ST-Link** | 하드웨어 디버그 프로브 | PC(USB)와 MCU(SWD/JTAG) 사이의 실제 연결 담당 |
| **GDB** | PC 디버거 | 중단점, 단계 실행, 변수·메모리·레지스터 검사(OpenOCD와 함께 사용 가능) |
| **UART 터미널** | PC 소프트웨어 | MCU가 시리얼로 출력한 로그 확인(예: screen, minicom) |

**핵심:** `OpenOCD`와 `probe-rs`는 **ST-Link 하드웨어의 대체품이 아니다.** ST-Link와 통신하는 PC 측 도구다. 또한 개발 언어와 독립적으로 선택할 수 있다. C라고 OpenOCD만, Rust라고 probe-rs만 쓰는 것은 아니다.

## 3. 개발 방식 A: STM32CubeIDE 중심

```text
STM32CubeIDE
  ├─ MCU 설정(CubeMX 기능)
  ├─ C 코드 작성
  ├─ 빌드(ARM GCC)
  ├─ 다운로드 및 디버그(IDE에 통합된 도구)
  └─ ST-Link(하드웨어) -> SWD -> STM32
```

- 장점: 설정이 비교적 간편하고, 컴파일·다운로드·디버깅을 하나의 IDE에서 관리한다.
- 참고: CubeIDE가 내부에서 사용하는 디버깅 백엔드/서버는 버전·설정에 따라 달라질 수 있다. **CubeIDE = 반드시 OpenOCD**라고 단정하면 안 된다.

## 4. 개발 방식 B: VSCode 중심

```text
VSCode: C 코드 작성
      |
      v
Make 또는 CMake(+ Ninja): 빌드 관리
      |
      v
arm-none-eabi-gcc + linker script
      |
      v
firmware.elf
      |
      +--> OpenOCD --> ST-Link --> STM32에 기록
      |         ^
      |         +--- GDB를 연결해 디버깅 가능
      |
      +--> probe-rs --> ST-Link --> STM32에 기록/디버깅
```

- VSCode는 **편집기**이며 OpenOCD/probe-rs를 자체적으로 필수 제공하는 것이 아니다.
- **CubeMX는 VSCode에서도 선택적으로 사용**할 수 있다. CubeMX에서 프로젝트/초기화 코드를 생성한 뒤 VSCode에서 수정·빌드하면 된다.
- CubeMX 없이 CMSIS 헤더, 시작 코드(startup), 링커 스크립트와 직접 작성한 레지스터 초기화 코드로 구성할 수도 있다.

### 사용 중인 STM32F446RE 예시

| 항목 | 예시 |
|---|---|
| Host | MacBook(Apple Silicon) |
| Editor | VSCode |
| Target | NUCLEO-F446RE, ARM Cortex-M4 |
| Compiler | `arm-none-eabi-gcc` |
| Build | Make 또는 CMake/Ninja |
| Flash/Debug | OpenOCD + ST-Link, 또는 probe-rs + ST-Link |
| Hardware connection | 보드에 내장된 ST-Link → SWD → STM32F446RE |

## 5. 개발 방식 C: Rust + probe-rs

```text
VSCode / Editor
      |
      v
Rust 소스코드
      |
      v
cargo build (rustc, ARM target)
      |
      v
firmware ELF
      |
      v
probe-rs
      |
      v
ST-Link(하드웨어)
      | SWD
      v
STM32 MCU
```

- `probe-rs`는 Rust로 작성된 디버그·플래시 도구이지만 **C로 만든 ELF도 다룰 수 있다**.
- 반대로 Rust로 빌드한 ELF를 OpenOCD 및 GDB로 디버깅하는 구성도 가능하다.
- `espflash`는 ESP32 펌웨어 다운로드용 소프트웨어이며, ST-Link와는 다른 연결 방식/하드웨어를 주로 이용한다.

## 6. ELF, BIN, Flash, RAM

1. **컴파일·링크**: 소스코드 → `firmware.elf` 생성. ELF에는 로드할 코드·데이터와 디버깅 정보 등이 포함될 수 있다.
2. **형식 변환(필요한 경우)**: `arm-none-eabi-objcopy -O binary firmware.elf firmware.bin`으로 BIN 생성 가능. 반드시 BIN이 있어야 하는 것은 아니다. OpenOCD와 probe-rs는 ELF를 직접 사용할 수 있다.
3. **다운로드(Flash programming)**: ST-Link 등을 통해 MCU의 비휘발성 Flash에 펌웨어를 기록한다.
4. **실행**: 리셋 후 MCU가 벡터 테이블·시작 코드를 따라 초기화하고 `main()`으로 진입한다. 일반적인 STM32에서는 명령어가 주로 Flash에서 실행되고, 가변 데이터와 스택은 RAM을 이용한다.
5. **디버깅**: SWD로 MCU를 제어하며 중단점·메모리·레지스터 등을 점검한다.
6. **UART 로그**: UART 기능을 구성하고 USB-UART 연결/보드의 가상 COM 포트(VCP) 구성을 확인한 후 별도 터미널에서 출력값을 본다. UART는 다운로드·디버깅과 다른 통신 경로다.

## 7. Mac에서 설치/연결 상태 확인

```bash
# ARM C 툴체인
arm-none-eabi-gcc --version

# OpenOCD 설치 및 경로
which openocd
openocd --version

# Homebrew로 설치되었는지 확인
brew list --versions open-ocd

# probe-rs 도구(설치한 경우)
probe-rs --version
```

OpenOCD로 ST-Link/STM32 연결을 점검하는 **예시**:

```bash
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg
```

- ST-Link 펌웨어/설치 방식에 따라 인터페이스 설정 파일 이름이나 경로가 다를 수 있다.
- OpenOCD 실행 중 별도 터미널에서 GDB를 연결하거나, OpenOCD 명령으로 펌웨어를 기록할 수 있다.
- 동시에 둘 이상의 프로그램이 같은 ST-Link를 독점적으로 제어하려 하면 연결이 실패할 수 있다.

## 8. 무엇을 선택하면 좋을까?

| 목적 | 추천 구성 |
|---|---|
| STM32를 빠르게 설정하고 시작 | STM32CubeIDE(필요한 CubeMX 기능 통합) |
| VSCode에서 C 레지스터 실습 | ARM GCC + Make + OpenOCD + ST-Link |
| VSCode에서 HAL 설정 활용 | CubeMX로 코드 생성 + ARM GCC/CMake + OpenOCD + ST-Link |
| Rust에서 STM32 개발 | rustc/Cargo + probe-rs + ST-Link |

**한 줄 정리:** **CubeMX는 MCU 설정·코드 생성, GCC/rustc는 컴파일, OpenOCD/probe-rs는 다운로드·디버깅 지원, ST-Link는 실제 하드웨어 연결, STM32는 실행 대상**이다.
