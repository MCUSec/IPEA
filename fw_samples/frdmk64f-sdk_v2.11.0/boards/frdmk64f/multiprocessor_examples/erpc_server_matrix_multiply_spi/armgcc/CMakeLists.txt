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


project(erpc_server_matrix_multiply_spi)

set(MCUX_SDK_PROJECT_NAME erpc_server_matrix_multiply_spi.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../../erpc_common/erpc_error_handler.h"
"${ProjDirPath}/../../erpc_common/erpc_error_handler.cpp"
"${ProjDirPath}/../../erpc_common/erpc_matrix_multiply/service/erpc_matrix_multiply.erpc"
"${ProjDirPath}/../../erpc_common/erpc_matrix_multiply/service/erpc_matrix_multiply.h"
"${ProjDirPath}/../../erpc_common/erpc_matrix_multiply/service/erpc_matrix_multiply_server.h"
"${ProjDirPath}/../../erpc_common/erpc_matrix_multiply/service/erpc_matrix_multiply_server.cpp"
"${ProjDirPath}/../main_server.c"
"${ProjDirPath}/../erpc_config.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../board.c"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../clock_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/../../erpc_common
    ${ProjDirPath}/../../erpc_common/erpc_matrix_multiply/service
    ${ProjDirPath}/..
)

set(CMAKE_MODULE_PATH
    ${ProjDirPath}/../../../../../middleware/multicore
    ${ProjDirPath}/../../../../../devices/MK64F12/drivers
    ${ProjDirPath}/../../../../../devices/MK64F12/utilities/debug_console_lite
    ${ProjDirPath}/../../../../../devices/MK64F12
    ${ProjDirPath}/../../../../../components/uart
    ${ProjDirPath}/../../../../../components/lists
    ${ProjDirPath}/../../../../../CMSIS/Core/Include
    ${ProjDirPath}/../../../../../devices/MK64F12/utilities
)

# include modules
include(middleware_multicore_erpc_eRPC_dspi_slave_transport_MK64F12)

include(middleware_multicore_erpc_eRPC_dspi_slave_c_wrapper_MK64F12)

include(driver_dspi_MK64F12)

include(utility_debug_console_lite_MK64F12)

include(utility_assert_lite_MK64F12)

include(driver_gpio_MK64F12)

include(driver_port_MK64F12)

include(middleware_multicore_erpc_eRPC_server_MK64F12)

include(middleware_multicore_erpc_eRPC_port_stdlib_MK64F12)

include(middleware_multicore_erpc_doc_MK64F12)

include(driver_flash_MK64F12)

include(driver_clock_MK64F12)

include(driver_uart_MK64F12)

include(driver_smc_MK64F12)

include(device_MK64F12_CMSIS_MK64F12)

include(component_uart_adapter_MK64F12)

include(driver_common_MK64F12)

include(component_lists_MK64F12)

include(device_MK64F12_startup_MK64F12)

include(middleware_multicore_erpc_common_MK64F12)

include(CMSIS_Include_core_cm_MK64F12)

include(utilities_misc_utilities_MK64F12)

include(device_MK64F12_system_MK64F12)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE g)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE stdc++)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


