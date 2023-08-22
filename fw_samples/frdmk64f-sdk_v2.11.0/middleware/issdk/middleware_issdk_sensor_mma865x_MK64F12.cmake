include_guard(GLOBAL)
message("middleware_issdk_sensor_mma865x component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/sensors/mma865x_drv.c
)


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/sensors
)


include(CMSIS_Driver_Include_I2C_MK64F12)

include(CMSIS_Driver_Include_SPI_MK64F12)

