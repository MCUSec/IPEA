include_guard(GLOBAL)
message("middleware_mcu-boot_src_security component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/security/src/aes_security.c
    ${CMAKE_CURRENT_LIST_DIR}/src/security/src/aes128_key_wrap_unwrap.c
    ${CMAKE_CURRENT_LIST_DIR}/src/security/src/cbc_mac.c
)


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/security
)


