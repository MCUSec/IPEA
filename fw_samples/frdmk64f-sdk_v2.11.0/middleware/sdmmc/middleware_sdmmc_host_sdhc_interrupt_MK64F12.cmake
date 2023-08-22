include_guard(GLOBAL)
message("middleware_sdmmc_host_sdhc_interrupt component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/host/sdhc/non_blocking/fsl_sdmmc_host.c
)


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/host/sdhc
)


include(middleware_sdmmc_common_MK64F12)

include(middleware_sdmmc_osa_bm_MK64F12)

include(driver_sdhc_MK64F12)

