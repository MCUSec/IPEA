include_guard(GLOBAL)
message("device_MK64F12_CMSIS component is included.")


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/.
)

include(CMSIS_Include_core_cm_MK64F12)

