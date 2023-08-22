include_guard(GLOBAL)
message("middleware_issdk_drivers_gpio_kinetis component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/drivers/gpio/kinetis/gpio_driver_irq.c
    ${CMAKE_CURRENT_LIST_DIR}/drivers/gpio/kinetis/gpio_driver.c
)


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/drivers/gpio/kinetis
    ${CMAKE_CURRENT_LIST_DIR}/drivers/gpio
)


