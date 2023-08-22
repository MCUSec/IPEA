include_guard(GLOBAL)
message("driver_cmsis_i2c component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/fsl_i2c_cmsis.c
)


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/.
)


include(driver_i2c_edma_MK64F12)

include(driver_i2c_MK64F12)

include(CMSIS_Driver_Include_I2C_MK64F12)

