# CROSS COMPILER SETTING
SET(CMAKE_SYSTEM_NAME Generic)
CMAKE_MINIMUM_REQUIRED (VERSION 3.10.0)

# THE VERSION NUMBER
SET (Tutorial_VERSION_MAJOR 1)
SET (Tutorial_VERSION_MINOR 0)

# ENABLE ASM
ENABLE_LANGUAGE(ASM)

SET(CMAKE_STATIC_LIBRARY_PREFIX)
SET(CMAKE_STATIC_LIBRARY_SUFFIX)

SET(CMAKE_EXECUTABLE_LIBRARY_PREFIX)
SET(CMAKE_EXECUTABLE_LIBRARY_SUFFIX)

# CURRENT DIRECTORY
SET(ProjDirPath ${CMAKE_CURRENT_SOURCE_DIR})

SET(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})
SET(LIBRARY_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})


project(flashloader_loader)

set(MCUX_SDK_PROJECT_NAME flashloader_loader.elf)

include(${ProjDirPath}/flags.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../../../../../middleware/mcu-boot/targets/MK64F12/src/bootloader_config.h"
"${ProjDirPath}/../../../../../middleware/mcu-boot/targets/MK64F12/src/target_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/../../flashloader/armgcc
    ${ProjDirPath}/../../../../../middleware/mcu-boot/targets/MK64F12/src
)

set(CMAKE_MODULE_PATH
    ${ProjDirPath}/../../../../../devices/MK64F12/drivers
    ${ProjDirPath}/../../../../../devices/MK64F12
    ${ProjDirPath}/../../../../../middleware/mcu-boot
    ${ProjDirPath}/../../../../../CMSIS/Core/Include
)

# include modules
include(driver_clock_MK64F12)

include(device_MK64F12_CMSIS_MK64F12)

include(middleware_mcu-boot_MK64F12_startup_MK64F12)

include(middleware_mcu-boot_loadersources_MK64F12)

include(driver_uart_MK64F12)

include(middleware_mcu-boot_src_startup_MK64F12)

include(driver_common_MK64F12)

include(CMSIS_Include_core_cm_MK64F12)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


