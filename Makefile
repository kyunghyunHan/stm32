# macOS에서는 newlib가 포함된 CubeIDE 툴체인을 우선 사용한다.
CUBE_GCC := $(lastword $(sort $(wildcard /Applications/STM32CubeIDE.app/Contents/Eclipse/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.*/tools/bin/arm-none-eabi-gcc)))
CROSS_COMPILE ?= $(if $(CUBE_GCC),$(patsubst %gcc,%,$(CUBE_GCC)),arm-none-eabi-)
CC := $(CROSS_COMPILE)gcc
OBJCOPY := $(CROSS_COMPILE)objcopy
SIZE := $(CROSS_COMPILE)size
OPENOCD ?= openocd
CLANG_FORMAT ?= clang-format

# STM32F446xE: Flash 512 KiB, RAM 128 KiB
CUBE_F4 ?= $(lastword $(sort $(wildcard $(HOME)/STM32Cube/Repository/STM32Cube_FW_F4_V*)))
CMSIS := $(CUBE_F4)/Drivers/CMSIS
DEVICE := $(CMSIS)/Device/ST/STM32F4xx
STARTUP := $(DEVICE)/Source/Templates/gcc/startup_stm32f446xx.s
SYSTEM := $(DEVICE)/Source/Templates/system_stm32f4xx.c
LINKER_SCRIPT ?= $(CUBE_F4)/Projects/STM32F446ZE-Nucleo/Templates/STM32CubeIDE/STM32F446ZETX_FLASH.ld

EXAMPLE ?= led
EXAMPLES := led led_swicth can
BUILD_DIR := target/$(EXAMPLE)
TARGET := $(BUILD_DIR)/$(EXAMPLE)
ARCH_FLAGS := -mcpu=cortex-m4 -mthumb -mfloat-abi=soft
CPPFLAGS := -DSTM32F446xx -I$(DEVICE)/Include -I$(CMSIS)/Include
CFLAGS := $(ARCH_FLAGS) -std=c11 -Wall -Wextra -Wpedantic -Og -g3 -ffunction-sections -fdata-sections
LDFLAGS := $(ARCH_FLAGS) -T$(LINKER_SCRIPT) --specs=nano.specs --specs=nosys.specs -Wl,--gc-sections -Wl,-Map=$(TARGET).map
C_SOURCES := $(addprefix src/,$(addsuffix .c,$(EXAMPLES)))

.DEFAULT_GOAL := stm32-build
.PHONY: stm32-check stm32-build stm32-run check check-all check-config fmt fmt-check lint clean help

check-config:
	@command -v $(CC) >/dev/null || { echo "$(CC)가 필요합니다."; exit 1; }
	@test -f "$(DEVICE)/Include/stm32f446xx.h" || { echo "STM32CubeF4 SDK 경로를 지정하세요: make CUBE_F4=/path/to/STM32Cube_FW_F4_Vx"; exit 1; }
	@case " $(EXAMPLES) " in *" $(EXAMPLE) "*) ;; *) echo "지원 예제: $(EXAMPLES)"; exit 1 ;; esac

stm32-check: check-config
	$(CC) $(CPPFLAGS) $(CFLAGS) -fsyntax-only src/$(EXAMPLE).c

check: stm32-check

check-all:
	@set -e; for example in $(EXAMPLES); do $(MAKE) stm32-check EXAMPLE=$$example; done

# SDK/옵션 변경을 반영하도록 매번 링크한다. 각 예제는 main이 있어 별도로 빌드한다.
stm32-build: check-config
	@test -f "$(STARTUP)" -a -f "$(SYSTEM)" -a -f "$(LINKER_SCRIPT)" || { echo "SDK의 시작 코드, 시스템 코드, 링커 스크립트를 확인하세요."; exit 1; }
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) src/$(EXAMPLE).c $(SYSTEM) $(STARTUP) $(LDFLAGS) -o $(TARGET).elf
	$(OBJCOPY) -O binary $(TARGET).elf $(TARGET).bin
	$(SIZE) $(TARGET).elf

stm32-run: stm32-build
	$(OPENOCD) -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program $(TARGET).elf verify reset exit"

fmt:
	$(CLANG_FORMAT) -i $(C_SOURCES)

fmt-check:
	$(CLANG_FORMAT) --dry-run --Werror $(C_SOURCES)

lint: check-config
	$(CC) $(CPPFLAGS) $(CFLAGS) -Werror -fsyntax-only $(C_SOURCES)

clean:
	rm -rf $(addprefix target/,$(EXAMPLES))

help:
	@echo "STM32F446xE C 예제: $(EXAMPLES) (기본: led)"
	@echo "예제 선택: make <명령> EXAMPLE=can"
	@echo "컴파일 검사: make stm32-check / check"
	@echo "모든 F446 예제 검사: make check-all"
	@echo "펌웨어 빌드: make stm32-build"
	@echo "ST-LINK 업로드/실행: make stm32-run"
	@echo "C 포맷 적용/검사: make fmt / fmt-check"
	@echo "모든 F446 예제 경고 검사: make lint"
	@echo "빌드 결과 정리: make clean"
