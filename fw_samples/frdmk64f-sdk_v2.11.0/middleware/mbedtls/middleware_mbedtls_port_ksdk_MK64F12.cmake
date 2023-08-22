include_guard(GLOBAL)
message("middleware_mbedtls_port_ksdk component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/port/ksdk/ksdk_mbedtls.c
    ${CMAKE_CURRENT_LIST_DIR}/port/ksdk/des_alt.c
    ${CMAKE_CURRENT_LIST_DIR}/port/ksdk/aes_alt.c
    ${CMAKE_CURRENT_LIST_DIR}/port/ksdk/ecp_alt.c
    ${CMAKE_CURRENT_LIST_DIR}/port/ksdk/ecp_curves_alt.c
    ${CMAKE_CURRENT_LIST_DIR}/port/ksdk/ecp_alt_ksdk.c
)


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/port/ksdk
)


#OR Logic component
if(CONFIG_USE_middleware_mbedtls_kinetis_MK64F12)
     include(middleware_mbedtls_kinetis_MK64F12)
endif()
if(CONFIG_USE_middleware_mbedtls_kinetis1_MK64F12)
     include(middleware_mbedtls_kinetis1_MK64F12)
endif()
if(CONFIG_USE_middleware_mbedtls_kinetis2_MK64F12)
     include(middleware_mbedtls_kinetis2_MK64F12)
endif()
if(CONFIG_USE_middleware_mbedtls_kinetis3_MK64F12)
     include(middleware_mbedtls_kinetis3_MK64F12)
endif()
if(CONFIG_USE_middleware_mbedtls_kinetis4_MK64F12)
     include(middleware_mbedtls_kinetis4_MK64F12)
endif()
if(CONFIG_USE_middleware_mbedtls_kinetis5_MK64F12)
     include(middleware_mbedtls_kinetis5_MK64F12)
endif()
if(CONFIG_USE_middleware_mbedtls_kinetis6_MK64F12)
     include(middleware_mbedtls_kinetis6_MK64F12)
endif()
if(CONFIG_USE_middleware_mbedtls_lpc1_MK64F12)
     include(middleware_mbedtls_lpc1_MK64F12)
endif()
if(CONFIG_USE_middleware_mbedtls_lpc2_MK64F12)
     include(middleware_mbedtls_lpc2_MK64F12)
endif()
if(CONFIG_USE_middleware_mbedtls_lpc3_MK64F12)
     include(middleware_mbedtls_lpc3_MK64F12)
endif()
if(CONFIG_USE_middleware_mbedtls_lpc4_MK64F12)
     include(middleware_mbedtls_lpc4_MK64F12)
endif()
if(CONFIG_USE_middleware_mbedtls_rt_MK64F12)
     include(middleware_mbedtls_rt_MK64F12)
endif()
if(CONFIG_USE_middleware_mbedtls_rt1_MK64F12)
     include(middleware_mbedtls_rt1_MK64F12)
endif()
if(CONFIG_USE_middleware_mbedtls_rt2_MK64F12)
     include(middleware_mbedtls_rt2_MK64F12)
endif()
if(CONFIG_USE_middleware_mbedtls_empty_MK64F12)
     include(middleware_mbedtls_empty_MK64F12)
endif()
if(NOT (CONFIG_USE_middleware_mbedtls_kinetis_MK64F12 OR CONFIG_USE_middleware_mbedtls_kinetis1_MK64F12 OR CONFIG_USE_middleware_mbedtls_kinetis2_MK64F12 OR CONFIG_USE_middleware_mbedtls_kinetis3_MK64F12 OR CONFIG_USE_middleware_mbedtls_kinetis4_MK64F12 OR CONFIG_USE_middleware_mbedtls_kinetis5_MK64F12 OR CONFIG_USE_middleware_mbedtls_kinetis6_MK64F12 OR CONFIG_USE_middleware_mbedtls_lpc1_MK64F12 OR CONFIG_USE_middleware_mbedtls_lpc2_MK64F12 OR CONFIG_USE_middleware_mbedtls_lpc3_MK64F12 OR CONFIG_USE_middleware_mbedtls_lpc4_MK64F12 OR CONFIG_USE_middleware_mbedtls_rt_MK64F12 OR CONFIG_USE_middleware_mbedtls_rt1_MK64F12 OR CONFIG_USE_middleware_mbedtls_rt2_MK64F12 OR CONFIG_USE_middleware_mbedtls_empty_MK64F12))
    message(WARNING "Since middleware_mbedtls_kinetis_MK64F12/middleware_mbedtls_kinetis1_MK64F12/middleware_mbedtls_kinetis2_MK64F12/middleware_mbedtls_kinetis3_MK64F12/middleware_mbedtls_kinetis4_MK64F12/middleware_mbedtls_kinetis5_MK64F12/middleware_mbedtls_kinetis6_MK64F12/middleware_mbedtls_lpc1_MK64F12/middleware_mbedtls_lpc2_MK64F12/middleware_mbedtls_lpc3_MK64F12/middleware_mbedtls_lpc4_MK64F12/middleware_mbedtls_rt_MK64F12/middleware_mbedtls_rt1_MK64F12/middleware_mbedtls_rt2_MK64F12/middleware_mbedtls_empty_MK64F12 is not included at first or config in config.cmake file, use middleware_mbedtls_empty_MK64F12 by default.")
    include(middleware_mbedtls_empty_MK64F12)
endif()

include(middleware_mbedtls_MK64F12)

