include_guard(GLOBAL)
message("middleware_mcu-boot_MK64F12_sources component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/targets/MK64F12/src/bl_clock_config_MK64F12.c
    ${CMAKE_CURRENT_LIST_DIR}/targets/MK64F12/src/hardware_init_MK64F12.c
    ${CMAKE_CURRENT_LIST_DIR}/targets/MK64F12/src/memory_map_MK64F12.c
    ${CMAKE_CURRENT_LIST_DIR}/targets/MK64F12/src/bl_peripherals_MK64F12.c
    ${CMAKE_CURRENT_LIST_DIR}/targets/common/src/pinmux_utility_common.c
    ${CMAKE_CURRENT_LIST_DIR}/targets/common/src/bl_dspi_irq_config_common.c
    ${CMAKE_CURRENT_LIST_DIR}/targets/common/src/bl_i2c_irq_config_common.c
    ${CMAKE_CURRENT_LIST_DIR}/targets/common/src/bl_uart_irq_config_common.c
    ${CMAKE_CURRENT_LIST_DIR}/src/bootloader/src/scuart_peripheral_interface.c
    ${CMAKE_CURRENT_LIST_DIR}/src/bootloader/src/dspi_peripheral_interface.c
    ${CMAKE_CURRENT_LIST_DIR}/src/bootloader/src/i2c_peripheral_interface.c
    ${CMAKE_CURRENT_LIST_DIR}/src/bootloader/src/usb_hid_msc_peripheral_interface.c
    ${CMAKE_CURRENT_LIST_DIR}/src/memory/src/sram_init_cm4.c
)


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/targets/MK64F12/src
    ${CMAKE_CURRENT_LIST_DIR}/src/drivers/ltc
)


