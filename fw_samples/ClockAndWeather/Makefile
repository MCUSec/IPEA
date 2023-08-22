CC = clang
CXX = clang++
LD = arm-none-eabi-gcc
AS = arm-none-eabi-gcc

ARCH = --target=arm-none-eabi --sysroot=$(TOOLCHAIN_DIR)/arm-none-eabi -march=armv7e-m
PASS = -flegacy-pass-manager -Xclang -load -Xclang $(MCU_SANITIZER_WORKDIR)/llvmpass/build/skeleton/SkeletonPass.so

INCLUDES = -I./Config \
			-I./Core/Inc \
			-I./Drivers/BSP/Components/Common \
			-I./Drivers/BSP/Components/cs42l51 \
			-I./Drivers/BSP/Components/es_wifi \
			-I./Drivers/BSP/Components/ft5336 \
			-I./Drivers/BSP/Components/is42s32800j \
			-I./Drivers/BSP/Components/m24lr64 \
			-I./Drivers/BSP/Components/mfxtm32l152 \
			-I./Drivers/BSP/Components/mx25lm51245g \
			-I./Drivers/BSP/Components/ov5640 \
			-I./Drivers/BSP/Components/rk070er9427 \
			-I./Drivers/BSP/Components/s5kcag \
			-I./Drivers/BSP/STM32H7B3I-EVAL \
			-I./Drivers/CMSIS/Include \
			-I./Drivers/CMSIS/ST/STM32H7xx/Include \
			-I./Drivers/CMSIS/Device/ST/STM32H7xx/Include \
			-I./Drivers/STM32H7xx_HAL_Driver \
			-I./Drivers/STM32H7xx_HAL_Driver/Inc \
			-I./Drivers/STM32H7xx_HAL_Driver/Inc/Legacy \
			-I./Middlewares/ST/STM32_Network_Library/Includes \
			-I./Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS \
			-I$(MCU_SANITIZER_WORKDIR)/include \
			-I$(MCU_SANITIZER_WORKDIR)/include/target

C_SOURCES = $(wildcard Core/Src/*.c)
C_SOURCES += $(wildcard Drivers/BSP/Components/es_wifi/*.c)
C_SOURCES += $(wildcard Drivers/BSP/Components/cs42l51/*.c)
C_SOURCES += $(wildcard Drivers/BSP/Components/ft5336/*.c)
C_SOURCES += $(wildcard Drivers/BSP/Components/is42s32800j/*.c)
C_SOURCES += $(wildcard Drivers/BSP/Components/m24lr64/*.c)
C_SOURCES += $(wildcard Drivers/BSP/Components/mfxstm32l152/*.c)
C_SOURCES += $(wildcard Drivers/BSP/Components/mx25lm51245g/*.c)
C_SOURCES += $(wildcard Drivers/BSP/STM32H7B3I-EVAL/*.c)
C_SOURCES += $(wildcard Drivers/STM32H7xx_HAL_Driver/Src/*.c)
C_SOURCES += $(wildcard Middlewares/ST/STM32_Network_Library/core/*.c)
C_SOURCES += $(wildcard Middlewares/ST/STM32_Network_Library/netif/wifi_if/es_wifi/*.c)

C_OBJECTS = $(C_SOURCES:.c=.o)

AS_SOURCES = $(wildcard Startup/*.S)
AS_OBJECTS = $(AS_SOURCES:.S=.o)

TARGET = WiFiClient.elf

DEFS = -DST -DUSE_HAL_DRIVER -DUSE_BPP=24 -DUSE_FLOATING_POINT -DUSE_SDCARD -DSTM32H7B3xxQ -DUSE_STM32H7B3I_EVAL

ASFLAGS = -x assembler-with-cpp -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard

CFLAGS = $(DEFS) \
		$(INCLUDES) \
		-O0 \
		-gdwarf-2 \
		-g \
		-std=gnu11 \
		-fshort-enums \
		-ffunction-sections \
		-fdata-sections \
		-mcpu=cortex-m7 \
		-mfpu=fpv5-d16 \
		-mfloat-abi=hard \
		-mthumb

LDFLAGS = --specs=nosys.specs \
			--specs=nano.specs \
			-mcpu=cortex-m7 \
			-mfpu=fpv5-d16 \
			-mfloat-abi=hard \
			-mthumb \
			-Wl,--gc-sections \
			-static \
			-L$(MCU_SANITIZER_WORKDIR)/mcu_asan_rt \
			-Wl,--start-group \
			-lc \
			-lm \
			-lmcuasan-rt \
			-Wl,--end-group \
			-T./STM32H7B3I-EVAL.ld


.PHONY: all clean

all: $(TARGET)

$(TARGET): $(C_OBJECTS) $(AS_OBJECTS)
	$(LD) $(C_OBJECTS) $(AS_OBJECTS) $(LDFLAGS) -o $@

$(C_OBJECTS): %.o:%.c
	$(CC) $(ARCH) $(PASS) $(CFLAGS) -c $^ -o $@

$(AS_OBJECTS): %.o:%.S
	$(AS) $(ASFLAGS) -c $^ -o $@

clean:
	find ./ -name *.o -exec rm -f {} \;
	rm -f $(TARGET)
