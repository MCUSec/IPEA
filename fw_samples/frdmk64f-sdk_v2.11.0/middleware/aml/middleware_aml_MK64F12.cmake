include_guard(GLOBAL)
message("middleware_aml component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/spi_aml/spi_aml.c
    ${CMAKE_CURRENT_LIST_DIR}/tmr_aml/ftm_aml.c
    ${CMAKE_CURRENT_LIST_DIR}/tmr_aml/tpm_aml.c
    ${CMAKE_CURRENT_LIST_DIR}/wait_aml/wait_aml.c
)


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/.
    ${CMAKE_CURRENT_LIST_DIR}/spi_aml
    ${CMAKE_CURRENT_LIST_DIR}/tmr_aml
    ${CMAKE_CURRENT_LIST_DIR}/wait_aml
)


#OR Logic component
if(CONFIG_USE_driver_dspi_MK64F12)
     include(driver_dspi_MK64F12)
endif()
if(CONFIG_USE_driver_ftm_MK64F12)
     include(driver_ftm_MK64F12)
endif()
if(NOT (CONFIG_USE_driver_dspi_MK64F12 OR CONFIG_USE_driver_ftm_MK64F12))
    message(WARNING "Since driver_dspi_MK64F12/driver_ftm_MK64F12 is not included at first or config in config.cmake file, use driver_spi_MK64F12/driver_tpm_MK64F12 by default.")
    include(driver_spi_MK64F12)
    include(driver_tpm_MK64F12)
endif()

include(driver_common_MK64F12)

include(utility_assert_MK64F12)

include(driver_port_MK64F12)

include(driver_gpio_MK64F12)

include(driver_clock_MK64F12)

