include_guard(GLOBAL)
message("middleware_mcu-boot_MK64F12_startup component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/startup/crt0_gcc.S
    ${CMAKE_CURRENT_LIST_DIR}/targets/MK64F12/src/startup/gcc/startup_MK64F12.S
    ${CMAKE_CURRENT_LIST_DIR}/../../devices/MK64F12/system_MK64F12.c
)


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/../../devices/MK64F12
)


