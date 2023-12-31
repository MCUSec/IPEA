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


project(freertos_segger_sysview)

set(MCUX_SDK_PROJECT_NAME freertos_segger_sysview.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../main.c"
"${ProjDirPath}/../FreeRTOSConfig.h"
"${ProjDirPath}/../SEGGER/SEGGER.h"
"${ProjDirPath}/../SEGGER/SEGGER_RTT.c"
"${ProjDirPath}/../SEGGER/SEGGER_RTT.h"
"${ProjDirPath}/../SEGGER/SEGGER_SYSVIEW.c"
"${ProjDirPath}/../SEGGER/SEGGER_SYSVIEW.h"
"${ProjDirPath}/../SEGGER/SEGGER_RTT_ASM_ARMv7M.S"
"${ProjDirPath}/../SEGGER/SEGGER_SYSVIEW_ConfDefaults.h"
"${ProjDirPath}/../SEGGER/SEGGER_SYSVIEW_Int.h"
"${ProjDirPath}/../Config/Global.h"
"${ProjDirPath}/../Config/SEGGER_RTT_Conf.h"
"${ProjDirPath}/../Config/SEGGER_SYSVIEW_Conf.h"
"${ProjDirPath}/../Sample/FreeRTOSV10/SEGGER_SYSVIEW_FreeRTOS.c"
"${ProjDirPath}/../Sample/FreeRTOSV10/SEGGER_SYSVIEW_FreeRTOS.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../board.c"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../clock_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${ProjDirPath}/../SEGGER
    ${ProjDirPath}/../Config
    ${ProjDirPath}/../Sample/FreeRTOSV10
)

set(CMAKE_MODULE_PATH
    ${ProjDirPath}/../../../../../../devices/MK64F12/drivers
    ${ProjDirPath}/../../../../../../rtos/freertos/freertos_kernel
    ${ProjDirPath}/../../../../../../devices/MK64F12/utilities
    ${ProjDirPath}/../../../../../../devices/MK64F12
    ${ProjDirPath}/../../../../../../components/uart
    ${ProjDirPath}/../../../../../../components/serial_manager
    ${ProjDirPath}/../../../../../../components/lists
    ${ProjDirPath}/../../../../../../CMSIS/Core/Include
)

# include modules
include(driver_port_MK64F12)

include(driver_gpio_MK64F12)

include(driver_clock_MK64F12)

include(middleware_freertos-kernel_heap_4_MK64F12)

include(driver_flash_MK64F12)

include(utility_debug_console_MK64F12)

include(driver_uart_MK64F12)

include(driver_smc_MK64F12)

include(device_MK64F12_CMSIS_MK64F12)

include(component_uart_adapter_MK64F12)

include(component_serial_manager_MK64F12)

include(driver_common_MK64F12)

include(component_lists_MK64F12)

include(component_serial_manager_uart_MK64F12)

include(device_MK64F12_startup_MK64F12)

include(utility_assert_MK64F12)

include(CMSIS_Include_core_cm_MK64F12)

include(middleware_freertos-kernel_MK64F12)

include(middleware_freertos-kernel_extension_MK64F12)

include(utilities_misc_utilities_MK64F12)

include(device_MK64F12_system_MK64F12)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


