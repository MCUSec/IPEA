CC = gcc
CXX = g++
AR = ar

TARGET_LIB = libmcuasan.a

INCDIRS = -I../ -I../JLinkSDK_Linux_V758e/Samples_Inc -I../mcu_asan_rt/fuzz

C_SOURCE = common.c init.c
CXX_SOURCE = bridge.cpp context.cpp RTT.cpp 

C_OBJECTS = $(C_SOURCE:.c=.o)
CXX_OBJECTS = $(CXX_SOURCE:.cpp=.o)

.PHONY: clean


$(TARGET_LIB): $(C_OBJECTS) $(CXX_OBJECTS)
	$(AR) -r $@ $(C_OBJECTS) $(CXX_OBJECTS)

$(C_OBJECTS): %.o: %.c
	$(CC) $(INCDIRS) -c $< -o $@

$(CXX_OBJECTS): %.o: %.cpp
	$(CXX) $(INCDIRS) -c $< -o $@

clean:
	rm -f *.o *.a