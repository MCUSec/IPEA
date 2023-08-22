include_guard(GLOBAL)
message("middleware_mmcau_cm4_cm7 component is included.")


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/.
)

include(middleware_mmcau_common_files_MK64F12)

include(driver_clock_MK64F12)

include(driver_common_MK64F12)

