include_guard(GLOBAL)
message("middleware_mcu-boot_drv_mmcau component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/drivers/mmcau/src/mmcau_aes_functions.c
)


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/drivers/mmcau
)


