include_guard(GLOBAL)
message("middleware_se_hostlib_mbedtls_a71ch_demos component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/hostlib/hostLib/mbedtls/src/ecdh_alt_ax.c
    ${CMAKE_CURRENT_LIST_DIR}/hostlib/hostLib/mbedtls/src/ecdh_alt.c
)


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/sss/plugin/mbedtls
    ${CMAKE_CURRENT_LIST_DIR}/sss/port/ksdk
)


include(middleware_se_hostlib_commonA71CHDemo_MK64F12)

include(middleware_se_hostlib_mbedtls_alt_demo_common_MK64F12)

