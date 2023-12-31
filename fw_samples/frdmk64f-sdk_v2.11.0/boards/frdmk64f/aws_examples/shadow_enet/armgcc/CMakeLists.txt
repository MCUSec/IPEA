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


project(aws_shadow_enet)

set(MCUX_SDK_PROJECT_NAME aws_shadow_enet.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../config_files/FreeRTOSConfig.h"
"${ProjDirPath}/../config_files/aws_mbedtls_config.h"
"${ProjDirPath}/../config_files/aws_demo_config.h"
"${ProjDirPath}/../config_files/aws_shadow_config.h"
"${ProjDirPath}/../config_files/core_mqtt_config.h"
"${ProjDirPath}/../config_files/core_pkcs11_config.h"
"${ProjDirPath}/../config_files/FreeRTOSIPConfig.h"
"${ProjDirPath}/../config_files/iot_config.h"
"${ProjDirPath}/../config_files/iot_secure_sockets_config.h"
"${ProjDirPath}/../config_files/shadow_config.h"
"${ProjDirPath}/../config_files/shadow_demo_config.h"
"${ProjDirPath}/../config_files/aws_iot_network_config.h"
"${ProjDirPath}/../main_enet.c"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../lwipopts.h"
"${ProjDirPath}/../board.c"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../clock_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/../config_files
    ${ProjDirPath}/../../..
    ${ProjDirPath}/..
)

if(CMAKE_BUILD_TYPE STREQUAL debug)
     target_compile_definitions(${MCUX_SDK_PROJECT_NAME}  PRIVATE MBEDTLS_CONFIG_FILE="aws_mbedtls_config.h")
endif(CMAKE_BUILD_TYPE STREQUAL debug)

if(CMAKE_BUILD_TYPE STREQUAL release)
     target_compile_definitions(${MCUX_SDK_PROJECT_NAME}  PRIVATE MBEDTLS_CONFIG_FILE="aws_mbedtls_config.h")
endif(CMAKE_BUILD_TYPE STREQUAL release)

set(CMAKE_MODULE_PATH
    ${ProjDirPath}/../../../../../components/phy/mdio/enet
    ${ProjDirPath}/../../../../../components/phy/device/phyksz8081
    ${ProjDirPath}/../../../../../devices/MK64F12/drivers
    ${ProjDirPath}/../../../../../middleware/mmcau
    ${ProjDirPath}/../../../../../middleware/mbedtls
    ${ProjDirPath}/../../../../../components/flash/mflash
    ${ProjDirPath}/../../../../../components/flash/mflash/frdmk64f
    ${ProjDirPath}/../../../../../middleware/lwip
    ${ProjDirPath}/../../../../../components/phy
    ${ProjDirPath}/../../../../../devices/MK64F12/utilities
    ${ProjDirPath}/../../../../../rtos/freertos
    ${ProjDirPath}/../../../../../rtos/freertos/freertos_kernel
    ${ProjDirPath}/../../../../../devices/MK64F12
    ${ProjDirPath}/../../../../../components/uart
    ${ProjDirPath}/../../../../../components/serial_manager
    ${ProjDirPath}/../../../../../components/lists
    ${ProjDirPath}/../../../../../CMSIS/Core/Include
)

# include modules
include(driver_mdio-enet_MK64F12)

include(driver_phy-device-ksz8081_MK64F12)

include(driver_rnga_MK64F12)

include(middleware_mmcau_cm4_cm7_MK64F12)

include(middleware_mbedtls_MK64F12)

include(middleware_mbedtls_port_ksdk_MK64F12)

include(middleware_mbedtls_kinetis_MK64F12)

include(component_mflash_file_MK64F12)

include(component_mflash_frdmk64f_MK64F12)

include(middleware_lwip_enet_ethernetif_MK64F12)

include(middleware_lwip_MK64F12)

include(driver_phy-common_MK64F12)

include(driver_enet_MK64F12)

include(utility_debug_console_MK64F12)

include(driver_port_MK64F12)

include(middleware_freertos-aws_iot_demos_MK64F12)

include(middleware_freertos-aws_iot_demos_dev_mode_key_provisioning_MK64F12)

include(middleware_freertos-aws_iot_demos_shadow_MK64F12)

include(middleware_freertos-aws_iot_libraries_abstractions_transport_MK64F12)

include(middleware_freertos-kernel_MK64F12)

include(middleware_freertos-aws_iot_libraries_abstractions_secure_sockets_MK64F12)

include(middleware_freertos-aws_iot_libraries_logging_MK64F12)

include(middleware_freertos-aws_iot_libraries_coremqtt_MK64F12)

include(middleware_freertos-aws_iot_vendor_nxp_secure_sockets_lwip_MK64F12)

include(middleware_freertos-aws_iot_libraries_3rdparty_mbedtls_config_MK64F12)

include(middleware_freertos-aws_iot_libraries_3rdparty_pkcs11_MK64F12)

include(middleware_freertos-aws_iot_libraries_abstractions_pkcs11_MK64F12)

include(middleware_freertos-aws_iot_libraries_abstractions_pkcs11_mbedtls_MK64F12)

include(middleware_freertos-aws_iot_vendor_nxp_pkcs11_MK64F12)

include(middleware_freertos-kernel_heap_4_MK64F12)

include(driver_flash_MK64F12)

include(driver_gpio_MK64F12)

include(driver_clock_MK64F12)

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

include(driver_mdio-common_MK64F12)

include(CMSIS_Include_core_cm_MK64F12)

include(middleware_mmcau_common_files_MK64F12)

include(component_mflash_common_MK64F12)

include(middleware_freertos-aws_iot_libraries_abstractions_platform_MK64F12)

include(middleware_freertos-aws_iot_libraries_c_sdk_standard_common_MK64F12)

include(middleware_freertos-kernel_extension_MK64F12)

include(middleware_freertos-aws_iot_network_manager_MK64F12)

include(middleware_freertos-aws_iot_libraries_corejason_MK64F12)

include(middleware_freertos-aws_iot_libraries_device_shadow_for_aws_MK64F12)

include(middleware_freertos-aws_iot_mqtt_demo_helpers_MK64F12)

include(middleware_freertos-aws_iot_libraries_abstractions_backoff_algorithm_MK64F12)

include(middleware_freertos-aws_iot_pkcs11_helpers_MK64F12)

include(middleware_freertos-aws_iot_libraries_freertos_plus_standard_tls_MK64F12)

include(middleware_freertos-aws_iot_libraries_3rdparty_mbedtls_utils_MK64F12)

include(middleware_freertos-aws_iot_libraries_freertos_plus_standard_crypto_MK64F12)

include(middleware_freertos-aws_iot_libraries_freertos_plus_standard_utils_MK64F12)

include(utilities_misc_utilities_MK64F12)

include(device_MK64F12_system_MK64F12)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

if(CMAKE_BUILD_TYPE STREQUAL debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../../../../middleware/mmcau/asm-cm4-cm7/lib_mmcau.a)
endif(CMAKE_BUILD_TYPE STREQUAL debug)

if(CMAKE_BUILD_TYPE STREQUAL release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../../../../middleware/mmcau/asm-cm4-cm7/lib_mmcau.a)
endif(CMAKE_BUILD_TYPE STREQUAL release)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


