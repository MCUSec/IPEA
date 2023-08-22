include_guard(GLOBAL)
message("middleware_lwip_apps_httpd component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/apps/http/httpd.c
)


include(middleware_lwip_MK64F12)

include(middleware_lwip_apps_httpd_support_MK64F12)

