include_guard(GLOBAL)
message("middleware_se_hostlib_commonA71CHsmCom component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/hostlib/hostLib/libCommon/smCom/sci2c.c
    ${CMAKE_CURRENT_LIST_DIR}/hostlib/hostLib/libCommon/smCom/smCom.c
    ${CMAKE_CURRENT_LIST_DIR}/hostlib/hostLib/libCommon/smCom/smComSCI2C.c
)


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/hostlib/hostLib/libCommon/smCom
)


