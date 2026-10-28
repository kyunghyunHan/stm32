# 빌드 검증 기록

검증일: 2026-10-28 (Asia/Seoul)

툴체인: `arm-none-eabi-gcc (GNU Tools for STM32 14.3.rel1.20251027-0700) 14.3.1 20250623`

외부 SDK 두 경로에서 전체 빌드 성공:

- `~/STM32Cube/Repository/STM32Cube_FW_F7_V1.17.4`
- `~/STM32Cube/Repository/STM32CubeF7`

두 경로가 서로 다른 SDK 버전이라는 의미는 아니다.

검증한 명령:

```bash
make clean
make check-all
make build-all
make stm32-build MODE=register EXAMPLE=led
make stm32-build MODE=hal EXAMPLE=led
make stm32-build MODE=register EXAMPLE=adc
make stm32-build MODE=hal EXAMPLE=adc
make lint
```

모두 종료 코드 0. 컴파일·링크 경고 없음. 링크 경고도 실패로 처리했다.

| MODE | EXAMPLE | text (B) | data (B) | bss (B) | 결과 |
|---|---|---:|---:|---:|---|
| hal | adc | 3408 | 12 | 2572 | 성공 |
| hal | led | 1812 | 12 | 2564 | 성공 |
| register | adc | 1208 | 0 | 2568 | 성공 |
| register | interrupt | 1236 | 0 | 2560 | 성공 |
| register | led | 1004 | 4 | 2564 | 성공 |
| register | timer_counter | 1212 | 0 | 2560 | 성공 |
| register | urtc | 1192 | 0 | 2568 | 성공 |

`bss` 열에는 NOLOAD heap/stack 최소 예약 공간도 포함된다.

추가 확인:

- ELF 7개의 main/SystemInit, hard-float ABI 및 BIN 벡터 테이블 확인.
- 초기 SP = 0x20020000, Reset 벡터는 Flash 범위의 Thumb 주소.
- HAL의 SysTick_Handler/HAL_MspInit은 프로젝트의 strong 심볼이며,
  HAL ADC의 HAL_ADC_MspInit도 strong 심볼. SysTick 벡터가 해당 핸들러를 가리킨다.
- 레지스터 빌드에 HAL_Init이 없으며 SysTick은 startup의 weak 기본 핸들러다.
- 잘못된 MODE, HAL에서 미지원 EXAMPLE, 존재하지 않는 SDK 경로는 예상대로 실패.
- 이동한 기존 C 파일 10개의 내용은 Git HEAD 원본과 바이트 단위로 동일.
- Core 디렉터리 없는 상태로 모든 검사·빌드 완료.

첫 링크에서는 `-nostartfiles`에 따른 `_init` 누락이 발견되어
`src/system/runtime.c`에 newlib 초기화 진입점을 추가하고 전체 재검증했다.

기존 Core/Src/main.c와 링커 스크립트 원문은 작업 시작 시 없었다.
남아 있던 target/core/core.map의 Flash 2 MiB / DTCM 128 KiB 배치를 확인했다.

Flash 기록 및 실제 보드 실행은 수행하지 않았다.
