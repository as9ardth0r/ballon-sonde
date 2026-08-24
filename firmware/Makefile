TARGET = hab_payload
BUILD_DIR = build

CC = arm-none-eabi-gcc
SIZE = arm-none-eabi-size

MCU_FLAGS = -mcpu=cortex-m0plus -mthumb

C_SOURCES = \
	Core/Src/main.c \
	Core/Src/clock.c \
	Core/Src/gpio.c \
	Core/Src/i2c_bus.c \
	Core/Src/spi1_bus.c \
	Core/Src/bme280.c \
	Core/Src/gps_uart.c \
	Core/Src/lora_sx1276.c \
	Core/Src/telemetry.c \
	Core/Src/nmea.c \
	Core/Src/ubx.c \
	Core/Src/syscalls.c \
	Core/Src/system_stm32l0xx.c

ASM_SOURCES = startup/startup_stm32l052xx.s

C_INCLUDES = -ICore/Inc -IDrivers -IDrivers/CMSIS_Core
C_DEFS = -DSTM32L052xx

CFLAGS = $(MCU_FLAGS) $(C_DEFS) $(C_INCLUDES) -Wall -Wextra -O2 -std=c11 -ffunction-sections -fdata-sections -g

LDSCRIPT = startup/STM32L052K8Tx_FLASH.ld
# -u _printf_float : active le support des flottants dans printf/snprintf
# sous newlib-nano (désactivé par défaut avec -specs=nano.specs) —
# nécessaire pour tm_encode() (telemetry.c) qui utilise %.6f/%.1f.
LDFLAGS = $(MCU_FLAGS) -specs=nano.specs -u _printf_float -T$(LDSCRIPT) -Wl,--gc-sections -Wl,-Map=$(BUILD_DIR)/$(TARGET).map -lm

OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

all: $(BUILD_DIR)/$(TARGET).elf

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(CC) -c $(MCU_FLAGS) -x assembler-with-cpp $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SIZE) $@

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
