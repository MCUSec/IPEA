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


project(flashloader)

set(MCUX_SDK_PROJECT_NAME flashloader.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../clock_config.h"
"${ProjDirPath}/../usb_device_hid.c"
"${ProjDirPath}/../usb_device_hid.h"
"${ProjDirPath}/../usb_device_msc.c"
"${ProjDirPath}/../usb_device_msc.h"
"${ProjDirPath}/../usb_device_msc_ufi.c"
"${ProjDirPath}/../usb_device_msc_ufi.h"
"${ProjDirPath}/../usb_device_ch9.c"
"${ProjDirPath}/../usb_device_ch9.h"
"${ProjDirPath}/../usb_device_class.c"
"${ProjDirPath}/../usb_device_class.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
)

set(CMAKE_MODULE_PATH
    ${ProjDirPath}/../../../../../middleware/mcu-boot
    ${ProjDirPath}/../iar
    ${ProjDirPath}/../mdk
    ${ProjDirPath}/
    ${ProjDirPath}/../../../../../devices/MK64F12/drivers
    ${ProjDirPath}/../../../../../devices/MK64F12
    ${ProjDirPath}/../../../../../middleware
    ${ProjDirPath}/../../../../../middleware/usb
    ${ProjDirPath}/../../../../../components/osa
    ${ProjDirPath}/../../../../../CMSIS/Core/Include
    ${ProjDirPath}/../../../../../components/lists
)

# include modules
include(middleware_mcu-boot_postaction_MK64F12)

include(middleware_mcu-boot_build_scripts_iar_MK64F12)

include(middleware_mcu-boot_build_scripts_mdk_MK64F12)

include(middleware_mcu-boot_build_scripts_armgcc_MK64F12)

include(driver_clock_MK64F12)

include(driver_flash_MK64F12)

include(device_MK64F12_CMSIS_MK64F12)

include(driver_uart_MK64F12)

include(driver_gpio_MK64F12)

include(driver_i2c_MK64F12)

include(driver_port_MK64F12)

include(driver_dspi_MK64F12)

include(driver_crc_MK64F12)

include(middleware_baremetal_MK64F12)

include(middleware_usb_device_controller_driver_MK64F12)

include(component_osa_MK64F12)

include(middleware_usb_device_khci_MK64F12)

include(middleware_mcu-boot_MK64F12_startup_MK64F12)

include(middleware_mcu-boot_bootsources_MK64F12)

include(middleware_mcu-boot_bootsources_kinetis_MK64F12)

include(middleware_mcu-boot_MK64F12_sources_MK64F12)

include(middleware_mcu-boot_src_bm_usb_MK64F12)

include(middleware_mcu-boot_src_security_MK64F12)

include(middleware_mcu-boot_drv_mmcau_MK64F12)

include(middleware_mcu-boot_src_startup_MK64F12)

include(middleware_mcu-boot_drv_microseconds_MK64F12)

include(driver_common_MK64F12)

include(CMSIS_Include_core_cm_MK64F12)

include(component_lists_MK64F12)

include(middleware_usb_device_common_header_MK64F12)

include(middleware_usb_common_header_MK64F12)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/flashloader.bin)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND
python bin/create_fl_image_mcux.py ${EXECUTABLE_OUTPUT_PATH}/flashloader.elf ${EXECUTABLE_OUTPUT_PATH}/flashloader.bin ${EXECUTABLE_OUTPUT_PATH}/flashloader_image.c
)

