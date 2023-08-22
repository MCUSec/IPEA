include_guard(GLOBAL)
message("middleware_se_hostlib_commonA71CHDemo component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/hostlib/hostLib/a71ch/src/a71ch_com_scp.c
    ${CMAKE_CURRENT_LIST_DIR}/hostlib/hostLib/a71ch/src/a71ch_crypto_derive.c
    ${CMAKE_CURRENT_LIST_DIR}/hostlib/hostLib/a71ch/src/a71ch_crypto_ecc.c
    ${CMAKE_CURRENT_LIST_DIR}/hostlib/hostLib/a71ch/src/a71ch_module.c
    ${CMAKE_CURRENT_LIST_DIR}/hostlib/hostLib/a71ch/src/a71ch_sst.c
    ${CMAKE_CURRENT_LIST_DIR}/sss/ex/src/ex_sss_boot.c
    ${CMAKE_CURRENT_LIST_DIR}/sss/ex/src/ex_sss_boot_connectstring.c
    ${CMAKE_CURRENT_LIST_DIR}/sss/ex/src/ex_sss_a71ch.c
    ${CMAKE_CURRENT_LIST_DIR}/sss/src/fsl_sss_apis.c
    ${CMAKE_CURRENT_LIST_DIR}/sss/src/fsl_sss_util_asn1_der.c
    ${CMAKE_CURRENT_LIST_DIR}/sss/src/fsl_sss_util_rsa_sign_utils.c
    ${CMAKE_CURRENT_LIST_DIR}/sss/src/a71ch/fsl_sscp_a71ch.c
    ${CMAKE_CURRENT_LIST_DIR}/sss/src/sscp/fsl_sscp_mu.c
    ${CMAKE_CURRENT_LIST_DIR}/sss/src/sscp/fsl_sss_sscp.c
    ${CMAKE_CURRENT_LIST_DIR}/sss/src/a71cx_common/fsl_sss_a71cx_cmn.c
    ${CMAKE_CURRENT_LIST_DIR}/sss/src/keystore/keystore_cmn.c
)


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/sss/ex/inc
    ${CMAKE_CURRENT_LIST_DIR}/sss/ex/src
    ${CMAKE_CURRENT_LIST_DIR}/sss/plugin/mbedtls
    ${CMAKE_CURRENT_LIST_DIR}/sss/port/ksdk
    ${CMAKE_CURRENT_LIST_DIR}/.
    ${CMAKE_CURRENT_LIST_DIR}/hostlib/hostLib/inc
    ${CMAKE_CURRENT_LIST_DIR}/hostlib/hostLib/libCommon/scp
    ${CMAKE_CURRENT_LIST_DIR}/hostLib/platform/inc
    ${CMAKE_CURRENT_LIST_DIR}/hostLib/platform/ksdk
    ${CMAKE_CURRENT_LIST_DIR}/platform
    ${CMAKE_CURRENT_LIST_DIR}/sss/inc
)


include(middleware_se_hostlib_A71CHhostCrypto_MK64F12)

include(middleware_se_hostlib_commonA71CHsmCom_MK64F12)

include(middleware_se_hostlib_commonInfra_MK64F12)

include(middleware_se_hostlib_mwlog_MK64F12)

include(middleware_se_hostlib_A71CHApisrc_MK64F12)

include(middleware_se_hostlib_commonCloudDemos_frdmk64f_MK64F12)

include(driver_i2c_freertos_MK64F12)

include(driver_enet_MK64F12)

include(driver_sim_MK64F12)

include(middleware_se_hostlib_mbedtls_a71ch_demos_MK64F12)

include(middleware_freertos-aws_iot_vendor_nxp_secure_sockets_lwip_MK64F12)

