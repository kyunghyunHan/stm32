# STM32 C

STM32 레지스터 제어를 연습하는 C 프로젝트다. 저장소 루트에서 Make 명령을 실행한다.
ARM 전용 `arm-none-eabi-gcc`와 STM32CubeF4의 CMSIS 헤더·시작 코드·링커 스크립트를 사용한다.

현재 Makefile은 **STM32F446xE**(Flash 512 KiB, RAM 128 KiB)를 대상으로 한다.
각 예제는 `main()`이 있어 하나씩 선택해 빌드한다.

| 예제 (`EXAMPLE`) | 소스 동작 |
| --- | --- |
| `led` (기본) | PA5 LED를 약 3초마다 반전 (16 MHz 기준) |
| `led_swicth` | PC13이 LOW이면 PA5 LED ON, HIGH이면 OFF |
| `can` | CAN1 내부 루프백, 디버거에서 수신 결과 확인 |

`led_swicth`는 기존 파일 이름을 그대로 사용한다. 버튼 극성과 핀 배선은 실제 보드에 맞춰 확인한다.

## 준비

- newlib가 포함된 Arm GNU Toolchain (`arm-none-eabi-gcc`, `arm-none-eabi-objcopy`, `arm-none-eabi-size`)
- STM32CubeF4 SDK (STM32CubeMX 등으로 설치)
- OpenOCD 및 ST-LINK (업로드할 때)
- clang-format (포맷 명령을 사용할 때)

macOS의 `/Applications/STM32CubeIDE.app`에 툴체인이 있으면 우선 사용한다.
그 외에는 PATH의 `arm-none-eabi-*`를 사용하며, `CROSS_COMPILE=/path/to/bin/arm-none-eabi-`로 지정할 수도 있다.
Homebrew의 GCC 단독 패키지에는 표준 헤더와 `nano.specs`가 없을 수 있으므로 완전한 툴체인이 필요하다.

SDK는 `~/STM32Cube/Repository/STM32Cube_FW_F4_V*`에서 자동 선택한다.
다른 위치에 설치했다면 경로를 지정한다.

```bash
make stm32-build CUBE_F4=/path/to/STM32Cube_FW_F4_V1.28.3
```

F446xE의 메모리 구성이 같은 CubeF4 F446ZE 템플릿 링커 스크립트를 사용한다.
필요하면 `LINKER_SCRIPT=/path/to/board.ld`로 지정할 수 있다.

## 명령

```bash
make stm32-check                       # 선택한 예제의 C 컴파일 검사
make stm32-build                       # led 펌웨어 링크까지 확인
make stm32-run                         # led를 ST-LINK로 업로드하고 실행
make stm32-build EXAMPLE=led_swicth
make stm32-run EXAMPLE=led_swicth
make stm32-build EXAMPLE=can
make check-all                         # 세 F446 예제 컴파일 검사
make lint                              # 세 F446 예제 경고를 오류로 검사
make fmt                               # 세 F446 예제 C 포맷 정리
make fmt-check                         # 세 F446 예제 C 포맷 검사
make clean                             # 빌드 결과 정리
make help                              # 전체 명령 안내
```

`make check`는 `make stm32-check`와 같다. 링크까지 검사하려면 `make stm32-build`를 사용한다.
빌드 결과는 `target/<예제>/`에 ELF, BIN, MAP 파일로 생성된다.

`make stm32-run`은 연결된 보드의 Flash에 펌웨어를 쓰고 재시작한다.
보드의 **ST-LINK USB 커넥터**를 연결하고, 실행 전 소스의 핀 설정과 보드 배선을 확인한다.

## F767 학습 소스

`interrupt.c`, `timer_counter.c`, `urtc.c`, `throttle.c`, `clock.c`는 F767 관련 학습 소스다.
현재 F446 빌드 대상에 포함하지 않는다. F767에서 사용하려면 해당 보드용 CubeF7 CMSIS,
Cortex-M7 옵션, 시작 코드, 메모리 구성에 맞는 링커 스크립트를 별도로 준비해야 한다.
`clock.c`와 `throttle.c`는 독립 실행용 `main()`이 없는 초기화 코드이며,
`register_offset.c`는 레지스터 주소·구조체 정의 예제다.

## ST-LINK

## Flash Memory
- 전원을 꺼도 저장된 내용이 유지되는 메모리
- 실행할 프로그램을 저장하는 공간으로 사용