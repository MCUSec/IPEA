include_guard(GLOBAL)
message("driver_cmsis_dspi component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/fsl_dspi_cmsis.c
)


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/.
)


include(driver_dspi_edma_MK64F12)

include(CMSIS_Driver_Include_SPI_MK64F12)

