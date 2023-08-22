include_guard(GLOBAL)
message("middleware_usb_pd_phy_ptn5110 component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/pd/ptn5110/usb_pd_ptn5110_connect.c
    ${CMAKE_CURRENT_LIST_DIR}/pd/ptn5110/usb_pd_ptn5110_hal.c
    ${CMAKE_CURRENT_LIST_DIR}/pd/ptn5110/usb_pd_ptn5110_interface.c
    ${CMAKE_CURRENT_LIST_DIR}/pd/ptn5110/usb_pd_ptn5110_msg.c
    ${CMAKE_CURRENT_LIST_DIR}/pd/usb_pd_timer.c
    ${CMAKE_CURRENT_LIST_DIR}/pd/phy_interface/usb_pd_i2c.c
)


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/pd/ptn5110
    ${CMAKE_CURRENT_LIST_DIR}/pd
    ${CMAKE_CURRENT_LIST_DIR}/pd/phy_interface
)


include(component_gpio_adapter_MK64F12)

include(component_i2c_adapter_MK64F12)

include(component_osa_MK64F12)

include(middleware_usb_common_header_MK64F12)

