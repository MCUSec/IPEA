include_guard(GLOBAL)
message("middleware_se_hostlib_common_A71CH component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/hostlib/hostLib/libCommon/scp/scp_a7x.c
    ${CMAKE_CURRENT_LIST_DIR}/hostlib/hostLib/platform/ksdk/ax_reset.c
    ${CMAKE_CURRENT_LIST_DIR}/hostlib/hostLib/platform/ksdk/i2c_frdm.c
    ${CMAKE_CURRENT_LIST_DIR}/hostlib/hostLib/platform/ksdk/i2c_imxrt.c
    ${CMAKE_CURRENT_LIST_DIR}/hostlib/hostLib/platform/ksdk/timer_kinetis.c
    ${CMAKE_CURRENT_LIST_DIR}/hostlib/hostLib/platform/ksdk/timer_kinetis_bm.c
    ${CMAKE_CURRENT_LIST_DIR}/hostlib/hostLib/platform/ksdk/timer_kinetis_freertos.c
)


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/hostlib/hostLib/inc
    ${CMAKE_CURRENT_LIST_DIR}/hostlib/hostLib/platform/inc
    ${CMAKE_CURRENT_LIST_DIR}/hostlib/hostLib/platform/ksdk
    ${CMAKE_CURRENT_LIST_DIR}/sss/inc
    ${CMAKE_CURRENT_LIST_DIR}/sss/port/ksdk
    ${CMAKE_CURRENT_LIST_DIR}/.
    ${CMAKE_CURRENT_LIST_DIR}/hostlib/hostLib/libCommon/scp
    ${CMAKE_CURRENT_LIST_DIR}/platform
)


