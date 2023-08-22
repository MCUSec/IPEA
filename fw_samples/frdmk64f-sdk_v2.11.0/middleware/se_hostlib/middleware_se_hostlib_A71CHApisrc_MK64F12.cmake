include_guard(GLOBAL)
message("middleware_se_hostlib_A71CHApisrc component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/hostlib/hostLib/api/src/A71HLSEWrapper.c
    ${CMAKE_CURRENT_LIST_DIR}/hostlib/hostLib/api/src/ax_crypto_aes_key.c
    ${CMAKE_CURRENT_LIST_DIR}/hostlib/hostLib/api/src/ax_crypto_ecc.c
    ${CMAKE_CURRENT_LIST_DIR}/hostlib/hostLib/api/src/ax_crypto_rng.c
    ${CMAKE_CURRENT_LIST_DIR}/hostlib/hostLib/api/src/ax_scp.c
    ${CMAKE_CURRENT_LIST_DIR}/hostlib/hostLib/api/src/ax_sss_scp.c
    ${CMAKE_CURRENT_LIST_DIR}/hostlib/hostLib/api/src/ax_sst.c
    ${CMAKE_CURRENT_LIST_DIR}/hostlib/hostLib/api/src/ax_switch.c
    ${CMAKE_CURRENT_LIST_DIR}/hostlib/hostLib/api/src/ax_util.c
)


