CC = gcc
CXX = g++

CFLAGS += -D_GNU_SOURCE -O3 -Wall -Wno-unused -Wno-pointer-sign 
CPPFLAGS += -D_GNU_SOURCE -O3 -Wall -Wno-unused 
LDFLAGS += -L ../JLinkSDK_Linux_V758e/Samples_Inc \
		-L ../MCU_Sanitizer_Src \
		-lmcuasan \
		-ljlinkarmimport \
		-lsysimport \
		-lutilimport \
		-pthread \
		-lrt \
		-lm \
		-ldl \
		-ljsoncpp \
		-lstdc++ 

INCLUDE_DIRS +=	-I. \
				-I../JLinkSDK_Linux_V758e/Samples_Inc \
				-I../MCU_Sanitizer_Src

C_SOURCE = afl-fuzz.c
C_OBJECTS = $(C_SOURCE:.c=.o)

TARGET = micro-afl-fuzz

.PHONY: default all clean

default: all

all: $(TARGET)

$(C_OBJECTS): %.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -c $< -o $@

$(TARGET): $(C_OBJECTS)
	$(CC) $(C_OBJECTS) $(LDFLAGS) -o $(TARGET)

clean:
	rm -f *.o $(TARGET)