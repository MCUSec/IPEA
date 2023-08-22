cmake_minimum_required(VERSION 3.5)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

set(TARGET_NAME "ipea-rt")

project(${TARGET_NAME})

set(CMAKE_C_COMPILER_WORKS true)
set(CMAKE_CXX_COMPILER_WORKS true)

enable_language(ASM)

set(SRCS
	"${CMAKE_CURRENT_SOURCE_DIR}/libc_wrappers.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/libusan.c"
	# "${CMAKE_CURRENT_SOURCE_DIR}/RTT/SEGGER_RTT_ASM_ARMv7M.S"
	"${CMAKE_CURRENT_SOURCE_DIR}/RTT/SEGGER_RTT.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/fuzz/fuzz.c"
)

add_library(ipea-rt STATIC ${SRCS})

target_include_directories(ipea-rt PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/../include
	${CMAKE_CURRENT_SOURCE_DIR}/../include/target
)

target_compile_options(ipea-rt PRIVATE
	-DUSAN_ENABLE_PROFILING=1
	-Os
	-mcpu=cortex-m4
	-mthumb
	-mfloat-abi=hard
	-mfpu=fpv4-sp-d16
	-Wall
	-fno-common
	-ffunction-sections
	-fdata-sections
	-ffreestanding
	-fno-builtin
	-std=gnu99
)

