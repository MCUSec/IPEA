TARGET_TRIPLE = arm-none-eabi

CC = $(TARGET_TRIPLE)-gcc
LD = $(TARGET_TRIPLE)-gcc
AS = $(TARGET_TRIPLE)-gcc

C_BOARD_SOURCES = $(wildcard $(BOARD_DIR)*.c)
C_BOARD_OBJECTS = $(C_BOARD_SOURCES:.c=.o)

AS_BOARD_SOURCES = $(wildcard $(BOARD_DIR)*.S)
AS_BOARD_OBJECTS = $(AS_BOARD_SOURCES:.S=.o)

INCLUDES += -I$(BOARD_DIR)
INCLUDES += -I$(BOARD_DIR)../../support
INCLUDES += -I$(BOARD_DIR)../CMSIS/Core/Include
INCLUDES += -I$(IPEA_HOME)/projects/MCU_ASAN

SAN = -fsanitize=kernel-address --param asan-globals=1 --param asan-stack=1 -fasan-shadow-offset=0x1c02a000

CFLAGS += -DCPU_MK64FN1M0VLL12 \
		  -DFRDM_K64F \
		  -DFREEDOM \
		  -DFSL_OSA_BM_TASK_ENABLE=0 \
		  -DFSL_OSA_BM_TIMER_CONFIG=0 \
		  -DSDK_DEBUGCONSOLE=2 \
		  -DMCUXPRESSO_SDK \
		  -DENABLE_PROFILE=1 \
		  -DENABLE_ASAN=1 \
		  -g \
		  -march=armv7e-m \
		  -mcpu=cortex-m4 \
		  -mfloat-abi=hard \
		  -mfpu=fpv4-sp-d16 \
		  -mthumb \
		  -finstrument-functions \
		  -fshort-enums \
		  -fno-common \
		  -ffunction-sections \
		  -fdata-sections \
		  -ffreestanding \
		  -fno-builtin

WRAPPERS += -Wl,--wrap=malloc_beebs
WRAPPERS += -Wl,--wrap=free_beebs
WRAPPERS += -Wl,--wrap=malloc
WRAPPERS += -Wl,--wrap=free
WRAPPERS += -Wl,--wrap=memset
WRAPPERS += -Wl,--wrap=memmove
WRAPPERS += -Wl,--wrap=memcpy
WRAPPERS += -Wl,--wrap=strcpy
WRAPPERS += -Wl,--wrap=strncpy
WRAPPERS += -Wl,--wrap=strcat
WRAPPERS += -Wl,--wrap=strncat
WRAPPERS += -Wl,--wrap=sprintf
WRAPPERS += -Wl,--wrap=snprintf

ASFLAGS += -x assembler-with-cpp -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -D__STARTUP_CLEAR_BSS
LDFLAGS += -L$(IPEA_HOME)/projects/MCU_ASAN -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 $(WRAPPERS) -Wl,--start-group -lc -lm -lmcuasan -specs=nano.specs -specs=nosys.specs -Wl,--end-group -static -T$(BOARD_DIR)MK64FN1M0xxx12_flash.ld

$(C_BOARD_OBJECTS): %.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(AS_BOARD_OBJECTS): %.o: %.S
	$(AS) $(ASFLAGS) -c $< -o $@
