# 기반 코드·설정·문서 전수 목록

펌웨어 61개는 [전체 분석](INVENTORY.md), 나머지 원본 29개 파일은 아래와 같습니다. Git 내부·target/·외부 Cube SDK는 저장소 소스 집계에서 제외했습니다.

| 이전 경로 | 현재 경로 | 역할 및 처리 |
|---|---|---|
| `.cargo/config.toml` | `.cargo/config.toml` | ARM hard-float target 및 -Tlink.x, f446/f767 alias; 변경 없음 |
| `.gitignore` | `.gitignore` | target/ 및 SDK/펌웨어 산출물 제외; 변경 없음 |
| `.vscode/c_cpp_properties.json` | `.vscode/c_cpp_properties.json` | tasks/F5/probe-rs 및 C IntelliSense; F767 HAL include 경로 갱신, 기존 binary 경로 유지 |
| `.vscode/extensions.json` | `.vscode/extensions.json` | tasks/F5/probe-rs 및 C IntelliSense; F767 HAL include 경로 갱신, 기존 binary 경로 유지 |
| `.vscode/launch.json` | `.vscode/launch.json` | tasks/F5/probe-rs 및 C IntelliSense; F767 HAL include 경로 갱신, 기존 binary 경로 유지 |
| `.vscode/tasks.json` | `.vscode/tasks.json` | tasks/F5/probe-rs 및 C IntelliSense; F767 HAL include 경로 갱신, 기존 binary 경로 유지 |
| `Cargo.lock` | `Cargo.lock` | 기존 Rust 의존성 잠금; 변경 없음 |
| `Cargo.toml` | `Cargo.toml` | MCU feature와 4개 binary; 기존 이름 유지, 소스 경로 갱신 |
| `Makefile` | `Makefile` | C 예제 선택, SDK/MCU/flags/startup/linker, ELF/BIN 생성; 경로와 선택 체계 갱신 |
| `README.md` | `README.md` | 프로젝트 구조·상태·명령·보고서 안내로 확장 |
| `STM32F429ZI_FLASH.ld` | `system/STM32F429ZI_FLASH.ld` | C startup용 섹션과 heap/stack 배치; 파일 내용 보존, Makefile 경로 갱신 |
| `STM32F446RE_FLASH.ld` | `system/STM32F446RE_FLASH.ld` | C startup용 섹션과 heap/stack 배치; 파일 내용 보존, Makefile 경로 갱신 |
| `STM32F767VI_FLASH.ld` | `system/STM32F767VI_FLASH.ld` | C startup용 섹션과 heap/stack 배치; 파일 내용 보존, Makefile 경로 갱신 |
| `build.rs` | `build.rs` | 정확히 하나의 feature 및 ARM 타깃 검사, MCU memory.x 복사; 경로 갱신 |
| `cache.md` | `notes/cache.md` | 빈 학습 메모; notes로 보존 |
| `docs/BUILD_RUN.md` | `docs/BUILD_RUN.md` | 기존 기술 문서; 실제 파일 이동 경로 참조 갱신, 원문 학습 내용 보존 |
| `docs/REGISTER_LEARNING.md` | `docs/REGISTER_LEARNING.md` | 기존 기술 문서; 실제 파일 이동 경로 참조 갱신, 원문 학습 내용 보존 |
| `docs/register.md` | `docs/register.md` | 기존 기술 문서; 실제 파일 이동 경로 참조 갱신, 원문 학습 내용 보존 |
| `docs/stm32_env.md` | `docs/stm32_env.md` | 기존 기술 문서; 실제 파일 이동 경로 참조 갱신, 원문 학습 내용 보존 |
| `include/stm32f7xx_hal_conf.h` | `system/stm32f7xx_hal_conf.h` | F767 HAL GPIO/RCC/Flash/PWR/Cortex 및 조건부 ADC/DMA; 내용 보존 |
| `memory/f446.x` | `system/f446.x` | Rust Flash/RAM 배치; 내용 보존, build.rs와 active-file 복사 경로 갱신 |
| `memory/f767.x` | `system/f767.x` | Rust Flash/RAM 배치; 내용 보존, build.rs와 active-file 복사 경로 갱신 |
| `scripts/build_active_f446.py` | 사용자 요청으로 삭제 | Python 제거; active-f446는 Cargo target으로 선택 |
| `src/hal/stm32f7xx_hal_msp.c` | `system/stm32f7xx_hal_msp.c` | HAL PWR/SYSCFG 및 ADC1 PA7 analog 초기화; 내용 보존 |
| `src/hal/stm32f7xx_it.c` | `system/stm32f7xx_it.c` | HAL 전용 SysTick_Handler: HAL_IncTick/HAL_SYSTICK_IRQHandler; Register와 함께 링크하지 않음 |
| `src/stm32.md` | `docs/gpio_register_notes.md` | 기존 기술 문서; 실제 파일 이동 경로 참조 갱신, 원문 학습 내용 보존 |
| `src/system/CMSIS_DEVICE_LICENSE.md` | `system/CMSIS_DEVICE_LICENSE.md` | CMSIS 장치 라이선스 원문; system 파일 옆에 보존 |
| `src/system/runtime.c` | `system/runtime.c` | -nostartfiles용 _init/_fini; 세 MCU가 실제로 공유 |
| `src/system/system_stm32f7xx.c` | `system/system_stm32f7xx.c` | ST CMSIS SystemInit/FPU/VTOR/SystemCoreClockUpdate; 라이선스와 내용 보존 |

외부 의존성: STM32CubeF4/F7의 CMSIS core/device 헤더, MCU별 GCC startup Assembly, F4 system template, 선택한 HAL 모듈. 저장소에 별도 Assembly 파일·CubeIDE 생성 프로젝트·플래시 스크립트는 없었습니다. OpenOCD/probe-rs 명령은 문서와 Makefile/VS Code 설정으로 제공합니다.
