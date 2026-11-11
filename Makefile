# STM32Cube SDK startup/drivers stay external; project code is selected explicitly.
.DEFAULT_GOAL := stm32-build
MCU ?= f767
ifneq ($(strip $(BOARD)),)
BOARD_MCU := $(if $(filter f446re,$(BOARD)),f446,$(if $(filter f429zi,$(BOARD)),f429,$(if $(filter f767vi,$(BOARD)),f767,invalid)))
ifneq ($(origin MCU),file)
ifneq ($(MCU),$(BOARD_MCU))
$(error BOARD=$(BOARD) conflicts with MCU=$(MCU))
endif
endif
MCU := $(BOARD_MCU)
endif
MODE ?= register
EXAMPLE ?= $(if $(filter f429,$(MCU)),gpio_button_toggle_debounce,$(if $(filter f446,$(MCU)),gpio_led_blink,adc_polling))
# MODE=team remains a compatibility spelling for F429 register examples.
IMPL := $(if $(and $(filter f429,$(MCU)),$(filter team,$(MODE))),register,$(MODE))
# Explicit example source paths; edit this table when adding a C example.
f446_register_EXAMPLES := can_silent_loopback gpio_led_blink i2c_oled_hello systick_interrupt_led_blink timer_polling_led_blink uart_button_led_sync uart_button_led_sync_blocking uart_button_level_tx uart_command_error_counters uart_rx_external_led uart_rx_increment_tx_polling uart_rx_led_control uart_rx_led_control_variant uart_three_node_hub
SOURCE_f446_register_can_silent_loopback := src/f446re/register/can_silent_loopback.c
SOURCE_f446_register_gpio_led_blink := src/f446re/register/gpio_led_blink.c
SOURCE_f446_register_i2c_oled_hello := src/f446re/register/i2c_oled_hello.c
SOURCE_f446_register_systick_interrupt_led_blink := src/f446re/register/systick_interrupt_led_blink.c
SOURCE_f446_register_timer_polling_led_blink := src/f446re/register/timer_polling_led_blink.c
SOURCE_f446_register_uart_button_led_sync := src/f446re/register/uart_button_led_sync.c
SOURCE_f446_register_uart_button_led_sync_blocking := src/f446re/register/uart_button_led_sync_blocking.c
SOURCE_f446_register_uart_button_level_tx := src/f446re/register/uart_button_level_tx.c
SOURCE_f446_register_uart_command_error_counters := src/f446re/register/uart_command_error_counters.c
SOURCE_f446_register_uart_rx_external_led := src/f446re/register/uart_rx_external_led.c
SOURCE_f446_register_uart_rx_increment_tx_polling := src/f446re/register/uart_rx_increment_tx_polling.c
SOURCE_f446_register_uart_rx_led_control := src/f446re/register/uart_rx_led_control.c
SOURCE_f446_register_uart_rx_led_control_variant := src/f446re/register/uart_rx_led_control_variant.c
SOURCE_f446_register_uart_three_node_hub := src/f446re/register/uart_three_node_hub.c
SOURCE_f446_register_led := src/f446re/register/gpio_led_blink.c
SOURCE_f446_register_led_button := src/f429zi/register/gpio_button_toggle.c
f446_hal_EXAMPLES := uart_command_error_counters
SOURCE_f446_hal_uart_command_error_counters := src/f446re/hal/uart_command_error_counters.c
f429_register_EXAMPLES := can_loopback_led exti_both_edges_led exti_rising_led_toggle gpio_button_event_counter gpio_button_event_counter_variant gpio_button_toggle gpio_button_toggle_debounce gpio_button_toggle_open_drain gpio_button_toggle_raw gpio_external_button_led gpio_led_blink gpio_led_morse_sos gpio_pc13_external_led gpio_pc13_toggle_open_drain systick_interrupt_uart_tx systick_pll_interrupt_uart_tx systick_pll_polling_uart_tx systick_polling_uart_tx timer_interrupt_polling_uart_tx timer_polling_uart_tx uart6_button_led_sync uart_button_led_report uart_button_led_sync uart_echo uart_echo_greeting uart_external_button_tx uart_heap_memory_dump uart_rx_three_led_select
SOURCE_f429_register_exti_both_edges_led := src/f429zi/register/exti_both_edges_led.c
SOURCE_f429_register_gpio_button_event_counter := src/f429zi/register/gpio_button_event_counter.c
SOURCE_f429_register_gpio_button_event_counter_variant := src/f429zi/register/gpio_button_event_counter_variant.c
SOURCE_f429_register_gpio_button_toggle_debounce := src/f429zi/register/gpio_button_toggle_debounce.c
SOURCE_f429_register_gpio_button_toggle_raw := src/f429zi/register/gpio_button_toggle_raw.c
SOURCE_f429_register_gpio_led_blink := src/f429zi/register/gpio_led_blink.c
SOURCE_f429_register_gpio_led_morse_sos := src/f429zi/register/gpio_led_morse_sos.c
SOURCE_f429_register_systick_interrupt_uart_tx := src/f429zi/register/systick_interrupt_uart_tx.c
SOURCE_f429_register_systick_pll_interrupt_uart_tx := src/f429zi/register/systick_pll_interrupt_uart_tx.c
SOURCE_f429_register_systick_pll_polling_uart_tx := src/f429zi/register/systick_pll_polling_uart_tx.c
SOURCE_f429_register_systick_polling_uart_tx := src/f429zi/register/systick_polling_uart_tx.c
SOURCE_f429_register_timer_interrupt_polling_uart_tx := src/f429zi/register/timer_interrupt_polling_uart_tx.c
SOURCE_f429_register_timer_polling_uart_tx := src/f429zi/register/timer_polling_uart_tx.c
SOURCE_f429_register_uart6_button_led_sync := src/f429zi/register/uart6_button_led_sync.c
SOURCE_f429_register_uart_echo := src/f429zi/register/uart_echo.c
SOURCE_f429_register_uart_echo_greeting := src/f429zi/register/uart_echo_greeting.c
SOURCE_f429_register_uart_heap_memory_dump := src/f429zi/register/uart_heap_memory_dump.c
SOURCE_f429_register_led_button := src/f429zi/register/gpio_button_toggle_debounce.c
SOURCE_f429_register_gpio_button_toggle := src/f429zi/register/gpio_button_toggle.c
SOURCE_f429_register_gpio_external_button_led := src/f429zi/register/gpio_external_button_led.c
SOURCE_f429_register_gpio_pc13_external_led := src/f429zi/register/gpio_pc13_external_led.c
SOURCE_f429_register_gpio_button_toggle_open_drain := src/f429zi/register/gpio_button_toggle_open_drain.c
SOURCE_f429_register_gpio_pc13_toggle_open_drain := src/f429zi/register/gpio_pc13_toggle_open_drain.c
SOURCE_f429_register_exti_rising_led_toggle := src/f429zi/register/exti_rising_led_toggle.c
SOURCE_f429_register_uart_rx_three_led_select := src/f429zi/register/uart_rx_three_led_select.c
SOURCE_f429_register_uart_external_button_tx := src/f429zi/register/uart_external_button_tx.c
SOURCE_f429_register_uart_button_led_report := src/f429zi/register/uart_button_led_report.c
SOURCE_f429_register_uart_button_led_sync := src/f429zi/register/uart_button_led_sync.c
SOURCE_f429_register_can_loopback_led := src/f429zi/register/can_loopback_led.c
f429_hal_EXAMPLES :=
f767_register_EXAMPLES := adc_polling exti_button_led_toggle timer_interrupt_led_toggle uart_rx_increment_tx_polling
SOURCE_f767_register_adc_polling := src/f767vi/register/adc_polling.c
SOURCE_f767_register_exti_button_led_toggle := src/f767vi/register/exti_button_led_toggle.c
SOURCE_f767_register_timer_interrupt_led_toggle := src/f767vi/register/timer_interrupt_led_toggle.c
SOURCE_f767_register_uart_rx_increment_tx_polling := src/f767vi/register/uart_rx_increment_tx_polling.c
SOURCE_f767_register_adc := src/f767vi/register/adc_polling.c
SOURCE_f767_register_interrupt := src/f767vi/register/exti_button_led_toggle.c
SOURCE_f767_register_led := src/f446re/register/gpio_led_blink.c
SOURCE_f767_register_led_button := src/f429zi/register/gpio_button_toggle.c
SOURCE_f767_register_timer_counter := src/f767vi/register/timer_interrupt_led_toggle.c
SOURCE_f767_register_urtc := src/f767vi/register/uart_rx_increment_tx_polling.c
f767_hal_EXAMPLES := adc_polling gpio_led_blink
SOURCE_f767_hal_adc_polling := src/f767vi/hal/adc_polling.c
SOURCE_f767_hal_gpio_led_blink := src/f767vi/hal/gpio_led_blink.c
SOURCE_f767_hal_adc := src/f767vi/hal/adc_polling.c
SOURCE_f767_hal_led := src/f767vi/hal/gpio_led_blink.c

EXAMPLES := $($(MCU)_$(IMPL)_EXAMPLES)
APP_SOURCE := $(SOURCE_$(MCU)_$(IMPL)_$(EXAMPLE))
CUBE_GCC := $(lastword $(sort $(wildcard /Applications/STM32CubeIDE.app/Contents/Eclipse/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.*/tools/bin/arm-none-eabi-gcc)))
CROSS_COMPILE ?= $(if $(CUBE_GCC),$(patsubst %gcc,%,$(CUBE_GCC)),arm-none-eabi-)
CC := $(CROSS_COMPILE)gcc
OBJCOPY := $(CROSS_COMPILE)objcopy
SIZE := $(CROSS_COMPILE)size
CUBE_F4 ?= $(lastword $(sort $(wildcard $(HOME)/STM32Cube/Repository/STM32Cube_FW_F4_V*)))
CUBE_F7 ?= $(lastword $(sort $(wildcard $(HOME)/STM32Cube/Repository/STM32Cube_FW_F7_V* $(HOME)/STM32Cube/Repository/STM32CubeF7)))
ifeq ($(MCU),f767)
SDK := $(CUBE_F7)
FAMILY := f7
DEVICE_NAME := stm32f767xx
DEVICE_DEFINE := STM32F767xx
LINKER_SCRIPT ?= system/STM32F767VI_FLASH.ld
SYSTEM := system/system_stm32f7xx.c
ARCH_FLAGS := -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard
PROJECT_CPPFLAGS := -DUSER_VECT_TAB_ADDRESS
# Preserve the existing F767 artifact locations.
BUILD_DIR ?= target/$(MODE)/$(EXAMPLE)
else
SDK := $(CUBE_F4)
FAMILY := f4
DEVICE_NAME := stm32$(MCU)xx
DEVICE_DEFINE := STM32$(shell echo $(MCU) | tr a-z A-Z)xx
LINKER_SCRIPT ?= system/STM32$(if $(filter f446,$(MCU)),F446RE,F429ZI)_FLASH.ld
SYSTEM = $(DEVICE)/Source/Templates/system_stm32f4xx.c
ARCH_FLAGS := -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
BUILD_DIR ?= target/$(MCU)/$(MODE)/$(EXAMPLE)
endif
FAMILY_UPPER := $(shell echo $(FAMILY) | tr a-z A-Z)
DEVICE := $(SDK)/Drivers/CMSIS/Device/ST/STM32$(FAMILY_UPPER)xx
HAL_DIR := $(SDK)/Drivers/STM32$(FAMILY_UPPER)xx_HAL_Driver
STARTUP := $(DEVICE)/Source/Templates/gcc/startup_$(DEVICE_NAME).s
TARGET := $(BUILD_DIR)/$(EXAMPLE)
SOURCES := $(APP_SOURCE) $(SYSTEM) system/runtime.c
PROJECT_CPPFLAGS += -D$(DEVICE_DEFINE) -I$(SDK)/Drivers/CMSIS/Include -I$(DEVICE)/Include
CONFIG_FILES = $(DEVICE)/Include/$(DEVICE_NAME).h $(SDK)/Drivers/CMSIS/Include/core_cm$(if $(filter f767,$(MCU)),7,4).h
HAL_MODULES := hal hal_cortex hal_gpio hal_rcc hal_rcc_ex hal_pwr hal_pwr_ex hal_flash hal_flash_ex
ifeq ($(IMPL),hal)
HAL_SUPPORT := system
PROJECT_CPPFLAGS += -DUSE_HAL_DRIVER -I$(HAL_SUPPORT) -I$(HAL_DIR)/Inc -I$(HAL_DIR)/Inc/Legacy
CONFIG_FILES += $(HAL_SUPPORT)/stm32$(FAMILY)xx_hal_conf.h
SOURCES += $(HAL_SUPPORT)/stm32$(FAMILY)xx_it.c $(wildcard $(HAL_SUPPORT)/stm32$(FAMILY)xx_hal_msp.c)
ifneq ($(filter adc adc_polling,$(EXAMPLE)),)
PROJECT_CPPFLAGS += -DPROJECT_HAL_ADC
HAL_MODULES += hal_adc hal_adc_ex hal_dma hal_dma_ex
endif
ifeq ($(MCU),f446)
HAL_MODULES += hal_uart hal_dma hal_dma_ex
endif
SOURCES += $(addprefix $(HAL_DIR)/Src/stm32$(FAMILY)xx_,$(addsuffix .c,$(HAL_MODULES)))
endif
PROJECT_CFLAGS := $(ARCH_FLAGS) -std=c11 -Wall -Wextra -Wpedantic -Og -g3 -ffunction-sections -fdata-sections
PROJECT_LDFLAGS := $(ARCH_FLAGS) -T$(LINKER_SCRIPT) -nostartfiles --specs=nano.specs --specs=nosys.specs -Wl,--gc-sections -Wl,--fatal-warnings -Wl,-Map=$(TARGET).map
CLANG_FORMAT ?= clang-format
FORMAT_SOURCES := $(shell find src system -name '*.c' -o -name '*.h')
.PHONY: check-config stm32-check check stm32-build check-all build-all check-boards build-boards clean help lint fmt fmt-check check-snippets list
check-config:
	@case "$(MCU)" in f446|f429|f767) ;; *) echo 'ERROR: BOARD=f446re|f429zi|f767vi or MCU=f446|f429|f767'; exit 1;; esac
	@case "$(IMPL)" in register|hal) ;; *) echo 'ERROR: MODE=register|hal (legacy F429: team)'; exit 1;; esac
	@test -n "$(APP_SOURCE)" || { echo "ERROR: unknown example $(EXAMPLE); use make list BOARD=$(BOARD) MCU=$(MCU) MODE=$(MODE)"; exit 1; }
	@command -v "$(CC)" >/dev/null || { echo "ERROR: ARM GCC not found: $(CC)"; exit 1; }
	@for file in $(CONFIG_FILES) $(STARTUP) $(LINKER_SCRIPT) $(SOURCES); do test -f "$$file" || { echo "ERROR: missing $$file; set CUBE_F4/CUBE_F7"; exit 1; }; done
stm32-check: check-config
	$(CC) $(PROJECT_CPPFLAGS) $(CPPFLAGS) $(PROJECT_CFLAGS) $(CFLAGS) $(EXTRA_WARNINGS) -fsyntax-only $(SOURCES)
check: stm32-check
stm32-build: check-config
	@mkdir -p $(BUILD_DIR)
	$(CC) $(PROJECT_CPPFLAGS) $(CPPFLAGS) $(PROJECT_CFLAGS) $(CFLAGS) $(EXTRA_WARNINGS) $(SOURCES) $(STARTUP) $(PROJECT_LDFLAGS) $(LDFLAGS) -o $(TARGET).elf
	$(OBJCOPY) -O binary $(TARGET).elf $(TARGET).bin
	$(SIZE) $(TARGET).elf
check-snippets:
	$(MAKE) stm32-check MCU=f767 BOARD= MODE=register EXAMPLE=adc_polling APP_SOURCE=src/f767vi/register/clock_pll_init.c CPPFLAGS='$(CPPFLAGS) -include stm32f7xx.h'
	$(MAKE) stm32-check MCU=f767 BOARD= MODE=register EXAMPLE=adc_polling APP_SOURCE=src/f429zi/register/gpio_register_layout.c
check-all build-all:
	@set -e; for mode in register hal; do for example in $(foreach m,register hal,$(addprefix $(m):,$($(MCU)_$(m)_EXAMPLES))); do \
	 case "$$example" in $$mode:*) name=$${example#*:}; $(MAKE) --no-print-directory $(if $(filter check-all,$@),stm32-check,stm32-build) MODE=$$mode EXAMPLE=$$name;; esac; \
	 done; done
check-boards build-boards:
	@set -e; for board in f446re f429zi f767vi; do $(MAKE) --no-print-directory $(if $(filter check-boards,$@),check-all,build-all) BOARD=$$board; done
lint:
	$(MAKE) check-all EXTRA_WARNINGS=-Werror
list:
	@echo '$(MCU) / $(IMPL): $(EXAMPLES)'
fmt:
	$(CLANG_FORMAT) -i $(FORMAT_SOURCES)
fmt-check:
	$(CLANG_FORMAT) --dry-run --Werror $(FORMAT_SOURCES)
clean:
	rm -rf target
help:
	@echo 'make BOARD=f429zi EXAMPLE=uart_echo [MODE=register|hal]'
	@echo 'make list | build-all | build-boards | check-all | check-boards | check-snippets'
	@echo 'SDK overrides: CUBE_F4=/path CUBE_F7=/path CROSS_COMPILE=arm-none-eabi-'
