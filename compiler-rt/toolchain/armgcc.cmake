set(CMAKE_SYSTEM_NAME Linux)

set(TOOLCHAIN_PATH $ENV{ARMGCC_DIR})
set(TARGET_TRIPLE arm-none-eabi)

set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/bin/arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/bin/arm-none-eabi-gcc)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PATH}/bin/arm-none-eabi-gcc)
set(CMAKE_AR ${TOOLCHAIN_PATH}/bin/arm-none-eabi-ar)
