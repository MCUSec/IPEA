include_guard(GLOBAL)
message("middleware_usb_host_cdc_rndis component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/host/class/usb_host_cdc_rndis.c
)


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/host/class
)


include(middleware_usb_host_stack_MK64F12)

include(middleware_usb_host_cdc_MK64F12)

