include_guard(GLOBAL)
message("middleware_freertos-aws_iot_vendor_nxp_pkcs11 component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/vendors/nxp/pkcs11/common/iot_pkcs11_pal.c
)


include(middleware_freertos-kernel_MK64F12)

include(middleware_freertos-aws_iot_libraries_freertos_plus_standard_crypto_MK64F12)

include(middleware_freertos-aws_iot_libraries_abstractions_pkcs11_MK64F12)

include(component_mflash_file_MK64F12)

