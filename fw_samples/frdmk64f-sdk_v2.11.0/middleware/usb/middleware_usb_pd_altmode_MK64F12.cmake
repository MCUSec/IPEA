include_guard(GLOBAL)
message("middleware_usb_pd_altmode component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/pd/alt_mode/usb_pd_alt_mode.c
    ${CMAKE_CURRENT_LIST_DIR}/pd/alt_mode/usb_pd_alt_mode_dp.c
)


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/pd/alt_mode
)


include(middleware_usb_pd_MK64F12)

