IF(NOT MCU_SANITIZER_WORKDIR)
    MESSAGE(FATA_ERROR "***Please set MCU_SANITIZER_WORKDIR in environment variables***")
ENDIF()

include_directories("$ENV{MCU_SANITIZER_WORKDIR}/include/target")
include_directories("$ENV{MCU_SANITIZER_WORKDIR}/include")

link_directories("$ENV{MCU_SANITIZER_WORKDIR}/compiler-rt")
add_definitions(-DENABLE_FUZZ=1)
add_definitions(-DSDK_DEBUGCONSOLE=2)

SET(LLVM_FLAGS " \
    --target=arm-none-eabi \
    --sysroot=${TOOLCHAIN_DIR}/arm-none-eabi \
    -march=armv7e-m \
    -flegacy-pass-manager -Xclang -load -Xclang ${LLVM_PLUGIN} \
    -fno-discard-value-names \
    -fshort-enums \
    -mllvm -polly \
")


SET(CMAKE_ASM_FLAGS_DEBUG " \
    ${CMAKE_ASM_FLAGS_DEBUG} \
    -DDEBUG \
    -D__STARTUP_CLEAR_BSS \
    -g \
    -mcpu=cortex-m4 \
    -Wall \
    -mfloat-abi=hard \
    -mfpu=fpv4-sp-d16 \
    -mthumb \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -ffreestanding \
    -fno-builtin \
    -mapcs \
    -std=gnu99 \
")
SET(CMAKE_ASM_FLAGS_RELEASE " \
    ${CMAKE_ASM_FLAGS_RELEASE} \
    -DNDEBUG \
    -D__STARTUP_CLEAR_BSS \
    -mcpu=cortex-m4 \
    -Wall \
    -mfloat-abi=hard \
    -mfpu=fpv4-sp-d16 \
    -mthumb \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -ffreestanding \
    -fno-builtin \
    -mapcs \
    -std=gnu99 \
")
SET(CMAKE_C_FLAGS_DEBUG " \
    ${CMAKE_C_FLAGS_DEBUG} \
    ${LLVM_FLAGS} \
    -DDEBUG \
    -DCPU_MK64FN1M0VLL12 \
    -DFRDM_K64F \
    -DFREEDOM \
    -DSD_ENABLED \
    -DMCUXPRESSO_SDK \
    -g \
    -O0 \
    -mcpu=cortex-m4 \
    -Wall \
    -mfloat-abi=hard \
    -mfpu=fpv4-sp-d16 \
    -mthumb \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -ffreestanding \
    -fno-builtin \
    -std=gnu99 \
")
SET(CMAKE_C_FLAGS_RELEASE " \
    ${CMAKE_C_FLAGS_RELEASE} \
    ${LLVM_FLAGS} \
    -DNDEBUG \
    -DCPU_MK64FN1M0VLL12 \
    -DFRDM_K64F \
    -DFREEDOM \
    -DSD_ENABLED \
    -DMCUXPRESSO_SDK \
    -Os \
    -mcpu=cortex-m4 \
    -Wall \
    -mfloat-abi=hard \
    -mfpu=fpv4-sp-d16 \
    -mthumb \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -ffreestanding \
    -fno-builtin \
    -std=gnu99 \
")
SET(CMAKE_CXX_FLAGS_DEBUG " \
    ${CMAKE_CXX_FLAGS_DEBUG} \
    ${LLVM_FLAGS} \
    -DDEBUG \
    -DCPU_MK64FN1M0VLL12 \
    -DMCUXPRESSO_SDK \
    -g \
    -O0 \
    -mcpu=cortex-m4 \
    -Wall \
    -mfloat-abi=hard \
    -mfpu=fpv4-sp-d16 \
    -mthumb \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -ffreestanding \
    -fno-builtin \
    -fno-rtti \
    -fno-exceptions \
")
SET(CMAKE_CXX_FLAGS_RELEASE " \
    ${CMAKE_CXX_FLAGS_RELEASE} \
    ${LLVM_FLAGS} \
    -DNDEBUG \
    -DCPU_MK64FN1M0VLL12 \
    -DMCUXPRESSO_SDK \
    -Os \
    -mcpu=cortex-m4 \
    -Wall \
    -mfloat-abi=hard \
    -mfpu=fpv4-sp-d16 \
    -mthumb \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -ffreestanding \
    -fno-builtin \
    -fno-rtti \
    -fno-exceptions \
")
SET(CMAKE_EXE_LINKER_FLAGS_DEBUG " \
    ${CMAKE_EXE_LINKER_FLAGS_DEBUG} \
    -g \
    -mcpu=cortex-m4 \
    -Wall \
    -mfloat-abi=hard \
    -mfpu=fpv4-sp-d16 \
    --specs=nano.specs \
    --specs=nosys.specs \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -ffreestanding \
    -fno-builtin \
    -mthumb \
    -mapcs \
    -Xlinker \
    --gc-sections \
    -Xlinker \
    -static \
    -Xlinker \
    -z \
    -Xlinker \
    muldefs \
    -Xlinker \
    -Map=output.map \
    -Wl,--print-memory-usage \
    -Xlinker \
    --defsym=__stack_size__=0x800 \
    -T${ProjDirPath}/MK64FN1M0xxx12_flash.ld -static \
")
SET(CMAKE_EXE_LINKER_FLAGS_RELEASE " \
    ${CMAKE_EXE_LINKER_FLAGS_RELEASE} \
    -mcpu=cortex-m4 \
    -Wall \
    -mfloat-abi=hard \
    -mfpu=fpv4-sp-d16 \
    --specs=nano.specs \
    --specs=nosys.specs \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -ffreestanding \
    -fno-builtin \
    -mthumb \
    -mapcs \
    -Xlinker \
    --gc-sections \
    -Xlinker \
    -static \
    -Xlinker \
    -z \
    -Xlinker \
    muldefs \
    -Xlinker \
    -Map=output.map \
    -Wl,--print-memory-usage \
    -Xlinker \
    --defsym=__stack_size__=0x800 \
    -T${ProjDirPath}/MK64FN1M0xxx12_flash.ld -static \
")
