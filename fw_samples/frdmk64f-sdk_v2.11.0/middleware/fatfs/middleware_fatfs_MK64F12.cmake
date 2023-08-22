include_guard(GLOBAL)
message("middleware_fatfs component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/source/diskio.c
    ${CMAKE_CURRENT_LIST_DIR}/source/ff.c
    ${CMAKE_CURRENT_LIST_DIR}/source/ffsystem.c
    ${CMAKE_CURRENT_LIST_DIR}/source/ffunicode.c
)


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/source
)


#OR Logic component
if(CONFIG_USE_middleware_fatfs_template_ram_MK64F12)
     include(middleware_fatfs_template_ram_MK64F12)
endif()
if(CONFIG_USE_middleware_fatfs_ram_MK64F12)
     include(middleware_fatfs_ram_MK64F12)
endif()
if(CONFIG_USE_middleware_fatfs_template_sd_MK64F12)
     include(middleware_fatfs_template_sd_MK64F12)
endif()
if(CONFIG_USE_middleware_fatfs_sd_MK64F12)
     include(middleware_fatfs_sd_MK64F12)
endif()
if(CONFIG_USE_middleware_fatfs_template_sdspi_MK64F12)
     include(middleware_fatfs_template_sdspi_MK64F12)
endif()
if(CONFIG_USE_middleware_fatfs_sdspi_MK64F12)
     include(middleware_fatfs_sdspi_MK64F12)
endif()
if(CONFIG_USE_middleware_fatfs_template_mmc_MK64F12)
     include(middleware_fatfs_template_mmc_MK64F12)
endif()
if(CONFIG_USE_middleware_fatfs_mmc_MK64F12)
     include(middleware_fatfs_mmc_MK64F12)
endif()
if(CONFIG_USE_middleware_fatfs_template_usb_MK64F12)
     include(middleware_fatfs_template_usb_MK64F12)
endif()
if(CONFIG_USE_middleware_fatfs_usb_MK64F12)
     include(middleware_fatfs_usb_MK64F12)
endif()
if(CONFIG_USE_middleware_fatfs_template_nand_MK64F12)
     include(middleware_fatfs_template_nand_MK64F12)
endif()
if(CONFIG_USE_middleware_fatfs_nand_MK64F12)
     include(middleware_fatfs_nand_MK64F12)
endif()
if(NOT (CONFIG_USE_middleware_fatfs_template_ram_MK64F12 OR CONFIG_USE_middleware_fatfs_ram_MK64F12 OR CONFIG_USE_middleware_fatfs_template_sd_MK64F12 OR CONFIG_USE_middleware_fatfs_sd_MK64F12 OR CONFIG_USE_middleware_fatfs_template_sdspi_MK64F12 OR CONFIG_USE_middleware_fatfs_sdspi_MK64F12 OR CONFIG_USE_middleware_fatfs_template_mmc_MK64F12 OR CONFIG_USE_middleware_fatfs_mmc_MK64F12 OR CONFIG_USE_middleware_fatfs_template_usb_MK64F12 OR CONFIG_USE_middleware_fatfs_usb_MK64F12 OR CONFIG_USE_middleware_fatfs_template_nand_MK64F12 OR CONFIG_USE_middleware_fatfs_nand_MK64F12))
    message(WARNING "Since middleware_fatfs_template_ram_MK64F12/middleware_fatfs_ram_MK64F12/middleware_fatfs_template_sd_MK64F12/middleware_fatfs_sd_MK64F12/middleware_fatfs_template_sdspi_MK64F12/middleware_fatfs_sdspi_MK64F12/middleware_fatfs_template_mmc_MK64F12/middleware_fatfs_mmc_MK64F12/middleware_fatfs_template_usb_MK64F12/middleware_fatfs_usb_MK64F12/middleware_fatfs_template_nand_MK64F12/middleware_fatfs_nand_MK64F12 is not included at first or config in config.cmake file, use middleware_fatfs_template_ram_MK64F12/middleware_fatfs_ram_MK64F12 by default.")
    include(middleware_fatfs_template_ram_MK64F12)
    include(middleware_fatfs_ram_MK64F12)
endif()

