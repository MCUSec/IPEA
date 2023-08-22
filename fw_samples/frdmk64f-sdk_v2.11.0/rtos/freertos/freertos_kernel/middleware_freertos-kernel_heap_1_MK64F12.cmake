include_guard(GLOBAL)
message("middleware_freertos-kernel_heap_1 component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/portable/MemMang/heap_1.c
)


include(middleware_freertos-kernel_MK64F12)

