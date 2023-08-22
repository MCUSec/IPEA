TARGET_TRIPLE = arm-none-eabi
PROJECT_DIR = .
TOOLCHAIN_PATH = /home/jiameng/Projects/LLVM-embedded-toolchain-for-Arm/install-13.0.0/LLVMEmbeddedToolchainForArm-13.0.0
CC = arm-none-eabi-gcc
AS = arm-none-eabi-gcc 
LD = arm-none-eabi-gcc
OBJCOPY = $(TARGET_TRIPLE)-objcopy

BUILD_DIR = build

#ARCH = -march=armv7e-m -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
ARCH = --config armv7em_hard_fpv4_sp_d16_nosys.cfg
LLVM_PASS = -flegacy-pass-manager -Xclang -load -Xclang ../llvmpass/build/SanitizerPass/libSanitizerPass.so

ASAN = -fsanitize=kernel-address --param asan-globals=1 --param asan-stack=1 -fasan-shadow-offset=0x1c02a000

DEFS += -D_DEBUG=1 \
		-DDEBUG \
		-DCPU_MK64FN1M0VLL12 \
		-DCPU_MK64FN1M0VLL12_cm4 \
		-DFSL_RTOS_BM \
		-DFRDM_K64F \
		-DFREEDOM \
		-DFSL_OSA_BM_TASK_ENABLE=0 \
		-DFSL_OSA_BM_TIMER_CONFIG=0 \
		-DSDK_DEBUGCONSOLE=1 \
		-DSERIAL_PORT_TYPE_UART=1 \
		-DMCUXPRESSO_SDK \
		-DMBEDTLS_CONFIG_FILE='"ksdk_mbedtls_config.h"' \
		-DENABLE_PROFILE


INCLUDES += -I$(PROJECT_DIR)/board 
INCLUDES += -I$(PROJECT_DIR)/CMSIS 
INCLUDES += -I$(PROJECT_DIR)/component/lists 
INCLUDES += -I$(PROJECT_DIR)/component/uart 
INCLUDES += -I$(PROJECT_DIR)/component/serial_manager
INCLUDES += -I$(PROJECT_DIR)/mbedtls/include 
INCLUDES += -I$(PROJECT_DIR)/mbedtls/library
INCLUDES += -I$(PROJECT_DIR)/mmcau
INCLUDES += -I$(PROJECT_DIR)/mbedtls/port/ksdk
INCLUDES += -I$(PROJECT_DIR)/device 
INCLUDES += -I$(PROJECT_DIR)/drivers 
INCLUDES += -I$(PROJECT_DIR)/utilities

CFLAGS = $(DEFS) -Os -g -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mthumb -g -std=gnu99 -fshort-enums -ffunction-sections -fdata-sections -ffreestanding -fno-builtin -fno-exceptions -finstrument-functions
ASFLAGS = -x assembler-with-cpp -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mthumb
LDFLAGS = -mcpu=cortex-m4 \
			-mfloat-abi=hard \
			-mfpu=fpv4-sp-d16 \
			--specs=nano.specs \
			--specs=nosys.specs \
			-Xlinker --gc-sections \
			-Xlinker -static \
			-Xlinker -z \
			-Xlinker muldefs \
			-L./mmcau/asm-cm4-cm7 \
			-L$(MCU_SANITIZER_WORKDIR)/McuASAN \
			-Wl,--start-group \
			-l_mmcau \
			-lmcuasan \
			-Wl,--end-group \
			-Wl,--print-memory-usage \
			-Xlinker --defsym=__stack_size__=0x2000 \
			-Xlinker --defsym=__heap_size__=0x2000 \
			-static \
			-T ldscript/MK64FN1M0xxx12_flash.ld

C_SOURCE_DIR = $(PROJECT_DIR)/source
C_SOUPPORT_DIRS = $(filter-out $(PROJECT_DIR) $(C_SOURCE_DIR), $(shell find $(PROJECT_DIR) -maxdepth 3 -type d))

C_SOURCE_FILES = $(wildcard $(C_SOURCE_DIR)/*.c)
C_SUPPORT_FILES = $(foreach dir, $(C_SOUPPORT_DIRS), $(wildcard $(dir)/*.c))
ASM_SUPPORT_FILES = startup/startup_MK64F12.S

C_SOURCE_OBJECTS = $(C_SOURCE_FILES:.c=.o)
C_SUPPORT_OBJECTS = $(C_SUPPORT_FILES:.c=.o)
ASM_SUPPORT_OBJECTS = $(ASM_SUPPORT_FILES:.S=.o)

RTT_DEP = $(wildcard ../librtt/*.o)

TARGET = PinLock.elf

.PHONY: all build clean

all: build $(TARGET)

build:
	@if [ ! -d $(BUILD_DIR) ]; then \
		mkdir -p $(BUILD_DIR); \
	fi

$(TARGET): $(C_SUPPORT_OBJECTS) $(ASM_SUPPORT_OBJECTS) $(C_SOURCE_OBJECTS)
	@echo "LD $@"
	$(LD) $(BUILD_DIR)/*.o $(LDFLAGS) -o $(TARGET).elf
	@$(OBJCOPY) -O binary $(TARGET).elf $(TARGET).bin

$(C_SUPPORT_OBJECTS): %.o: %.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) $(ASAN) $(INCLUDES) -c $^ -o $(BUILD_DIR)/$(notdir $@)

$(ASM_SUPPORT_OBJECTS): %.o: %.S
	@echo "AS $<"
	@$(AS) $(ASFLAGS) -c $^ -o $(BUILD_DIR)/$(notdir $@)

$(C_SOURCE_OBJECTS): %.o: %.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) $(ASAN) $(INCLUDES) -c $^ -o $(BUILD_DIR)/$(notdir $@)

clean:
	rm -f $(BUILD_DIR)/*.o $(TARGET).elf

