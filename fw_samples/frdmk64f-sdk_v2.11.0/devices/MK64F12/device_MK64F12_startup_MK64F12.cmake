include_guard(GLOBAL)
message("device_MK64F12_startup component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/gcc/startup_MK64F12.S
)


include(device_MK64F12_system_MK64F12)

