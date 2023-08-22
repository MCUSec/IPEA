include_guard(GLOBAL)
message("middleware_se_hostlib_cloud_enet_frdmk64f component is included.")


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/demos/ksdk/common/freertos/boards/frdmk64f
)

include(driver_enet_MK64F12)

include(middleware_freertos-aws_iot_vendor_nxp_secure_sockets_lwip_MK64F12)

include(middleware_lwip_MK64F12)

