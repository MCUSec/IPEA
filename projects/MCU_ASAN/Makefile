TARGET=libmcuasan.a

CC=arm-none-eabi-gcc
AR=arm-none-eabi-ar

CFLAGS += \
	-fvisibility=hidden \
	-mcpu=cortex-m4 \
	-mthumb \
	-mfloat-abi=hard \
	-mfpu=fpv4-sp-d16 \
	-Wall \
	-g \
	-Os \
	-fno-common \
	-ffunction-sections \
	-fdata-sections \
	-ffreestanding \
	-fno-builtin \
	-std=gnu99

INCLUDE_DIR += -I../../include -I../../include/target

C_SOURCES = $(wildcard *.c)
C_OBJECTS = $(C_SOURCES:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(C_OBJECTS)
	$(AR) -r $@ $^

$(C_OBJECTS): %.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE_DIR) -c $< -o $(notdir $@)

clean:
	rm -f *.o *.a
