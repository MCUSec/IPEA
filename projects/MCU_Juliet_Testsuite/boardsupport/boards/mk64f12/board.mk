TARGET_TRIPLE ?= arm-none-eabi-gcc
CC ?= $(TARGET_TRIPLE)-gcc
AS ?= $(TARGET_TRIPLE)-gcc

CFLAGS += -DCPU_MK64FN1M0VLL12 \
		  -DFRDM_K64F \
		  -DFREEDOM \
		  -DFSL_OSA_BM_TASK_ENABLE=0 \
		  -DFSL_OSA_BM_TIMER_CONFIG=0 \
		  -DSDK_DEBUGCONSOLE=2 \
		  -DMCUXPRESSO_SDK \
		  -D_DEBUG=1 \
		  -DDEBUG \
		  -march=armv7e-m \
		  -mcpu=cortex-m4 \
		  -mfloat-abi=hard \
		  -mfpu=fpv4-sp-d16 \
		  -mthumb

ASFLAGS += -x assembler-with-cpp -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -D__STARTUP_CLEAR_BSS
LFLAGS += -mcpu=cortex-m4 \
		  -mfloat-abi=hard \
		  -mfpu=fpv4-sp-d16 \
		  --specs=nano.specs \
		  --specs=nosys.specs \
		  -Xlinker --defsym=__stack_size__=0x2000 \
		  -Xlinker --defsym=__heap_size__=0x2000 \
		  -static \
		  -T $(BOARD_SUPPORT_PATH)boards/mk64f12/MK64FN1M0xxx12_flash.ld

TARGET_SUPPORT_PATH = $(BOARD_SUPPORT_PATH)boards/mk64f12/

BOARD_C_SUPPORT_FILES = $(TARGET_SUPPORT_PATH)system_MK64F12.c
BOARD_ASM_SUPPORT_FILES = $(TARGET_SUPPORT_PATH)startup_MK64F12.S

BOARD_C_SUPPORT_OBJECTS = $(BOARD_C_SUPPORT_FILES:.c=.o)
BOARD_ASM_SUPPORT_OBJECTS = $(BOARD_ASM_SUPPORT_FILES:.S=.o)

INCLUDES += -I$(BOARD_SUPPORT_PATH)CMSIS/Core/Include

ASAN_SHADOW_OFFSET = 0x1c02a000

$(BOARD_C_SUPPORT_OBJECTS) : $(BOARD_C_SUPPORT_FILES)
	$(CC) $(CFLAGS) $(INCLUDES) -c $(TARGET_SUPPORT_PATH)$(@:.o=.c) -o $@

$(BOARD_ASM_SUPPORT_OBJECTS) : $(BOARD_ASM_SUPPORT_FILES)
	$(AS) $(ASFLAGS) -c $(TARGET_SUPPORT_PATH)$(@:.o=.S) -o $@
