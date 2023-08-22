include_guard(GLOBAL)
message("middleware_sdmmc_host_sdhc component is included.")


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/host/sdhc
)

#OR Logic component
if(CONFIG_USE_middleware_sdmmc_host_sdhc_freertos_MK64F12)
     include(middleware_sdmmc_host_sdhc_freertos_MK64F12)
endif()
if(CONFIG_USE_middleware_sdmmc_host_sdhc_interrupt_MK64F12)
     include(middleware_sdmmc_host_sdhc_interrupt_MK64F12)
endif()
if(CONFIG_USE_middleware_sdmmc_host_sdhc_polling_MK64F12)
     include(middleware_sdmmc_host_sdhc_polling_MK64F12)
endif()
if(NOT (CONFIG_USE_middleware_sdmmc_host_sdhc_freertos_MK64F12 OR CONFIG_USE_middleware_sdmmc_host_sdhc_interrupt_MK64F12 OR CONFIG_USE_middleware_sdmmc_host_sdhc_polling_MK64F12))
    message(WARNING "Since middleware_sdmmc_host_sdhc_freertos_MK64F12/middleware_sdmmc_host_sdhc_interrupt_MK64F12/middleware_sdmmc_host_sdhc_polling_MK64F12 is not included at first or config in config.cmake file, use middleware_sdmmc_host_sdhc_interrupt_MK64F12 by default.")
    include(middleware_sdmmc_host_sdhc_interrupt_MK64F12)
endif()

