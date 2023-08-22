include_guard(GLOBAL)
message("middleware_sigfox component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/sf.c
    ${CMAKE_CURRENT_LIST_DIR}/sf_setup.c
)


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/.
)


include(middleware_aml_MK64F12)

