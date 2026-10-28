# STM32F767VIT6: external STM32CubeF7 CMSIS/HAL, no CubeMX tree required.
.DEFAULT_GOAL := stm32-build
CUBE_GCC := $(lastword $(sort $(wildcard /Applications/STM32CubeIDE.app/Contents/Eclipse/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.*/tools/bin/arm-none-eabi-gcc)))
CROSS_COMPILE ?= $(if $(CUBE_GCC),$(patsubst %gcc,%,$(CUBE_GCC)),arm-none-eabi-)
CC := $(CROSS_COMPILE)gcc
OBJCOPY := $(CROSS_COMPILE)objcopy
SIZE := $(CROSS_COMPILE)size
SDK_SEARCH := $(sort $(wildcard $(HOME)/STM32Cube/Repository/STM32Cube_FW_F7_V* $(HOME)/STM32Cube/Repository/STM32CubeF7))
CUBE_F7 ?= $(lastword $(SDK_SEARCH))
DEVICE := $(CUBE_F7)/Drivers/CMSIS/Device/ST/STM32F7xx
HAL_DIR := $(CUBE_F7)/Drivers/STM32F7xx_HAL_Driver
STARTUP := $(DEVICE)/Source/Templates/gcc/startup_stm32f767xx.s
LINKER_SCRIPT ?= STM32F767VI_FLASH.ld
MODE ?= register
EXAMPLE ?= adc
MODES := register hal
REGISTER_EXAMPLES := led adc interrupt timer_counter urtc
HAL_EXAMPLES := led adc
EXAMPLES := $(if $(filter register,$(MODE)),$(REGISTER_EXAMPLES),$(HAL_EXAMPLES))
APP_SOURCE := src/$(MODE)/$(EXAMPLE).c
SOURCES := $(APP_SOURCE) src/system/system_stm32f7xx.c src/system/runtime.c
BUILD_DIR := target/$(MODE)/$(EXAMPLE)
TARGET := $(BUILD_DIR)/$(EXAMPLE)
ARCH_FLAGS := -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard
PROJECT_CPPFLAGS += -DSTM32F767xx -DUSER_VECT_TAB_ADDRESS -Iinclude \
 -I$(CUBE_F7)/Drivers/CMSIS/Include -I$(DEVICE)/Include
PROJECT_CFLAGS += $(ARCH_FLAGS) -std=c11 -Wall -Wextra -Wpedantic -Og -g3 -ffunction-sections -fdata-sections
HAL_MODULES := hal hal_cortex hal_gpio hal_rcc hal_rcc_ex hal_pwr hal_pwr_ex hal_flash hal_flash_ex
ifeq ($(MODE),hal)
PROJECT_CPPFLAGS += -DUSE_HAL_DRIVER -I$(HAL_DIR)/Inc -I$(HAL_DIR)/Inc/Legacy
SOURCES += src/hal/stm32f7xx_it.c src/hal/stm32f7xx_hal_msp.c
ifeq ($(EXAMPLE),adc)
PROJECT_CPPFLAGS += -DPROJECT_HAL_ADC
HAL_MODULES += hal_adc hal_adc_ex hal_dma hal_dma_ex
endif
SOURCES += $(addprefix $(HAL_DIR)/Src/stm32f7xx_,$(addsuffix .c,$(HAL_MODULES)))
endif
# SDK startup calls __libc_init_array; omit generic crt0 that assumes a hosted main.
PROJECT_LDFLAGS += $(ARCH_FLAGS) -T$(LINKER_SCRIPT) -nostartfiles --specs=nano.specs --specs=nosys.specs \
 -Wl,--gc-sections -Wl,--fatal-warnings -Wl,-Map=$(TARGET).map
CLANG_FORMAT ?= clang-format
FORMAT_SOURCES := $(shell find src include -name '*.c' -o -name '*.h')
.PHONY: check-config stm32-check check stm32-build check-all build-all clean help lint fmt fmt-check check-snippets
check-config:
	@case " $(MODES) " in *" $(MODE) "*) ;; *) echo "ERROR: MODE must be one of: $(MODES)"; exit 1;; esac
	@case " $(EXAMPLES) " in *" $(EXAMPLE) "*) ;; *) echo "ERROR: $(MODE) examples: $(EXAMPLES)"; exit 1;; esac
	@command -v "$(CC)" >/dev/null || { echo "ERROR: ARM GCC not found: $(CC)"; exit 1; }
	@for file in $(DEVICE)/Include/stm32f767xx.h $(CUBE_F7)/Drivers/CMSIS/Include/core_cm7.h $(STARTUP) $(LINKER_SCRIPT) $(SOURCES) include/stm32f7xx_hal_conf.h; do \
	 test -f "$$file" || { echo "ERROR: missing $$file (check CUBE_F7=$(CUBE_F7))"; exit 1; }; done
stm32-check: check-config
	$(CC) $(PROJECT_CPPFLAGS) $(CPPFLAGS) $(PROJECT_CFLAGS) $(CFLAGS) $(EXTRA_WARNINGS) -fsyntax-only $(SOURCES)
check: stm32-check
# Deliberately rebuild each invocation so SDK, flags and header changes cannot leave stale binaries.
stm32-build: check-config
	@mkdir -p $(BUILD_DIR)
	$(CC) $(PROJECT_CPPFLAGS) $(CPPFLAGS) $(PROJECT_CFLAGS) $(CFLAGS) $(EXTRA_WARNINGS) $(SOURCES) $(STARTUP) $(PROJECT_LDFLAGS) $(LDFLAGS) -o $(TARGET).elf
	$(OBJCOPY) -O binary $(TARGET).elf $(TARGET).bin
	$(SIZE) $(TARGET).elf
check-snippets: check-config
	$(CC) $(PROJECT_CPPFLAGS) $(CPPFLAGS) $(PROJECT_CFLAGS) $(CFLAGS) $(EXTRA_WARNINGS) -include stm32f7xx.h -fsyntax-only src/register/clock.c
	$(CC) $(PROJECT_CFLAGS) $(CFLAGS) $(EXTRA_WARNINGS) -fsyntax-only src/register/register_offset.c
check-all: check-snippets
check-all build-all:
	@set -e; for mode in $(MODES); do \
	 if [ "$$mode" = register ]; then examples="$(REGISTER_EXAMPLES)"; else examples="$(HAL_EXAMPLES)"; fi; \
	 for example in $$examples; do \
	 echo "== $$mode/$$example =="; \
	 $(MAKE) --no-print-directory $(if $(filter check-all,$@),stm32-check,stm32-build) MODE=$$mode EXAMPLE=$$example; \
	 done; done
lint:
	$(MAKE) check-all EXTRA_WARNINGS=-Werror
fmt:
	$(CLANG_FORMAT) -i $(FORMAT_SOURCES)
fmt-check:
	$(CLANG_FORMAT) --dry-run --Werror $(FORMAT_SOURCES)
clean:
	rm -rf target
help:
	@echo "make stm32-build MODE=register|hal EXAMPLE=led|adc"
	@echo "Register also supports: interrupt timer_counter urtc"
	@echo "make check-all | build-all | lint | clean"
	@echo "SDK: CUBE_F7=$(CUBE_F7); toolchain: CROSS_COMPILE=$(CROSS_COMPILE)"
