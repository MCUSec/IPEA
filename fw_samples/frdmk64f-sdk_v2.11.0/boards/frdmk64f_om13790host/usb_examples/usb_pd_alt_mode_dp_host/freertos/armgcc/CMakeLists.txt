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


project(usb_pd_alt_mode_dp_host_freertos)

set(MCUX_SDK_PROJECT_NAME usb_pd_alt_mode_dp_host_freertos.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../pd_board_config.h"
"${ProjDirPath}/../pd_app.c"
"${ProjDirPath}/../pd_app.h"
"${ProjDirPath}/../pd_app_demo.c"
"${ProjDirPath}/../pd_command_app.c"
"${ProjDirPath}/../pd_command_interface.c"
"${ProjDirPath}/../pd_command_interface.h"
"${ProjDirPath}/../pd_power_app.c"
"${ProjDirPath}/../pd_power_interface.c"
"${ProjDirPath}/../pd_power_interface.h"
"${ProjDirPath}/../usb_pd_config.h"
"${ProjDirPath}/../pd_app_misc.h"
"${ProjDirPath}/../pd_dp_board_chip.c"
"${ProjDirPath}/../pd_dp_board_chip.h"
"${ProjDirPath}/../pd_crossbar_cbtl.h"
"${ProjDirPath}/../pd_crossbar_cbtl.c"
"${ProjDirPath}/../pd_ptn36502.h"
"${ProjDirPath}/../pd_ptn36502.c"
"${ProjDirPath}/../pd_dp_hpd_driver.h"
"${ProjDirPath}/../pd_dp_hpd_driver.c"
"${ProjDirPath}/../pd_power_nx20p3483.h"
"${ProjDirPath}/../pd_power_nx20p3483.c"
"${ProjDirPath}/../FreeRTOSConfig.h"
"${ProjDirPath}/../board.c"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../clock_config.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${ProjDirPath}/../../../../../frdmk64f
)

set(CMAKE_MODULE_PATH
    ${ProjDirPath}/../../../../../../devices/MK64F12/drivers
    ${ProjDirPath}/../../../../../../components/i2c
    ${ProjDirPath}/../../../../../../middleware/usb
    ${ProjDirPath}/../../../../../../rtos/freertos/freertos_kernel
    ${ProjDirPath}/../../../../../../components/osa
    ${ProjDirPath}/../../../../../../devices/MK64F12
    ${ProjDirPath}/../../../../../../devices/MK64F12/utilities
    ${ProjDirPath}/../../../../../../components/uart
    ${ProjDirPath}/../../../../../../components/serial_manager
    ${ProjDirPath}/../../../../../../components/lists
    ${ProjDirPath}/../../../../../../components/gpio
    ${ProjDirPath}/../../../../../../components/timer
    ${ProjDirPath}/../../../../../../CMSIS/Core/Include
)

# include modules
include(driver_edma_MK64F12)

include(driver_dmamux_MK64F12)

include(driver_i2c_MK64F12)

include(driver_i2c_edma_MK64F12)

include(component_i2c_adapter_MK64F12)

include(driver_pit_MK64F12)

include(middleware_usb_pd_phy_ptn5110_MK64F12)

include(middleware_usb_pd_MK64F12)

include(middleware_freertos-kernel_heap_4_MK64F12)

include(middleware_usb_pd_altmode_MK64F12)

include(component_osa_free_rtos_MK64F12)

include(driver_smc_MK64F12)

include(driver_clock_MK64F12)

include(driver_common_MK64F12)

include(device_MK64F12_CMSIS_MK64F12)

include(device_MK64F12_startup_MK64F12)

include(driver_uart_MK64F12)

include(driver_gpio_MK64F12)

include(utility_assert_MK64F12)

include(utility_debug_console_MK64F12)

include(component_uart_adapter_MK64F12)

include(component_serial_manager_MK64F12)

include(component_lists_MK64F12)

include(component_serial_manager_uart_MK64F12)

include(component_gpio_adapter_MK64F12)

include(component_pit_adapter_MK64F12)

include(driver_port_MK64F12)

include(CMSIS_Include_core_cm_MK64F12)

include(component_osa_MK64F12)

include(middleware_usb_common_header_MK64F12)

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


