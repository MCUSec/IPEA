include_guard(GLOBAL)
message("middleware_lwip_usb_ethernetif component is included.")

if(CONFIG_USE_middleware_baremetal_MK64F12)
target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/port/usb_ethernetif_bm.c
)
elseif(CONFIG_USE_middleware_freertos-kernel_MK64F12)
target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/port/usb_ethernetif_freertos.c
)
else()
    message(WARNING "please config middleware.baremetal_MK64F12 or middleware.freertos-kernel_MK64F12 first.")
endif()


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/port
)


#OR Logic component
if(CONFIG_USE_middleware_usb_host_khci_MK64F12)
     include(middleware_usb_host_khci_MK64F12)
endif()
if(CONFIG_USE_middleware_usb_host_ehci_MK64F12)
     include(middleware_usb_host_ehci_MK64F12)
endif()
if(CONFIG_USE_middleware_usb_host_ohci_MK64F12)
     include(middleware_usb_host_ohci_MK64F12)
endif()
if(NOT (CONFIG_USE_middleware_usb_host_khci_MK64F12 OR CONFIG_USE_middleware_usb_host_ehci_MK64F12 OR CONFIG_USE_middleware_usb_host_ohci_MK64F12))
    message(WARNING "Since middleware_usb_host_khci_MK64F12/middleware_usb_host_ehci_MK64F12/middleware_usb_host_ohci_MK64F12 is not included at first or config in config.cmake file, use middleware_usb_host_ehci_MK64F12 by default.")
    include(middleware_usb_host_ehci_MK64F12)
endif()

include(middleware_lwip_MK64F12)

include(middleware_usb_host_cdc_MK64F12)

include(middleware_usb_host_cdc_rndis_MK64F12)

